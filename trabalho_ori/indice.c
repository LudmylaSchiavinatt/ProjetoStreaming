#include "indice.h"
#include "filme.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//constantes
#define NOME_INDICE_GENERO "indice_genero.dat"
#define NOME_INDICE_DIRETOR "indice_diretor.dat"

// variáveis globais
static FILE* arquivo_indice_genero = NULL;
static FILE* arquivo_indice_diretor = NULL;

//funções auxiliares
// Converte uma string para minúsculo (função caseira)
static void para_minusculo(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] + 32;  // 'A' (65) vira 'a' (97)
        }
    }
}

// Compara duas strings ignorando maiúsculas/minúsculas
static int comparar_sem_case(const char *str1, const char *str2) {
    int i = 0;
    
    while (str1[i] != '\0' && str2[i] != '\0') {
        char c1 = str1[i];
        char c2 = str2[i];
        
        // Converte temporariamente para minúsculo
        if (c1 >= 'A' && c1 <= 'Z') c1 = c1 + 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 = c2 + 32;
        
        if (c1 != c2) {
            return c1 - c2;
        }
        i++;
    }
    
    return str1[i] - str2[i];
}

// Abre ou cria um arquivo de índice
static FILE* abrir_arquivo_indice(const char* nome) {
    FILE* arq = fopen(nome, "rb+");
    
    if (arq == NULL) {
        arq = fopen(nome, "wb+");
        if (arq != NULL) {
            long cabeca = -1;
            fwrite(&cabeca, sizeof(long), 1, arq);
            fflush(arq);
        }
    }
    
    return arq;
}

// Busca uma chave no arquivo de índice (compara sem case)
static long buscar_chave_indice(FILE* arq, const char* chave) {
    IndiceSecundario entrada;
    long offset = sizeof(long);
    
    fseek(arq, offset, SEEK_SET);
    
    while (fread(&entrada, sizeof(IndiceSecundario), 1, arq) == 1) {
        // Usa comparação sem case
        // Compara sem converter a string original
        if (comparar_sem_case(entrada.chave, chave) == 0) {
            return offset;
        }
        offset = ftell(arq);
    }
    
    return -1;
}

// Insere uma nova chave no índice
static void inserir_chave_indice(FILE* arq, const char* chave, int idFilme) {
    long offset_chave = buscar_chave_indice(arq, chave);
    char chave_normalizada[60];
    
    // Copia a chave e normaliza (converte para minúsculo)
    strcpy(chave_normalizada, chave);
    //Converte string inteira
    para_minusculo(chave_normalizada);
    
    if (offset_chave != -1) {
        // Chave já existe
        IndiceSecundario entrada;
        fseek(arq, offset_chave, SEEK_SET);
        fread(&entrada, sizeof(IndiceSecundario), 1, arq);
        
        NoListaInvertida novo_no;
        novo_no.idFilme = idFilme;
        novo_no.proximo = entrada.primeiro;
        
        fseek(arq, 0, SEEK_END);
        long offset_no = ftell(arq);
        fwrite(&novo_no, sizeof(NoListaInvertida), 1, arq);
        
        entrada.primeiro = offset_no;
        fseek(arq, offset_chave, SEEK_SET);
        fwrite(&entrada, sizeof(IndiceSecundario), 1, arq);
        
        fflush(arq);
        return;
    }
    
    // Chave nova
    IndiceSecundario nova_entrada;
    strcpy(nova_entrada.chave, chave_normalizada);
    
    NoListaInvertida novo_no;
    novo_no.idFilme = idFilme;
    novo_no.proximo = -1;
    
    fseek(arq, 0, SEEK_END);
    long offset_no = ftell(arq);
    fwrite(&novo_no, sizeof(NoListaInvertida), 1, arq);
    
    nova_entrada.primeiro = offset_no;
    
    fseek(arq, 0, SEEK_END);
    fwrite(&nova_entrada, sizeof(IndiceSecundario), 1, arq);
    
    fflush(arq);
}

// Remove um ID da lista invertida
static void remover_id_da_chave(FILE* arq, const char* chave, int idFilme) {
    long offset_chave = buscar_chave_indice(arq, chave);
    
    if (offset_chave == -1) {
        return;
    }
    
    IndiceSecundario entrada;
    fseek(arq, offset_chave, SEEK_SET);
    fread(&entrada, sizeof(IndiceSecundario), 1, arq);
    
    long atual = entrada.primeiro;
    long anterior = -1;
    
    while (atual != -1) {
        NoListaInvertida no;
        fseek(arq, atual, SEEK_SET);
        fread(&no, sizeof(NoListaInvertida), 1, arq);
        
        if (no.idFilme == idFilme) {
            if (anterior == -1) {
                entrada.primeiro = no.proximo;
                fseek(arq, offset_chave, SEEK_SET);
                fwrite(&entrada, sizeof(IndiceSecundario), 1, arq);
            } else {
                NoListaInvertida no_anterior;
                fseek(arq, anterior, SEEK_SET);
                fread(&no_anterior, sizeof(NoListaInvertida), 1, arq);
                no_anterior.proximo = no.proximo;
                fseek(arq, anterior, SEEK_SET);
                fwrite(&no_anterior, sizeof(NoListaInvertida), 1, arq);
            }
            
            fflush(arq);
            return;
        }
        
        anterior = atual;
        atual = no.proximo;
    }
}

//implementação do indice por gênero
void inicializar_indice_genero() {
    if (arquivo_indice_genero == NULL) {
        arquivo_indice_genero = abrir_arquivo_indice(NOME_INDICE_GENERO);
    }
}

void inserir_indice_genero(Filme filme) {
    if (arquivo_indice_genero == NULL) {
        inicializar_indice_genero();
    }
    
    // Usa o gênero diretamente (a função inserir_chave_indice já normaliza)
    inserir_chave_indice(arquivo_indice_genero, filme.genero, filme.id);
}

