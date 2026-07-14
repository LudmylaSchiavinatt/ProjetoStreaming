#include "arvoreb.h"
#include <stdio.h>
#include <stdlib.h>

#define ARQUIVO_ARVORE "arvore_b.dat"

static FILE* arq_arvore = NULL;
static long offset_raiz = -1; // Guardado no cabeçalho


static long alocar_no(NoArvoreB* no);
static int inserir_recursivo(long offset_no, int id, long offset_dado,
                              int* chave_promovida, long* offset_dado_promovido,
                              long* offset_novo_no);

// Abre o arquivo e lê o cabeçalho (ou cria se não existir)
void inicializar_arvore_b() {
    if (arq_arvore == NULL) {
        arq_arvore = fopen(ARQUIVO_ARVORE, "rb+");
        if (arq_arvore == NULL) {
            arq_arvore = fopen(ARQUIVO_ARVORE, "wb+");
            offset_raiz = -1;
            // Grava o cabeçalho: 1 long (8 bytes) guardando a raiz
            fwrite(&offset_raiz, sizeof(long), 1, arq_arvore);
            fflush(arq_arvore);
        } else {
            // Lê o cabeçalho existente
            fseek(arq_arvore, 0, SEEK_SET);
            fread(&offset_raiz, sizeof(long), 1, arq_arvore);
        }
    }
}

// Funções utilitárias cruciais para manipular os nós no disco
void salvar_no(long offset, NoArvoreB* no) {
    fseek(arq_arvore, offset, SEEK_SET);
    fwrite(no, sizeof(NoArvoreB), 1, arq_arvore);
    fflush(arq_arvore);
}

void ler_no(long offset, NoArvoreB* no) {
    fseek(arq_arvore, offset, SEEK_SET);
    fread(no, sizeof(NoArvoreB), 1, arq_arvore);
}

void salvar_cabecalho(long novo_offset_raiz) {
    offset_raiz = novo_offset_raiz;
    fseek(arq_arvore, 0, SEEK_SET);
    fwrite(&offset_raiz, sizeof(long), 1, arq_arvore);
    fflush(arq_arvore);
}

// Grava um nó novo no final do arquivo e devolve o offset onde ele ficou
static long alocar_no(NoArvoreB* no) {
    fseek(arq_arvore, 0, SEEK_END);
    long offset = ftell(arq_arvore);
    fwrite(no, sizeof(NoArvoreB), 1, arq_arvore);
    fflush(arq_arvore);
    return offset;
}

// ==========================================
// BUSCA
// ==========================================
long buscar_arvore_b(int id) {
    if (offset_raiz == -1) return -1; // Árvore vazia
    long offset_atual = offset_raiz;
    NoArvoreB no_atual;
    while (offset_atual != -1) {
        ler_no(offset_atual, &no_atual);
        int i = 0;

        // Procura a chave no nó atual
        while (i < no_atual.num_chaves && id > no_atual.chaves[i]) {
            i++;
        }
        // Se encontrou a chave
        if (i < no_atual.num_chaves && id == no_atual.chaves[i]) {
            return no_atual.offsets_dados[i]; // Retorna o ponteiro para dados_filmes.dat
        }
        // Se é folha e não achou
        if (no_atual.eh_folha) {
            return -1;
        }
        // Desce para o filho correspondente
        offset_atual = no_atual.filhos[i];
    }
    return -1;
}


// INSERÇÃO (split "postecipado": insere e, se estourar, divide)

