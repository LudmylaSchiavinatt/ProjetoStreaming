#include "filme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FILE* abrir_arquivo(const char* nome_arquivo) {
    FILE* arq = fopen(nome_arquivo, "rb+");
    if (arq == NULL) {
        arq = fopen(nome_arquivo, "wb+");
    }
    return arq;
}

void fechar_arquivo(FILE* arq) {
    if (arq != NULL) {
        fclose(arq);
    }
}


void inserir_filme(FILE* arq, Filme f) {
    f.removido = '0'; // '0'registro ativo
    
    fseek(arq, 0, SEEK_END);
    
    fwrite(&f, sizeof(Filme), 1, arq);
}

//
Filme buscar_filme(FILE* arq, int id_procurado) {
    Filme f;
    rewind(arq); 

    while (fread(&f, sizeof(Filme), 1, arq) == 1) {
        if (f.id == id_procurado && f.removido == '0') {
            return f; // Retorna o filme encontrado
        }
    }

    f.id = -1;
    return f;
}

int atualizar_filme(FILE* arq, int id_procurado, Filme novos_dados) {
    Filme f;
    rewind(arq);

    while (fread(&f, sizeof(Filme), 1, arq) == 1) {
        if (f.id == id_procurado && f.removido == '0') {
            novos_dados.id = f.id; 
            novos_dados.removido = '0';
            fseek(arq, -sizeof(Filme), SEEK_CUR);
            fwrite(&novos_dados, sizeof(Filme), 1, arq);
            return 1; // Sucesso
        }
    }
    return 0; // o flme nao foi encontrado
}

int remover_filme(FILE* arq, int id_procurado) {
    Filme f;
    rewind(arq);

    while (fread(&f, sizeof(Filme), 1, arq) == 1) {
        if (f.id == id_procurado && f.removido == '0') {
            f.removido = '1'; // LED
            fseek(arq, -sizeof(Filme), SEEK_CUR);
            fwrite(&f, sizeof(Filme), 1, arq);
            return 1; // Sucesso na remoção
        }
    }
    return 0; //
}