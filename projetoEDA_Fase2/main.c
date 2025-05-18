/*****************************************************************//**
 * \file   main.c
 * \brief  Ficheiro principal para o processamento de antenas
 * 
 * \author Vitor Moreira 31553
 * \date   15 May 2025
 *********************************************************************/
#include "grafos.h"

int main() {
    printf("Iniciando processamento...\n");
    Grafo grafo = CarregarAntenasDoFicheiro("mapa.txt");

    if (grafo.total_antenas == 0) {
        printf("Nenhuma antena carregada. Verifique o arquivo de entrada.\n");
        return 1;
    }

    printf("Exportando resultados para resultado.txt...\n");
    ExportarResultados(grafo, "resultados.txt");

    LibertarGrafo(&grafo);
    printf("Processamento concluído com sucesso!\n");
    return 0;
}