// Retorna 1 se houve split neste nível (preenchendo os parâmetros de saída
// com a chave/offset promovidos e o offset do novo nó à direita), ou 0
// se a inserção coube sem precisar dividir nada.
static int inserir_recursivo(long offset_no, int id, long offset_dado,
                              int* chave_promovida, long* offset_dado_promovido,
                              long* offset_novo_no) {
    NoArvoreB no;
    ler_no(offset_no, &no);

    // Acha a posição i onde o id se encaixa (ou já existe)
    int i = 0;
    while (i < no.num_chaves && id > no.chaves[i]) i++;

    // Chave duplicada: apenas atualiza o offset de dados, sem inserir de novo
    if (i < no.num_chaves && no.chaves[i] == id) {
        no.offsets_dados[i] = offset_dado;
        salvar_no(offset_no, &no);
        return 0;
    }

    if (no.eh_folha) {
        // Cabe sem split
        if (no.num_chaves < ORDEM - 1) {
            for (int k = no.num_chaves; k > i; k--) {
                no.chaves[k] = no.chaves[k - 1];
                no.offsets_dados[k] = no.offsets_dados[k - 1];
            }
            no.chaves[i] = id;
            no.offsets_dados[i] = offset_dado;
            no.num_chaves++;
            salvar_no(offset_no, &no);
            return 0;
        }

        // Overflow: monta arrays temporárias com ORDEM chaves e divide
        int temp_chaves[ORDEM];
        long temp_offsets[ORDEM];
        int k;
        for (k = 0; k < i; k++) {
            temp_chaves[k] = no.chaves[k];
            temp_offsets[k] = no.offsets_dados[k];
        }
        temp_chaves[i] = id;
        temp_offsets[i] = offset_dado;
        for (k = i; k < no.num_chaves; k++) {
            temp_chaves[k + 1] = no.chaves[k];
            temp_offsets[k + 1] = no.offsets_dados[k];
        }

        int mid = ORDEM / 2;
        NoArvoreB no_esq, no_dir;
        no_esq.eh_folha = 1;
        no_dir.eh_folha = 1;

        no_esq.num_chaves = mid;
        for (k = 0; k < mid; k++) {
            no_esq.chaves[k] = temp_chaves[k];
            no_esq.offsets_dados[k] = temp_offsets[k];
        }

        no_dir.num_chaves = (ORDEM - 1) - mid;
        for (k = 0; k < no_dir.num_chaves; k++) {
            no_dir.chaves[k] = temp_chaves[mid + 1 + k];
            no_dir.offsets_dados[k] = temp_offsets[mid + 1 + k];
        }

        salvar_no(offset_no, &no_esq);           // reaproveita o offset original
        long offset_dir = alocar_no(&no_dir);    // nó novo no final do arquivo

        *chave_promovida = temp_chaves[mid];
        *offset_dado_promovido = temp_offsets[mid];
        *offset_novo_no = offset_dir;
        return 1;
    }

    // Nó interno: desce recursivamente para o filho i
    int chave_prom;
    long offset_dado_prom, offset_novo;
    int houve_split = inserir_recursivo(no.filhos[i], id, offset_dado,
                                         &chave_prom, &offset_dado_prom, &offset_novo);

    if (!houve_split) {
        return 0; // filho já resolveu tudo e se salvou sozinho
    }


    ler_no(offset_no, &no); // relê, pois nada mudou aqui ainda

    if (no.num_chaves < ORDEM - 1) {
        for (int k = no.num_chaves; k > i; k--) {
            no.chaves[k] = no.chaves[k - 1];
            no.offsets_dados[k] = no.offsets_dados[k - 1];
        }
        for (int k = no.num_chaves + 1; k > i + 1; k--) {
            no.filhos[k] = no.filhos[k - 1];
        }
        no.chaves[i] = chave_prom;
        no.offsets_dados[i] = offset_dado_prom;
        no.filhos[i + 1] = offset_novo;
        no.num_chaves++;
        salvar_no(offset_no, &no);
        return 0;
    }

    // Overflow no nó interno: monta temporárias e divide também
    int temp_chaves[ORDEM];
    long temp_offsets[ORDEM];
    long temp_filhos[ORDEM + 1];
    int k;

    for (k = 0; k < i; k++) {
        temp_chaves[k] = no.chaves[k];
        temp_offsets[k] = no.offsets_dados[k];
    }
    for (k = 0; k <= i; k++) {
        temp_filhos[k] = no.filhos[k];
    }
    temp_chaves[i] = chave_prom;
    temp_offsets[i] = offset_dado_prom;
    temp_filhos[i + 1] = offset_novo;

    for (k = i; k < no.num_chaves; k++) {
        temp_chaves[k + 1] = no.chaves[k];
        temp_offsets[k + 1] = no.offsets_dados[k];
    }
    for (k = i + 1; k <= no.num_chaves; k++) {
        temp_filhos[k + 1] = no.filhos[k];
    }

    int mid = ORDEM / 2;
    NoArvoreB no_esq, no_dir;
    no_esq.eh_folha = 0;
    no_dir.eh_folha = 0;

    no_esq.num_chaves = mid;
    for (k = 0; k < mid; k++) {
        no_esq.chaves[k] = temp_chaves[k];
        no_esq.offsets_dados[k] = temp_offsets[k];
    }
    for (k = 0; k <= mid; k++) {
        no_esq.filhos[k] = temp_filhos[k];
    }

    no_dir.num_chaves = (ORDEM - 1) - mid;
    for (k = 0; k < no_dir.num_chaves; k++) {
        no_dir.chaves[k] = temp_chaves[mid + 1 + k];
        no_dir.offsets_dados[k] = temp_offsets[mid + 1 + k];
    }
    int total_filhos_dir = (ORDEM + 1) - (mid + 1);
    for (k = 0; k < total_filhos_dir; k++) {
        no_dir.filhos[k] = temp_filhos[mid + 1 + k];
    }

    salvar_no(offset_no, &no_esq);
    long offset_dir = alocar_no(&no_dir);

    *chave_promovida = temp_chaves[mid];
    *offset_dado_promovido = temp_offsets[mid];
    *offset_novo_no = offset_dir;
    return 1;
}

