#include "filme.h"
#include "led.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_CABECALHO ((long)sizeof(long))

FILE* abrir_arquivo(const char* nome_arquivo) {
    FILE* arq = fopen(nome_arquivo, "rb+");
    if (arq == NULL) {
        // Se o arquivo não existe, cria do zero e inicializa a cabeça da LED com -1
        arq = fopen(nome_arquivo, "wb+");
        if (arq != NULL) {
            long cabeca_led = -1;
            fwrite(&cabeca_led, sizeof(long), 1, arq);
            fflush(arq);
        }
    }
    return arq;
}

void fechar_arquivo(FILE* arq) {
    if (arq != NULL) {
        fclose(arq);
    }
}

void inserir_filme(FILE* arq, Filme f) {
    f.removido = '0';
    f.prox_livre = -1;
    
    // Obtém o offset disponível usando o módulo da LED
    long offset = led_alocar_espaco(arq);
    
    if (offset == -1) {
        // Se a LED estiver vazia, insere no final do arquivo
        fseek(arq, 0, SEEK_END);
        offset = ftell(arq);
    }
    
    // Grava o registro no local determinado
    fseek(arq, offset, SEEK_SET);
    fwrite(&f, sizeof(Filme), 1, arq);
    fflush(arq);
}

Filme buscar_filme(FILE* arq, int id_procurado) {
    Filme f;
    // Pula o cabeçalho inicial para ler os registros
    fseek(arq, TAM_CABECALHO, SEEK_SET);

    while (fread(&f, sizeof(Filme), 1, arq) == 1) {
        if (f.id == id_procurado && f.removido == '0') {
            return f; 
        }
    }

    f.id = -1; // Retorna ID -1 indicando que não foi encontrado
    return f;
}

int atualizar_filme(FILE* arq, int id_procurado, Filme novos_dados) {
    Filme f;
    fseek(arq, TAM_CABECALHO, SEEK_SET);

    while (fread(&f, sizeof(Filme), 1, arq) == 1) {
        if (f.id == id_procurado && f.removido == '0') {
            novos_dados.id = f.id; 
            novos_dados.removido = '0';
            novos_dados.prox_livre = -1;
            
            // Volta o tamanho exato de um registro para sobrescrevê-lo
            fseek(arq, -(long)sizeof(Filme), SEEK_CUR);
            fwrite(&novos_dados, sizeof(Filme), 1, arq);
            fflush(arq);
            return 1; 
        }
    }
    return 0; 
}

int remover_filme(FILE* arq, int id_procurado) {
    Filme f;
    fseek(arq, TAM_CABECALHO, SEEK_SET);
    
    long offset_atual = ftell(arq);
    // Lê registro por registro guardando o offset onde ele começou
    while (fread(&f, sizeof(Filme), 1, arq) == 1) {
        if (f.id == id_procurado && f.removido == '0') {
            // Delega para o módulo da LED fazer a liberação física e lógica do espaço
            led_liberar_espaco(arq, offset_atual);
            return 1; 
        }
        offset_atual = ftell(arq);
    }
    return 0; 
}

long contar_espacos_livres(FILE *arq) {
    return led_contar_espacos_livres(arq);
}
