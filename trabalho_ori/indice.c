#include "indice.h"
#include "filme.h"

#include <stdio.h>
#include <string.h>

#define ARQ_INDICE_GENERO  "indice_genero.dat"
#define ARQ_LISTA_GENERO   "lista_genero.dat"

#define ARQ_INDICE_DIRETOR "indice_diretor.dat"
#define ARQ_LISTA_DIRETOR  "lista_diretor.dat"

static FILE *indice_genero = NULL;
static FILE *lista_genero = NULL;

static FILE *indice_diretor = NULL;
static FILE *lista_diretor = NULL;


//=========================================================
// AUXILIARES                                              
//=========================================================

static void para_minusculo(char *str) {
    int i;
    for(i = 0; str[i] != '\0'; i++) {
        if(str[i] >= 'A' && str[i] <= 'Z')
            str[i] += 32;
    }
}

static int comparar_sem_case(const char *a, const char *b) {
    while(*a && *b) {
        char c1 = *a;
        char c2 = *b;

        if(c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if(c2 >= 'A' && c2 <= 'Z') c2 += 32;

        if(c1 != c2)
            return c1 - c2;

        a++;
        b++;
    }
    return *a - *b;
}

static FILE *abrir_indice(const char *nome) {
    FILE *f = fopen(nome, "rb+");
    if(f == NULL)
        f = fopen(nome, "wb+");
    return f;
}

static long buscar_chave(FILE *indice, const char *chave) {
    IndiceSecundario entrada;
    rewind(indice);

    while(fread(&entrada, sizeof(IndiceSecundario), 1, indice) == 1) {
        if(comparar_sem_case(entrada.chave, chave) == 0)
            return ftell(indice) - sizeof(IndiceSecundario);
    }
    return -1;
}

static void inserir_chave(FILE *indice, FILE *lista, const char *chave, int idFilme) {
    char chave_normalizada[60];

    strcpy(chave_normalizada, chave);
    para_minusculo(chave_normalizada);

    long offset_chave = buscar_chave(indice, chave_normalizada);

    if(offset_chave != -1) {
        IndiceSecundario entrada;

        fseek(indice, offset_chave, SEEK_SET);
        fread(&entrada, sizeof(IndiceSecundario), 1, indice);

        NoListaInvertida novo;
        novo.idFilme = idFilme;
        novo.proximo = entrada.primeiro;

        fseek(lista, 0, SEEK_END);
        long offset_no = ftell(lista);

        fwrite(&novo, sizeof(NoListaInvertida), 1, lista);

        entrada.primeiro = offset_no;

        fseek(indice, offset_chave, SEEK_SET);
        fwrite(&entrada, sizeof(IndiceSecundario), 1, indice);

        fflush(indice);
        fflush(lista);
        return;
    }

    NoListaInvertida novo;
    novo.idFilme = idFilme;
    novo.proximo = -1;

    fseek(lista, 0, SEEK_END);
    long offset_no = ftell(lista);
    fwrite(&novo, sizeof(NoListaInvertida), 1, lista);

    IndiceSecundario entrada;
    strcpy(entrada.chave, chave_normalizada);
    entrada.primeiro = offset_no;

    fseek(indice, 0, SEEK_END);
    fwrite(&entrada, sizeof(IndiceSecundario), 1, indice);

    fflush(indice);
    fflush(lista);
}

static void remover_id(FILE *indice, FILE *lista, const char *chave, int idFilme) {
    long offset_chave = buscar_chave(indice, chave);

    if(offset_chave == -1) return;

    IndiceSecundario entrada;
    fseek(indice, offset_chave, SEEK_SET);
    fread(&entrada, sizeof(IndiceSecundario), 1, indice);

    long atual = entrada.primeiro;
    long anterior = -1;

    while(atual != -1) {
        NoListaInvertida no;
        fseek(lista, atual, SEEK_SET);
        fread(&no, sizeof(NoListaInvertida), 1, lista);

        if(no.idFilme == idFilme) {
            if(anterior == -1) {
                entrada.primeiro = no.proximo;
                fseek(indice, offset_chave, SEEK_SET);
                fwrite(&entrada, sizeof(IndiceSecundario), 1, indice);
            } else {
                NoListaInvertida ant;
                fseek(lista, anterior, SEEK_SET);
                fread(&ant, sizeof(NoListaInvertida), 1, lista);

                ant.proximo = no.proximo;

                fseek(lista, anterior, SEEK_SET);
                fwrite(&ant, sizeof(NoListaInvertida), 1, lista);
            }

            fflush(indice);
            fflush(lista);
            return;
        }
        anterior = atual;
        atual = no.proximo;
    }
}

//=========================================================
// GÊNERO                                                   
//=========================================================

void inicializar_indice_genero() {
    if(indice_genero == NULL)
        indice_genero = abrir_indice(ARQ_INDICE_GENERO);
    if(lista_genero == NULL)
        lista_genero = abrir_indice(ARQ_LISTA_GENERO);
}

void inserir_indice_genero(Filme filme) {
    inicializar_indice_genero();
    inserir_chave(indice_genero, lista_genero, filme.genero, filme.id);
}

// CORRIGIDO: Agora recebe o Filme inteiro e não tenta abrir o arquivo
void remover_indice_genero(Filme f) {
    remover_id(indice_genero, lista_genero, f.genero, f.id);
}

// CORRIGIDO: Agora recebe o ponteiro FILE *dados já aberto
void buscar_por_genero(FILE *dados, const char *genero) {
    inicializar_indice_genero();

    long offset = buscar_chave(indice_genero, genero);

    if(offset == -1) {
        printf("\nNenhum filme encontrado para o genero: %s\n", genero);
        return;
    }

    IndiceSecundario entrada;
    fseek(indice_genero, offset, SEEK_SET);
    fread(&entrada, sizeof(IndiceSecundario), 1, indice_genero);

    long atual = entrada.primeiro;
    int encontrou = 0;
    
    printf("\n--- RESULTADOS PARA O GENERO: %s ---", genero);

    while(atual != -1) {
        NoListaInvertida no;
        fseek(lista_genero, atual, SEEK_SET);
        fread(&no, sizeof(NoListaInvertida), 1, lista_genero);

        Filme f = buscar_filme(dados, no.idFilme);

        if(f.id != -1 && f.removido == '0') {
            printf("\nID: %d | Titulo: %s | Diretor: %s | Ano: %d", f.id, f.titulo, f.diretor, f.ano);
            encontrou = 1;
        }

        atual = no.proximo;
    }
    
    if(!encontrou) {
        printf("\nTodos os filmes deste genero foram removidos do sistema.\n");
    }
}


//=========================================================
// DIRETOR                                                  
//=========================================================

void inicializar_indice_diretor() {
    if(indice_diretor == NULL)
        indice_diretor = abrir_indice(ARQ_INDICE_DIRETOR);
    if(lista_diretor == NULL)
        lista_diretor = abrir_indice(ARQ_LISTA_DIRETOR);
}

void inserir_indice_diretor(Filme filme) {
    inicializar_indice_diretor();
    inserir_chave(indice_diretor, lista_diretor, filme.diretor, filme.id);
}

// CORRIGIDO: Agora recebe o Filme inteiro e não tenta abrir o arquivo
void remover_indice_diretor(Filme f) {
    remover_id(indice_diretor, lista_diretor, f.diretor, f.id);
}

// CORRIGIDO: Agora recebe o ponteiro FILE *dados já aberto
void buscar_por_diretor(FILE *dados, const char *diretor) {
    inicializar_indice_diretor();

    long offset = buscar_chave(indice_diretor, diretor);

    if(offset == -1) {
        printf("\nNenhum filme encontrado para o diretor: %s\n", diretor);
        return;
    }

    IndiceSecundario entrada;
    fseek(indice_diretor, offset, SEEK_SET);
    fread(&entrada, sizeof(IndiceSecundario), 1, indice_diretor);

    long atual = entrada.primeiro;
    int encontrou = 0;
    
    printf("\n--- RESULTADOS PARA O DIRETOR: %s ---", diretor);

    while(atual != -1) {
        NoListaInvertida no;
        fseek(lista_diretor, atual, SEEK_SET);
        fread(&no, sizeof(NoListaInvertida), 1, lista_diretor);

        Filme f = buscar_filme(dados, no.idFilme);

        if(f.id != -1 && f.removido == '0') {
            printf("\nID: %d | Titulo: %s | Genero: %s | Ano: %d", f.id, f.titulo, f.genero, f.ano);
            encontrou = 1;
        }

        atual = no.proximo;
    }
    
    if(!encontrou) {
        printf("\nTodos os filmes deste diretor foram removidos do sistema.\n");
    }
}


//=========================================================
// MANUTENÇÃO                                               
//=========================================================

void atualizar_indices(Filme antigo, Filme novo) {
    novo.id = antigo.id;

    if(comparar_sem_case(antigo.genero, novo.genero) != 0) {
        remover_indice_genero(antigo);
        inserir_indice_genero(novo);
    }

    if(comparar_sem_case(antigo.diretor, novo.diretor) != 0) {
        remover_indice_diretor(antigo);
        inserir_indice_diretor(novo);
    }
}

// CORRIGIDO: Recebe struct Filme
void remover_de_todos_indices(Filme f) {
    remover_indice_genero(f);
    remover_indice_diretor(f);
}

void fechar_indices() {
    if(indice_genero) fclose(indice_genero);
    if(lista_genero) fclose(lista_genero);

    if(indice_diretor) fclose(indice_diretor);
    if(lista_diretor) fclose(lista_diretor);

    indice_genero = NULL;
    lista_genero = NULL;

    indice_diretor = NULL;
    lista_diretor = NULL;
}