void inserir_arvore_b(int id, long offset_dado) {
    // Árvore vazia: cria a primeira folha (raiz)
    if (offset_raiz == -1) {
        NoArvoreB novo;
        novo.num_chaves = 1;
        novo.chaves[0] = id;
        novo.offsets_dados[0] = offset_dado;
        novo.eh_folha = 1;
        long offset_novo = alocar_no(&novo);
        salvar_cabecalho(offset_novo);
        return;
    }

    int chave_prom;
    long offset_dado_prom, offset_novo_no;
    int houve_split = inserir_recursivo(offset_raiz, id, offset_dado,
                                         &chave_prom, &offset_dado_prom, &offset_novo_no);

    if (houve_split) {
        // A raiz antiga dividiu -> cria uma nova raiz com 1 chave e 2 filhos
        NoArvoreB nova_raiz;
        nova_raiz.num_chaves = 1;
        nova_raiz.chaves[0] = chave_prom;
        nova_raiz.offsets_dados[0] = offset_dado_prom;
        nova_raiz.filhos[0] = offset_raiz;
        nova_raiz.filhos[1] = offset_novo_no;
        nova_raiz.eh_folha = 0;
        long offset_nova_raiz = alocar_no(&nova_raiz);
        salvar_cabecalho(offset_nova_raiz);
    }
}


void remover_arvore_b(int id) {
    if (offset_raiz == -1) return; // arvore vazia

    long offset_atual = offset_raiz;
    NoArvoreB no_atual;

    while (offset_atual != -1) {
        ler_no(offset_atual, &no_atual);
        int i = 0;
        while (i < no_atual.num_chaves && id > no_atual.chaves[i]) i++;

        if (i < no_atual.num_chaves && id == no_atual.chaves[i]) {
            no_atual.offsets_dados[i] = -1; // remoção lógica
            salvar_no(offset_atual, &no_atual);
            return;
        }

        if (no_atual.eh_folha) {
            return; // não encontrado
        }
        offset_atual = no_atual.filhos[i];
    }
}

void fechar_arvore_b() {
    if (arq_arvore != NULL) {
        fclose(arq_arvore);
        arq_arvore = NULL;
    }
}