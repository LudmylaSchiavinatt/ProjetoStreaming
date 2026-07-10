#include "indice.h"
#include "filme.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// constantes e variaveis globais

#define NOME_INDICE_GENERO "indice_genero.dat"
#define NOME_INDICE_DIRETOR "indice_diretor.dat"
#define TAM_CABECALHO_INDICE (sizeof(long))

// Arquivos de índice (abertos globalmente)
static FILE* arquivo_indice_genero = NULL;
static FILE* arquivo_indice_diretor = NULL;

//função auxiliar externa

// Abre ou cria um arquivo de índice
static FILE* abrir_arquivo_indice(const char* nome) {
    FILE* arq = fopen(nome, "rb+");
    
    if (arq == NULL) {
        // Arquivo não existe, cria com cabeçalho
        arq = fopen(nome, "wb+");
        if (arq != NULL) {
            long cabeca = -1;
            fwrite(&cabeca, sizeof(long), 1, arq);
            fflush(arq);
        }
    }
    
    return arq;
}

// Busca uma chave no arquivo de índice
static long buscar_chave_indice(FILE* arq, const char* chave) {
    IndiceSecundario entrada;
    long offset = sizeof(long);  // Pula o cabeçalho
    
    fseek(arq, offset, SEEK_SET);
    
    while (fread(&entrada, sizeof(IndiceSecundario), 1, arq) == 1) {
        if (strcmp(entrada.chave, chave) == 0) {
            return offset;  // Retorna o offset onde a chave está
        }
        offset = ftell(arq);
    }
    
    return -1;  // Chave não encontrada
}

// Insere uma nova chave no índice
static void inserir_chave_indice(FILE* arq, const char* chave, int idFilme) {
    // Verifica se a chave já existe
    long offset_chave = buscar_chave_indice(arq, chave);
    
    if (offset_chave != -1) {
        // Chave já existe, adiciona à lista invertida
        IndiceSecundario entrada;
        fseek(arq, offset_chave, SEEK_SET);
        fread(&entrada, sizeof(IndiceSecundario), 1, arq);
        
        // Cria novo nó da lista invertida
        NoListaInvertida novo_no;
        novo_no.idFilme = idFilme;
        novo_no.proximo = entrada.primeiro;  // Insere no início (LIFO)
        
        // Escreve o novo nó no final do arquivo
        fseek(arq, 0, SEEK_END);
        long offset_no = ftell(arq);
        fwrite(&novo_no, sizeof(NoListaInvertida), 1, arq);
        
        // Atualiza a entrada do índice para apontar para o novo nó
        entrada.primeiro = offset_no;
        fseek(arq, offset_chave, SEEK_SET);
        fwrite(&entrada, sizeof(IndiceSecundario), 1, arq);
        
        fflush(arq);
        return;
    }
    
    // Chave não existe, cria nova entrada
    IndiceSecundario nova_entrada;
    strcpy(nova_entrada.chave, chave);
    
    // Cria o primeiro nó da lista invertida
    NoListaInvertida novo_no;
    novo_no.idFilme = idFilme;
    novo_no.proximo = -1;
    
    // Escreve o nó no final do arquivo
    fseek(arq, 0, SEEK_END);
    long offset_no = ftell(arq);
    fwrite(&novo_no, sizeof(NoListaInvertida), 1, arq);
    
    // Configura a entrada do índice
    nova_entrada.primeiro = offset_no;
    
    // Escreve a entrada no final do arquivo
    fseek(arq, 0, SEEK_END);
    fwrite(&nova_entrada, sizeof(IndiceSecundario), 1, arq);
    
    fflush(arq);
}

