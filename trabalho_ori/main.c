#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filme.h"
#include "indice.h"  
#include "arvore_b.h"
#include "led.h" // Necessário para a opção 7 (contar_espacos_livres)

// auxiliar pra remover \n e \r (evita bugs no Windows)
void limpar_linha(char *str) {
    str[strcspn(str, "\r\n")] = '\0';
}

void exibir_menu() {
    printf("\n********************************\n");
    printf("   Streaming \n");
    printf("****************************\n");
    printf("1. Inserir Filme (Create)\n");
    printf("2. Buscar Filme por ID (Read)\n");
    printf("3. Atualizar Filme (Update)\n");
    printf("4. Remover Filme (Delete - Logico)\n");
    printf("5. Buscar por Genero (Indice Secundario)\n");
    printf("6. Buscar por Diretor (Indice Secundario)\n");
    printf("7. Mostrar Espacos Livres (LED)\n");
    printf("8. Visualizar Arvore B\n");
    printf("9. Sair\n");
    printf("Escolha:");
}

int main() {
    // 1. INICIALIZA A ÁRVORE B LOGO NO INÍCIO
    inicializar_arvore_b();

    FILE* arq = abrir_arquivo("dados_filmes.dat");
    if (arq == NULL) {
        printf("Erro fatal ao abrir o arquivo de dados.\n");
        fechar_arvore_b(); // Garante o fechamento correto caso falhe
        return 1;
    }

    // Inicializa os indices secundarios
    inicializar_indice_genero();
    inicializar_indice_diretor();
    
    int opcao = 0;

    while (opcao != 9) {  // Sai com 9
        exibir_menu();
        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida.\n");
            break;
        }
        getchar(); // Limpa o buffer do teclado

        switch (opcao) {
            case 1: { // CREATE
                Filme f;
                printf("\n--- INSERIR NOVO FILME ---\n");
                printf("ID (Inteiro): "); scanf("%d", &f.id); getchar();

                // Verifica na Árvore B se o ID já existe
                long offset_existente = buscar_arvore_b(f.id);
                if (offset_existente != -1) {
                    printf("\n>> Erro: O ID %d ja esta cadastrado no sistema! A Chave Primaria deve ser unica.\n", f.id);
                    break;
                }

                printf("Titulo: "); fgets(f.titulo, 100, stdin); limpar_linha(f.titulo);
                printf("Diretor: "); fgets(f.diretor, 60, stdin); limpar_linha(f.diretor);
                printf("Ano de Lancamento: "); scanf("%d", &f.ano); getchar();
                printf("Genero: "); fgets(f.genero, 30, stdin); limpar_linha(f.genero);
                printf("Duracao (minutos): "); scanf("%d", &f.duracao); getchar();
                printf("Classificacao Etaria: "); fgets(f.classificacao, 5, stdin); limpar_linha(f.classificacao);
                printf("Nota (0.0 a 10.0): "); scanf("%f", &f.nota); getchar();

                inserir_filme(arq, f);
                inserir_indice_genero(f);
                inserir_indice_diretor(f);
                
                fflush(arq); 

                printf("\n>> Filme gravado com sucesso no disco!\n");
                break;
            }
            
            case 2: { // READ
                int id;
                printf("\nDigite o ID do filme que deseja: ");
                scanf("%d", &id); getchar();

                Filme f = buscar_filme(arq, id);
                if (f.id != -1) {
                    printf("\n** O filme foi encontrado !!**\n");
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
                printf("\nDigite o id do filme que vc deseja atualizar: ");
                scanf("%d", &id); getchar();

                Filme existente = buscar_filme(arq, id);
                if (existente.id == -1) {
                    printf("\n>> Erro: Filme nao encontrado \n");
                    break;
                }

                printf("\nAtualizando os dados do filme ID %d:\n", id);
                printf("Novo Titulo: "); fgets(novos_dados.titulo, 100, stdin); limpar_linha(novos_dados.titulo);
                printf("Novo Diretor: "); fgets(novos_dados.diretor, 60, stdin); limpar_linha(novos_dados.diretor);
                printf("Novo Ano: "); scanf("%d", &novos_dados.ano); getchar();
                printf("Novo Genero: "); fgets(novos_dados.genero, 30, stdin); limpar_linha(novos_dados.genero);
                printf("Nova Duracao (min): "); scanf("%d", &novos_dados.duracao); getchar();
                printf("Nova Classificacao: "); fgets(novos_dados.classificacao, 5, stdin); limpar_linha(novos_dados.classificacao);
                printf("Nova Nota: "); scanf("%f", &novos_dados.nota); getchar();

                if (atualizar_filme(arq, id, novos_dados)) {
                    atualizar_indices(existente, novos_dados);
                    printf("\n>> Registro atualizado com sucesso no disco!\n");
                }
                break;
            }
            
            case 4: { // DELETE
                int id;
                printf("\nDigite o ID do filme que deseja REMOVER: ");
                scanf("%d", &id); getchar();

                // Resgata os dados completos antes de apagar, para atualizar os indices corretos
                Filme existente = buscar_filme(arq, id); 
                if (existente.id != -1) {
                    if (remover_filme(arq, id)) {
                        remover_de_todos_indices(existente); 
                        printf("\n>> Filme removido e indices atualizados.\n");
                    }
                } else {
                    printf("\n>> Erro: Filme nao encontrado.\n");
                }
                break;
            }
            
            case 5: { // BUSCAR GÊNERO
                char genero[60];
                printf("\n--- BUSCAR POR GENERO ---\n");
                printf("Digite o genero: ");
                fgets(genero, 60, stdin);
                limpar_linha(genero);
                
                buscar_por_genero(arq, genero);
                break;
            }
            
            case 6: { // BUSCAR DIRETOR
                char diretor[60];
                printf("\n--- BUSCAR POR DIRETOR ---\n");
                printf("Digite o nome do diretor: ");
                fgets(diretor, 60, stdin);
                limpar_linha(diretor);
                
                buscar_por_diretor(arq, diretor);
                break;
            }
            
            case 7: { // MOSTRAR ESPAÇOS LIVRES (LED)
                printf("\n--- STATUS DA LED ---\n");
                printf(">> Espacos livres para reaproveitamento: %ld\n", contar_espacos_livres(arq));
                break;
            }
            
            case 8: { // VISUALIZAR ÁRVORE B
                printf("\n--- ESTRUTURA DA ARVORE B ---\n");
                visualizar_arvore();
                break;
            }
            
            case 9: // SAIR 
                printf("\nFechando sistema de streaming.\n");
                break;
                
            default:
                printf("\nOpcao invalida! Tente novamente.\n");
        }
    }

    // Fecha os indices secundarios
    fechar_indices();
    
    // Fecha a Árvore B antes de terminar
    fechar_arvore_b();

    // Fecha arquivo principal
    fechar_arquivo(arq);
    return 0;
}
