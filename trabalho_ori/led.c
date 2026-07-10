#include "led.h"
#include "filme.h"
#include <stdio.h>

long led_alocar_espaco(FILE *f) {
    long cabeca;
    fseek(f, 0, SEEK_SET);
    
    // Lê a cabeça atual do arquivo
    if (fread(&cabeca, sizeof(long), 1, f) != 1 || cabeca == -1) {
        return -1; // LED vazia
    }
    
    // Lê o registro deletado para descobrir quem é o próximo na lista de reuso
    Filme registro_removido;
    fseek(f, cabeca, SEEK_SET);
    fread(&registro_removido, sizeof(Filme), 1, f);
    
    // Atualiza a cabeça do cabeçalho com o próximo endereço livre
    fseek(f, 0, SEEK_SET);
    fwrite(&registro_removido.prox_livre, sizeof(long), 1, f);
    fflush(f);
    
    return cabeca; // Retorna o offset que agora será reutilizado
}

void led_liberar_espaco(FILE *f, long offset_registro) {
    long antiga_cabeca;
    fseek(f, 0, SEEK_SET);
    fread(&antiga_cabeca, sizeof(long), 1, f);
    
    // Resgata o registro atual para marcar como removido
    Filme tmp;
    fseek(f, offset_registro, SEEK_SET);
    fread(&tmp, sizeof(Filme), 1, f);
    
    tmp.removido = '1';
    tmp.prox_livre = antiga_cabeca; // O próximo livre passa a ser quem estava na cabeça (LIFO)
    
    // Atualiza o registro fisicamente no disco
    fseek(f, offset_registro, SEEK_SET);
    fwrite(&tmp, sizeof(Filme), 1, f);
    
    // Atualiza o cabeçalho do arquivo para apontar para este novo espaço que acabou de liberar
    fseek(f, 0, SEEK_SET);
    fwrite(&offset_registro, sizeof(long), 1, f);
    fflush(f);
}

long led_contar_espacos_livres(FILE *f) {
    long total = 0;
    long atual;
    
    fseek(f, 0, SEEK_SET);
    if (fread(&atual, sizeof(long), 1, f) != 1) {
        return 0;
    }
    
    Filme tmp;
    while (atual != -1) {
        total++;
        fseek(f, atual, SEEK_SET);
        if (fread(&tmp, sizeof(Filme), 1, f) != 1) {
            break;
        }
        atual = tmp.prox_livre;
    }
    return total;
}