// Remove um ID da lista invertida de uma chave
static void remover_id_da_chave(FILE* arq, const char* chave, int idFilme) {
    long offset_chave = buscar_chave_indice(arq, chave);
    
    if (offset_chave == -1) {
        return;  // Chave não encontrada
    }
    
    // Lê a entrada do índice
    IndiceSecundario entrada;
    fseek(arq, offset_chave, SEEK_SET);
    fread(&entrada, sizeof(IndiceSecundario), 1, arq);
    
    // Percorre a lista invertida procurando o ID
    long atual = entrada.primeiro;
    long anterior = -1;
    
    while (atual != -1) {
        NoListaInvertida no;
        fseek(arq, atual, SEEK_SET);
        fread(&no, sizeof(NoListaInvertida), 1, arq);
        
        if (no.idFilme == idFilme) {
            // Remove o nó da lista
            if (anterior == -1) {
                // É o primeiro nó
                entrada.primeiro = no.proximo;
                fseek(arq, offset_chave, SEEK_SET);
                fwrite(&entrada, sizeof(IndiceSecundario), 1, arq);
            } else {
                // Não é o primeiro, atualiza o anterior
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

// implementacao das funções publicas
//indice por genero

void inicializar_indice_genero() {
    if (arquivo_indice_genero == NULL) {
        arquivo_indice_genero = abrir_arquivo_indice(NOME_INDICE_GENERO);
    }
}

void inserir_indice_genero(Filme filme) {
    if (arquivo_indice_genero == NULL) {
        inicializar_indice_genero();
    }
    
    // Normaliza o gênero (converte para minúsculo)
    char genero_normalizado[60];
    strcpy(genero_normalizado, filme.genero);
    for (int i = 0; genero_normalizado[i]; i++) {
        genero_normalizado[i] = tolower(genero_normalizado[i]);
    }
    
    inserir_chave_indice(arquivo_indice_genero, genero_normalizado, filme.id);
}

void remover_indice_genero(int idFilme) {
    // Precisamos descobrir qual gênero tem esse filme
    // Por enquanto, vamos buscar o filme no arquivo principal
    FILE* arq_dados = abrir_arquivo("dados_filmes.dat");
    if (arq_dados == NULL) return;
    
    Filme f = buscar_filme(arq_dados, idFilme);
    fechar_arquivo(arq_dados);
    
    if (f.id == -1) return;
    
    // Normaliza o gênero
    char genero_normalizado[60];
    strcpy(genero_normalizado, f.genero);
    for (int i = 0; genero_normalizado[i]; i++) {
        genero_normalizado[i] = tolower(genero_normalizado[i]);
    }
    
    remover_id_da_chave(arquivo_indice_genero, genero_normalizado, idFilme);
}

void buscar_por_genero(const char *genero) {
    if (arquivo_indice_genero == NULL) {
        inicializar_indice_genero();
    }
    
    // Normaliza o gênero
    char genero_normalizado[60];
    strcpy(genero_normalizado, genero);
    for (int i = 0; genero_normalizado[i]; i++) {
        genero_normalizado[i] = tolower(genero_normalizado[i]);
    }
    
    // Busca a chave no índice
    long offset_chave = buscar_chave_indice(arquivo_indice_genero, genero_normalizado);
    
    if (offset_chave == -1) {
        printf("\n>>> Nenhum filme encontrado para o genero: %s\n", genero);
        return;
    }
    
    // Lê a entrada do índice
    IndiceSecundario entrada;
    fseek(arquivo_indice_genero, offset_chave, SEEK_SET);
    fread(&entrada, sizeof(IndiceSecundario), 1, arquivo_indice_genero);
    
    // Abre o arquivo de dados para buscar os filmes
    FILE* arq_dados = abrir_arquivo("dados_filmes.dat");
    if (arq_dados == NULL) return;
    
    printf("\n=== FILMES DO GENERO: %s ===\n", genero);
    printf("----------------------------------------\n");
    
    // Percorre a lista invertida
    long atual = entrada.primeiro;
    int contador = 0;
    
    while (atual != -1) {
        NoListaInvertida no;
        fseek(arquivo_indice_genero, atual, SEEK_SET);
        fread(&no, sizeof(NoListaInvertida), 1, arquivo_indice_genero);
        
        // Busca o filme pelo ID
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

//indice por diretor

void inicializar_indice_diretor() {
    if (arquivo_indice_diretor == NULL) {
        arquivo_indice_diretor = abrir_arquivo_indice(NOME_INDICE_DIRETOR);
    }
}

void inserir_indice_diretor(Filme filme) {
    if (arquivo_indice_diretor == NULL) {
        inicializar_indice_diretor();
    }
    
    // Normaliza o diretor (converte para minúsculo)
    char diretor_normalizado[60];
    strcpy(diretor_normalizado, filme.diretor);
    for (int i = 0; diretor_normalizado[i]; i++) {
        diretor_normalizado[i] = tolower(diretor_normalizado[i]);
    }
    
    inserir_chave_indice(arquivo_indice_diretor, diretor_normalizado, filme.id);
}

void remover_indice_diretor(int idFilme) {
    FILE* arq_dados = abrir_arquivo("dados_filmes.dat");
    if (arq_dados == NULL) return;
    
    Filme f = buscar_filme(arq_dados, idFilme);
    fechar_arquivo(arq_dados);
    
    if (f.id == -1) return;
    
    char diretor_normalizado[60];
    strcpy(diretor_normalizado, f.diretor);
    for (int i = 0; diretor_normalizado[i]; i++) {
        diretor_normalizado[i] = tolower(diretor_normalizado[i]);
    }
    
    remover_id_da_chave(arquivo_indice_diretor, diretor_normalizado, idFilme);
}

void buscar_por_diretor(const char *diretor) {
    if (arquivo_indice_diretor == NULL) {
        inicializar_indice_diretor();
    }
    
    char diretor_normalizado[60];
    strcpy(diretor_normalizado, diretor);
    for (int i = 0; diretor_normalizado[i]; i++) {
        diretor_normalizado[i] = tolower(diretor_normalizado[i]);
    }
    
    long offset_chave = buscar_chave_indice(arquivo_indice_diretor, diretor_normalizado);
    
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
    if (strcmp(filme_antigo.genero, filme_novo.genero) != 0) {
        remover_indice_genero(filme_antigo.id);
        inserir_indice_genero(filme_novo);
    }
    
    // Se o diretor mudou
    if (strcmp(filme_antigo.diretor, filme_novo.diretor) != 0) {
        remover_indice_diretor(filme_antigo.id);
        inserir_indice_diretor(filme_novo);
    }
}

void remover_de_todos_indices(int idFilme) {
    remover_indice_genero(idFilme);
    remover_indice_diretor(idFilme);
}

//função para fechar os arq (chamar no final)
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