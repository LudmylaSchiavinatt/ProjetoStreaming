#ifndef FILME_H
#define FILME_H

#include <stdio.h>

// Estrutura de tamanho fixo com campo para encadeamento da LED
typedef struct {
    int id;
    char titulo[100];
    char diretor[60];
    int ano;
    char genero[30];
    int duracao;
    char classificacao[5];
    float nota;
    char removido;       // '0' para ativo, '1' para removido
    long prox_livre;     // Guarda o offset do próximo espaço vazio na LED (-1 se fim da lista)
} Filme;

// Protótipos das funções de manipulação de arquivo
FILE* abrir_arquivo(const char* nome_arquivo);
void fechar_arquivo(FILE* arq);

void inserir_filme(FILE* arq, Filme f);
Filme buscar_filme(FILE* arq, int id);
int atualizar_filme(FILE* arq, int id, Filme novos_dados);
int remover_filme(FILE* arq, int id);

// Funções utilitárias para a LED exigidas pelo módulo
long contar_espacos_livres(FILE *arq);

#endif
