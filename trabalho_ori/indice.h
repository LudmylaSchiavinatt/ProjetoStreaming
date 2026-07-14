#ifndef INDICE_H
#define INDICE_H

#include "filme.h"

typedef struct {
    int idFilme;
    long proximo;
} NoListaInvertida;

typedef struct {
    char chave[60];
    long primeiro;
} IndiceSecundario;

// Gênero 
void inicializar_indice_genero();
void inserir_indice_genero(Filme filme);
void remover_indice_genero(int idFilme);
void buscar_por_genero(FILE *dados, const char *genero); 

// Diretor 
void inicializar_indice_diretor();
void inserir_indice_diretor(Filme filme);
void remover_indice_diretor(int idFilme);
void buscar_por_diretor(FILE *dados, const char *diretor); 

// Manutenção 
void atualizar_indices(Filme antigo, Filme novo);
void remover_de_todos_indices(int idFilme);
void fechar_indices();

#endif