void remover_indice_genero(int idFilme) {
    FILE* arq_dados = abrir_arquivo("dados_filmes.dat");
    if (arq_dados == NULL) return;
    
    Filme f = buscar_filme(arq_dados, idFilme);
    fechar_arquivo(arq_dados);
    
    if (f.id == -1) return;
    
    remover_id_da_chave(arquivo_indice_genero, f.genero, idFilme);
}

void buscar_por_genero(const char *genero) {
    if (arquivo_indice_genero == NULL) {
        inicializar_indice_genero();
    }
    
    // Busca a chave (já faz comparação sem case)
    long offset_chave = buscar_chave_indice(arquivo_indice_genero, genero);
    
    if (offset_chave == -1) {
        printf("\n>>> Nenhum filme encontrado para o genero: %s\n", genero);
        return;
    }
    
    IndiceSecundario entrada;
    fseek(arquivo_indice_genero, offset_chave, SEEK_SET);
    fread(&entrada, sizeof(IndiceSecundario), 1, arquivo_indice_genero);
    
    FILE* arq_dados = abrir_arquivo("dados_filmes.dat");
    if (arq_dados == NULL) return;
    
    printf("\n=== FILMES DO GENERO: %s ===\n", genero);
    printf("----------------------------------------\n");
    
    long atual = entrada.primeiro;
    int contador = 0;
    
    while (atual != -1) {
        NoListaInvertida no;
        fseek(arquivo_indice_genero, atual, SEEK_SET);
        fread(&no, sizeof(NoListaInvertida), 1, arquivo_indice_genero);
        
        Filme f = buscar_filme(arq_dados, no.idFilme);
        if (f.id != -1) {
            printf("ID: %d | Titulo: %s | Diretor: %s | Ano: %d\n", 
                   f.id, f.titulo, f.diretor, f.ano);
            contador++;
        }
        
        atual = no.proximo;
    }
    
    printf("----------------------------------------\n");
    printf("Total: %d filme(s) encontrado(s)\n", contador);
    
    fechar_arquivo(arq_dados);
}

//implementação do indice por diretor
void inicializar_indice_diretor() {
    if (arquivo_indice_diretor == NULL) {
        arquivo_indice_diretor = abrir_arquivo_indice(NOME_INDICE_DIRETOR);
    }
}

void inserir_indice_diretor(Filme filme) {
    if (arquivo_indice_diretor == NULL) {
        inicializar_indice_diretor();
    }
    
    inserir_chave_indice(arquivo_indice_diretor, filme.diretor, filme.id);
}

void remover_indice_diretor(int idFilme) {
    FILE* arq_dados = abrir_arquivo("dados_filmes.dat");
    if (arq_dados == NULL) return;
    
    Filme f = buscar_filme(arq_dados, idFilme);
    fechar_arquivo(arq_dados);
    
    if (f.id == -1) return;
    
    remover_id_da_chave(arquivo_indice_diretor, f.diretor, idFilme);
}

void buscar_por_diretor(const char *diretor) {
    if (arquivo_indice_diretor == NULL) {
        inicializar_indice_diretor();
    }
    
    long offset_chave = buscar_chave_indice(arquivo_indice_diretor, diretor);
    
    if (offset_chave == -1) {
        printf("\n>>> Nenhum filme encontrado para o diretor: %s\n", diretor);
        return;
    }
    
    IndiceSecundario entrada;
    fseek(arquivo_indice_diretor, offset_chave, SEEK_SET);
    fread(&entrada, sizeof(IndiceSecundario), 1, arquivo_indice_diretor);
    
    FILE* arq_dados = abrir_arquivo("dados_filmes.dat");
    if (arq_dados == NULL) return;
    
    printf("\n=== FILMES DO DIRETOR: %s ===\n", diretor);
    printf("----------------------------------------\n");
    
    long atual = entrada.primeiro;
    int contador = 0;
    
    while (atual != -1) {
        NoListaInvertida no;
        fseek(arquivo_indice_diretor, atual, SEEK_SET);
        fread(&no, sizeof(NoListaInvertida), 1, arquivo_indice_diretor);
        
        Filme f = buscar_filme(arq_dados, no.idFilme);
        if (f.id != -1) {
            printf("ID: %d | Titulo: %s | Genero: %s | Ano: %d\n", 
                   f.id, f.titulo, f.genero, f.ano);
            contador++;
        }
        
        atual = no.proximo;
    }
    
    printf("----------------------------------------\n");
    printf("Total: %d filme(s) encontrado(s)\n", contador);
    
    fechar_arquivo(arq_dados);
}

//funções de manutenção
void atualizar_indices(Filme filme_antigo, Filme filme_novo) {
    // Se o gênero mudou
    if (comparar_sem_case(filme_antigo.genero, filme_novo.genero) != 0) {
        remover_indice_genero(filme_antigo.id);
        inserir_indice_genero(filme_novo);
    }
    
    // Se o diretor mudou
    if (comparar_sem_case(filme_antigo.diretor, filme_novo.diretor) != 0) {
        remover_indice_diretor(filme_antigo.id);
        inserir_indice_diretor(filme_novo);
    }
}

void remover_de_todos_indices(int idFilme) {
    remover_indice_genero(idFilme);
    remover_indice_diretor(idFilme);
}

void fechar_indices() {
    if (arquivo_indice_genero != NULL) {
        fclose(arquivo_indice_genero);
        arquivo_indice_genero = NULL;
    }
    
    if (arquivo_indice_diretor != NULL) {
        fclose(arquivo_indice_diretor);
        arquivo_indice_diretor = NULL;
    }
}
