#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filme.h"

// auxiliar pra remover /n
void limpar_linha(char *str) {
    str[strcspn(str, "\n")] = '\0';
}

void exibir_menu() {
    printf("\n********************************\n");
    printf("   Streaming \n");
    printf("****************************\n");
    printf("1. Inserir Filme (Create)\n");
    printf("2. Buscar Filme por ID (Read)\n");
    printf("3. Atualizar Filme (Update)\n");
    printf("4. Remover Filme (Delete - Logico)\n");
    printf("5. Sair\n");
    printf("Escolha:");
}

int main() {
    FILE* arq = abrir_arquivo("dados_filmes.dat");
    if (arq == NULL) {
        printf("Erro fatal ao abrir o arquivo de dados.\n");
        return 1;
    }

    int opcao = 0;

    while (opcao != 5) {
        exibir_menu();
        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida.\n");
            break;
        }
        getchar(); // Limpa o buffer do teclado para os próximos inputs de texto

        switch (opcao) {
       case 1: { // CREATE
                Filme f;
                printf("\n--- INSERIR NOVO FILME ---\n");
                printf("ID (Inteiro): "); scanf("%d", &f.id); getchar();

                // --- NOVA VERIFICAÇÃO DE CHAVE PRIMÁRIA ---
                Filme existente = buscar_filme(arq, f.id);
                if (existente.id != -1) {
                    // Se o ID retornado não for -1, significa que o filme já existe e está ativo.
                    printf("\n>> Erro: O ID %d ja esta cadastrado no sistema! A Chave Primaria deve ser unica.\n", f.id);
                    break; // Sai do 'case 1' e volta para o menu principal
                }
                // ------------------------------------------

                printf("Titulo: "); fgets(f.titulo, 100, stdin); limpar_linha(f.titulo);
                printf("Diretor: "); fgets(f.diretor, 60, stdin); limpar_linha(f.diretor);
                printf("Ano de Lancamento: "); scanf("%d", &f.ano); getchar();
                printf("Genero: "); fgets(f.genero, 30, stdin); limpar_linha(f.genero);
                printf("Duracao (minutos): "); scanf("%d", &f.duracao); getchar();
                printf("Classificacao Etaria: "); fgets(f.classificacao, 5, stdin); limpar_linha(f.classificacao);
                printf("Nota (0.0 a 10.0): "); scanf("%f", &f.nota); getchar();

                inserir_filme(arq, f);
                
                //força a gravação no disco
                fflush(arq); 

                printf("\n>> Filme gravado com sucesso no disco!\n");
                break;
            }
            case 2: { // leitura do registro
                int id;
                printf("\nDigite o ID do filme que deseja: ");
                scanf("%d", &id); getchar();

                Filme f = buscar_filme(arq, id);
                if (f.id != -1) {
                    printf("\n** O filme foi encotnrado !!**\n");
                    printf("ID: %d\nTitulo: %s\nDiretor: %s\nAno: %d\nGenero: %s\nDuracao: %d min\nClassificacao: %s\nNota: %.1f\n", 
                           f.id, f.titulo, f.diretor, f.ano, f.genero, f.duracao, f.classificacao, f.nota);
                } else {
                    printf("\n>> Erro: Filme nao encontrado ou removido.\n");
                }
                break;
            }
            case 3: { // UPDATE
                int id;
                Filme novos_dados;
                printf("\nDigite o id do filme que vc deseja atualizar  ");
                scanf("%d", &id); getchar();

                // Verifica primeiro se o filme existe
                Filme existente = buscar_filme(arq, id);
                if (existente.id == -1) {
                    printf("\n>> Erro: Filme nao encontrado \n");
                    break;
                }

                printf("\n atualizando os dados...\n", id);
                printf("Novo Titulo: "); fgets(novos_dados.titulo, 100, stdin); limpar_linha(novos_dados.titulo);
                printf("Novo Diretor: "); fgets(novos_dados.diretor, 60, stdin); limpar_linha(novos_dados.diretor);
                printf("Novo Ano: "); scanf("%d", &novos_dados.ano); getchar();
                printf("Novo Genero: "); fgets(novos_dados.genero, 30, stdin); limpar_linha(novos_dados.genero);
                printf("Nova Duracao (min): "); scanf("%d", &novos_dados.duracao); getchar();
                printf("Nova Classificacao: "); fgets(novos_dados.classificacao, 5, stdin); limpar_linha(novos_dados.classificacao);
                printf("Nova Nota: "); scanf("%f", &novos_dados.nota); getchar();

                if (atualizar_filme(arq, id, novos_dados)) {
                    printf("\n>> Registro atualizado com sucesso no disco!\n");
                }
                break;
            }
            case 4: { // DELETE
                int id;
                printf("\nDigite o ID do filme que deseja REMOVER: ");
                scanf("%d", &id); getchar();

                if (remover_filme(arq, id)) {
                    printf("\n>> Filme removido logicamente (Flag LED alterada).\n");
                } else {
                    printf("\n>> Erro: Filme nao encontrado.\n");
                }
                break;
            }
            case 5:
                printf("\nFechando sistema de streaming...\n");
                break;
            default:
                printf("\nOpcao invalida! Tente novamente.\n");
        }
    }

    fechar_arquivo(arq);
    return 0;
}