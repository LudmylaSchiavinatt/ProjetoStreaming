#include "filme.h"
#include "led.h"
#include "arvore_b.h" //cabeçalho da Árvore B
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_CABECALHO ((long)sizeof(long))

FILE* abrir_arquivo(const char* nome_arquivo) {
    FILE* arq = fopen(nome_arquivo, "rb+");
    if (arq == NULL) {
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
    
    // 1. Obtém o offset disponível usando o módulo da LED
    long offset = led_alocar_espaco(arq);
    
    if (offset == -1) {
        // Se a LED estiver vazia, insere no final do arquivo
        fseek(arq, 0, SEEK_END);
        offset = ftell(arq);
    }
    
    // 2. Grava o registro físico no disco
    fseek(arq, offset, SEEK_SET);
    fwrite(&f, sizeof(Filme), 1, arq);
    fflush(arq);

    // 3. NOVO: Insere a Chave Primária (ID) e o Offset na Árvore B
    inserir_arvore_b(f.id, offset); 
}

Filme buscar_filme(FILE* arq, int id_procurado) {
    Filme f;
    f.id = -1; // Valor padrão para não encontrado

    // 1. NOVO: Busca o offset diretamente na Árvore B em disco
    long offset = buscar_arvore_b(id_procurado);

    if (offset != -1) {
        // 2. Se a árvore encontrou, pula direto para o byte exato e lê
        fseek(arq, offset, SEEK_SET);
        if (fread(&f, sizeof(Filme), 1, arq) == 1) {
            // Garante que não é um registro logicamente removido
            if (f.removido == '0') {
                return f;
            }
        }
    }
    
    f.id = -1; 
    return f;
}

int atualizar_filme(FILE* arq, int id_procurado, Filme novos_dados) {
    Filme f;

    // Pega o offset direto da Árvore B
    long offset = buscar_arvore_b(id_procurado);

    if (offset != -1) {
        fseek(arq, offset, SEEK_SET);
        if (fread(&f, sizeof(Filme), 1, arq) == 1) {
            if (f.removido == '0') {
                // Prepara os novos dados mantendo a integridade estrutural
                novos_dados.id = f.id; 
                novos_dados.removido = '0';
                novos_dados.prox_livre = f.prox_livre;
                
                // Volta o ponteiro e sobrescreve no mesmo lugar
                fseek(arq, offset, SEEK_SET);
                fwrite(&novos_dados, sizeof(Filme), 1, arq);
                fflush(arq);
                return 1; 
            }
        }
    }
    return 0; // Filme não encontrado ou já removido
}

int remover_filme(FILE* arq, int id_procurado) {
    Filme f;

    //  pega o offset direto da Árvore B
    long offset = buscar_arvore_b(id_procurado);

    if (offset != -1) {
        fseek(arq, offset, SEEK_SET);
        if (fread(&f, sizeof(Filme), 1, arq) == 1) {
            if (f.removido == '0') {
                // delega para a LED fazer a liberação (marca como '1')
                led_liberar_espaco(arq, offset);
                
                //remove a chave do Índice Primário
                remover_arvore_b(id_procurado);
                
                return 1; 
            }
        }
    }
    return 0; 
}

long contar_espacos_livres(FILE *arq) {
    return led_contar_espacos_livres(arq);
}
