#ifndef ARVORE_B_H
#define ARVORE_B_H

#include <stdio.h>

#define ORDEM 5

typedef struct {
    int num_chaves;                  // Quantidade de chaves armazenadas no nó
    int chaves[ORDEM - 1];           // IDs dos filmes
    long offsets_dados[ORDEM - 1];   // Ponteiros (byte offsets) para o dados_filmes.dat
    long filhos[ORDEM];              // Ponteiros (byte offsets) para os filhos no arvore_b.dat
    int eh_folha;                    // 1 se for folha, 0 se for nó interno
} NoArvoreB;

// Funções principais
void inicializar_arvore_b();
long buscar_arvore_b(int id);
void inserir_arvore_b(int id, long offset_dado);
void remover_arvore_b(int id);
void fechar_arvore_b();

#endif