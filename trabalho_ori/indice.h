
#ifndef INDICE_H
#define INDICE_H

#include "filme.h"
#include <stdio.h>

// Estrutura do nó da lista invertida
typedef struct {
    int idFilme;
    long proximo;
} NoListaInvertida;

// Estrutura da entrada do índice
typedef struct {
    char chave[60];
    long primeiro;
} IndiceSecundario;


// funções do índice por gênero
// Inicializa o arquivo de índice de gênero
void inicializar_indice_genero();

// Insere um filme no índice de gênero
void inserir_indice_genero(Filme filme);

// Remove um filme do índice de gênero
void remover_indice_genero(int idFilme);

// Busca todos os filmes de um determinado gênero
void buscar_por_genero(const char *genero);


//funções do índice por diretor
// Inicializa o arquivo de índice de diretor
void inicializar_indice_diretor();

// Insere um filme no índice de diretor
void inserir_indice_diretor(Filme filme);

// Remove um filme do índice de diretor
void remover_indice_diretor(int idFilme);

// Busca todos os filmes de um determinado diretor
void buscar_por_diretor(const char *diretor);

//funções auxiliares
// Atualiza os índices quando um filme é atualizado
void atualizar_indices(Filme filme_antigo, Filme filme_novo);

// Remove um filme de todos os índices
void remover_de_todos_indices(int idFilme);

// Fecha os arquivos de índice
void fechar_indices();

#endif
