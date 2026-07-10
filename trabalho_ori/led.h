#ifndef LED_H
#define LED_H

#include <stdio.h>

// Aloca um espaço disponível da LED (retorna o offset ou -1 se vazia)
long led_alocar_espaco(FILE *f);

// Insere um novo offset livre na cabeça da LED (Estratégia LIFO)
void led_liberar_espaco(FILE *f, long offset_registro);

// Conta quantos elementos estão atualmente encadeados na LED
long led_contar_espacos_livres(FILE *f);

#endif
