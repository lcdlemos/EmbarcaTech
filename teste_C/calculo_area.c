#include <stdio.h>
#include <conio.h>

#include "area.c"
#include "volume.c"

int main(){

    int lado = 5;
    int area, volume;
    char nome[10];

    printf("\nInforme seu nome: ");
    scanf("%s", nome);

    printf("\nOi, %s, Vamos Calcular Area de um Quadrado e Volume de um Cubo\n", nome);

    area = calcula_area(lado);
    volume = calcula_volume(lado);

    printf("\n Area do quadrado de lado %i m sera %i m2", lado, area);
    printf("\n Volume do cubo de aresta %i sera %i m3.", lado, volume);

    printf("\n\n");
}