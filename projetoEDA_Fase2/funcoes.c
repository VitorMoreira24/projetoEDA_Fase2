/*****************************************************************//**
 * \file   funcoes.c
 * \brief  Ficheiro de implementação para as funções de processamento
 * 
 * \author Vitor Moreira 31553
 * \date   15 May 2025
 *********************************************************************/
#include "grafos.h"
/***************************************************************
 * Função para adicionar uma adjacência a uma antena
 * @param antena -  Ponteiro para a antena
 * @param destino - Índice da antena adjacente
 ***************************************************************/
void AdicionarAdjacencia(Antena* antena, int destino) {
    Adjacencia* nova = (Adjacencia*)malloc(sizeof(Adjacencia));
    nova->destino = destino;
    nova->proxima = antena->conexoes;
    antena->conexoes = nova;
}

/***************************************************************
 * Função para carregar antenas de um arquivo
 * @param nome_ficheiro - Nome do arquivo a ser lido
 * @return Grafo - Estrutura contendo as antenas e suas conexões
 ***************************************************************/
Grafo CarregarAntenasDoFicheiro(const char* nome_ficheiro) {
    FILE* ficheiro = fopen(nome_ficheiro, "r");
    Grafo grafo = { .total_antenas = 0 };

    if (!ficheiro) {
        perror("Erro ao abrir ficheiro");
        return grafo;
    }

    int linhas, colunas;
    if (fscanf(ficheiro, "%d %d", &linhas, &colunas) != 2) {
        fclose(ficheiro);
        fprintf(stderr, "Formato de ficheiro invalido\n");
        return grafo;
    }

    fgetc(ficheiro); // Consumir quebra de linha

    char linha[MAX_COLUNAS + 2];
    for (int y = 0; y < linhas && fgets(linha, sizeof(linha), ficheiro); y++) {
        linha[strcspn(linha, "\n")] = '\0';
        for (int x = 0; x < colunas && x < (int)strlen(linha); x++) {
            if (linha[x] != '.' && linha[x] != ' ') {
                grafo.antenas[grafo.total_antenas] = (Antena){
                    .frequencia = linha[x],
                    .coluna = x,
                    .linha = y,
                    .conexoes = NULL,
                    .visitada = false
                };
                grafo.total_antenas++;
            }
        }
    }
    fclose(ficheiro);

    // Conectar antenas da mesma frequência
    for (int i = 0; i < grafo.total_antenas; i++) {
        for (int j = i + 1; j < grafo.total_antenas; j++) {
            if (grafo.antenas[i].frequencia == grafo.antenas[j].frequencia) {
                AdicionarAdjacencia(&grafo.antenas[i], j);
                AdicionarAdjacencia(&grafo.antenas[j], i);
            }
        }
    }

    return grafo;
}
/***************************************************************
 * Função para limpar os estados de visitados das antenas
 * @param grafo - Ponteiro para o grafo
 ***************************************************************/
void LimparVisitados(Grafo* grafo) {
    for (int i = 0; i < grafo->total_antenas; i++) {
        grafo->antenas[i].visitada = false;
    }
}
/***************************************************************
* Função para realizar a busca em profundidade
* @param grafo - Ponteiro para o grafo
* @param atual - Índice da antena atual
* @param saida - Ponteiro para o arquivo de saída
***************************************************************/
void ProcuraEmProfundidade(Grafo* grafo, int atual, FILE* saida) {
    grafo->antenas[atual].visitada = true;
    Antena a = grafo->antenas[atual];
    fprintf(saida, "Antena %c @ (%d,%d)\n", a.frequencia, a.coluna, a.linha);

    for (Adjacencia* adj = a.conexoes; adj != NULL; adj = adj->proxima) {
        if (!grafo->antenas[adj->destino].visitada) {
            ProcuraEmProfundidade(grafo, adj->destino, saida);
        }
    }
}
/***************************************************************
 * Função para realizar a busca em largura
 * @param grafo - Ponteiro para o grafo
 * @param inicio - Índice da antena inicial
 * @param saida - Ponteiro para o arquivo de saída
 ***************************************************************/
void ProcuraEmLargura(Grafo* grafo, int inicio, FILE* saida) {
    int fila[MAX_ANTENAS];
    int frente = 0, tras = 0;

    grafo->antenas[inicio].visitada = true;
    fila[tras++] = inicio;

    while (frente < tras) {
        int atual = fila[frente++];
        Antena a = grafo->antenas[atual];
        fprintf(saida, "Antena %c @ (%d,%d)\n", a.frequencia, a.coluna, a.linha);

        for (Adjacencia* adj = a.conexoes; adj != NULL; adj = adj->proxima) {
            if (!grafo->antenas[adj->destino].visitada) {
                grafo->antenas[adj->destino].visitada = true;
                fila[tras++] = adj->destino;
            }
        }
    }
}
/***************************************************************
 * Função recursiva para encontrar caminhos entre antenas
 * @param grafo - Ponteiro para o grafo
 * @param atual - Índice da antena atual
 * @param destino - Índice da antena de destino
 * @param visitados - Array de controle de visitados
 * @param caminho - Array para armazenar o caminho atual
 * @param pos - Posição atual no caminho
 * @param saida - Ponteiro para o arquivo de saída
 * @param caminho_encontrado - Flag para indicar se um caminho foi encontrado
 ***************************************************************/
void EncontrarCaminhosRecursiva(Grafo* grafo, int atual, int destino, bool visitados[],
    int caminho[], int pos, FILE* saida, bool* caminho_encontrado) {
    visitados[atual] = true;
    caminho[pos] = atual;
    pos++;

    if (atual == destino) {
        *caminho_encontrado = true;
        for (int i = 0; i < pos; i++) {
            Antena a = grafo->antenas[caminho[i]];
            fprintf(saida, "%c(%d,%d)", a.frequencia, a.coluna, a.linha);
            if (i < pos - 1) fprintf(saida, " -> ");
        }
        fprintf(saida, "\n");
    }
    else {
        for (Adjacencia* adj = grafo->antenas[atual].conexoes; adj != NULL; adj = adj->proxima) {
            if (!visitados[adj->destino]) {
                EncontrarCaminhosRec(grafo, adj->destino, destino, visitados, caminho, pos, saida, caminho_encontrado);
            }
        }
    }

    visitados[atual] = false;
}

/***************************************************************
 * Função para encontrar caminhos entre antenas
 * @param grafo - Ponteiro para o grafo
 * @param origem - Índice da antena de origem
 * @param destino - Índice da antena de destino
 * @param saida - Ponteiro para o arquivo de saída
 ***************************************************************/
void EncontrarCaminhos(Grafo* grafo, int origem, int destino, FILE* saida) {
    if (origem < 0 || origem >= grafo->total_antenas ||
        destino < 0 || destino >= grafo->total_antenas) {
        fprintf(saida, "Indices invalidos!\n");
        return;
    }

    if (grafo->antenas[origem].frequencia != grafo->antenas[destino].frequencia) {
        fprintf(saida, "Frequencias diferentes. Sem caminho.\n");
        return;
    }

    bool visitados[MAX_ANTENAS] = { false };
    int caminho[MAX_ANTENAS];
    bool caminho_encontrado = false;

    EncontrarCaminhosRec(grafo, origem, destino, visitados, caminho, 0, saida, &caminho_encontrado);

    if (!caminho_encontrado) {
        fprintf(saida, "Nenhum caminho encontrado.\n");
    }
}

/***************************************************************
 * Função para mostrar interseções entre antenas de diferentes frequências
 * @param grafo - Ponteiro para o grafo
 * @param freqA - Frequência da antena A
 * @param freqB - Frequência da antena B
 * @param saida - Ponteiro para o arquivo de saída
 ***************************************************************/
void MostrarIntersecoes(Grafo* grafo, char freqA, char freqB, FILE* saida) {
    bool pares_exibidos[MAX_ANTENAS][MAX_ANTENAS] = { false };

    for (int i = 0; i < grafo->total_antenas; i++) {
        if (grafo->antenas[i].frequencia == freqA) {
            for (int j = 0; j < grafo->total_antenas; j++) {
                if (grafo->antenas[j].frequencia == freqB && !pares_exibidos[i][j]) {
                    if (grafo->antenas[i].linha == grafo->antenas[j].linha ||
                        grafo->antenas[i].coluna == grafo->antenas[j].coluna) {

                        fprintf(saida, "%c(%d,%d) - %c(%d,%d)\n",
                            freqA, grafo->antenas[i].coluna, grafo->antenas[i].linha,
                            freqB, grafo->antenas[j].coluna, grafo->antenas[j].linha);

                        pares_exibidos[i][j] = pares_exibidos[j][i] = true;
                    }
                }
            }
        }
    }
}

/***************************************************************
 * Função para calcular interferências entre antenas
 * @param grafo - Ponteiro para o grafo
 * @param saida - Ponteiro para o arquivo de saída
 ***************************************************************/
void CalcularInterferencias(Grafo* grafo, FILE* saida) {
    bool pontos_exibidos[MAX_LINHAS][MAX_COLUNAS] = { false };

    for (int i = 0; i < grafo->total_antenas; i++) {
        for (int j = i + 1; j < grafo->total_antenas; j++) {
            if (grafo->antenas[i].frequencia == grafo->antenas[j].frequencia) {
                int dx = grafo->antenas[j].coluna - grafo->antenas[i].coluna;
                int dy = grafo->antenas[j].linha - grafo->antenas[i].linha;

                if (dx == 0 || dy == 0 || abs(dx) == abs(dy)) {
                    for (int k = 1; k <= 2; k++) {
                        int x = grafo->antenas[i].coluna + (k * dx) / 3;
                        int y = grafo->antenas[i].linha + (k * dy) / 3;

                        if (x >= 0 && x < MAX_COLUNAS && y >= 0 && y < MAX_LINHAS && !pontos_exibidos[y][x]) {
                            fprintf(saida, "(%d,%d) - Entre %c(%d,%d) e %c(%d,%d)\n",
                                x, y,
                                grafo->antenas[i].frequencia, grafo->antenas[i].coluna, grafo->antenas[i].linha,
                                grafo->antenas[j].frequencia, grafo->antenas[j].coluna, grafo->antenas[j].linha);
                            pontos_exibidos[y][x] = true;
                        }
                    }
                }
            }
        }
    }
}

/***************************************************************
 * Função para exportar os resultados para um arquivo
 * @param grafo - Grafo contendo as antenas
 * @param nome_ficheiro - Nome do arquivo de saída
 ***************************************************************/
void ExportarResultados(Grafo grafo, const char* nome_ficheiro) {
    FILE* saida = fopen(nome_ficheiro, "w");
    if (!saida) {
        perror("Erro ao criar ficheiro");
        return;
    }

    // Antenas
    fprintf(saida, "=== LEITURA COLUNA-LINHA ===\n");
    fprintf(saida, "=== ANTENAS (%d) ===\n", grafo.total_antenas);
    for (int i = 0; i < grafo.total_antenas; i++) {
        fprintf(saida, "%d: %c @ (%d,%d)\n", i, grafo.antenas[i].frequencia,
            grafo.antenas[i].coluna, grafo.antenas[i].linha);
    }

    // DFS/BFS
    if (grafo.total_antenas > 0) {
        fprintf(saida, "\n=== PROCURA EM PROFUNDIDADE (Antena 0) ===\n");
        ProcuraEmProfundidade(&grafo, 0, saida);
        LimparVisitados(&grafo);

        fprintf(saida, "\n=== PROCURA EM LARGURA (Antena 0) ===\n");
        ProcuraEmLargura(&grafo, 0, saida);
        LimparVisitados(&grafo);

        // Caminhos
        if (grafo.total_antenas > 2) {
            fprintf(saida, "\n=== EXEMPLO CAMINHOS (0 -> 2) ===\n");
            EncontrarCaminhos(&grafo, 0, 2, saida);
        }

        // Intersecções únicas
        fprintf(saida, "\n=== INTERSECOES ===\n");
        char freqs[MAX_ANTENAS];
        int num_freqs = 0;

        // Coletar frequências únicas
        for (int i = 0; i < grafo.total_antenas; i++) {
            bool existe = false;
            for (int j = 0; j < num_freqs; j++) {
                if (freqs[j] == grafo.antenas[i].frequencia) {
                    existe = true;
                    break;
                }
            }
            if (!existe) freqs[num_freqs++] = grafo.antenas[i].frequencia;
        }

        // Mostrar pares únicos (A-B, A-C, etc.)
        for (int i = 0; i < num_freqs; i++) {
            for (int j = i + 1; j < num_freqs; j++) {
                MostrarIntersecoes(&grafo, freqs[i], freqs[j], saida);
            }
        }

        // Interferências
        fprintf(saida, "\n=== INTERFERENCIAS ===\n");
        CalcularInterferencias(&grafo, saida);
    }

    fclose(saida);
}

/***************************************************************
 * Função para libertar a memória alocada para o grafo
 * @param grafo - Ponteiro para o grafo
 ***************************************************************/
void LibertarGrafo(Grafo* grafo) {
    for (int i = 0; i < grafo->total_antenas; i++) {
        Adjacencia* atual = grafo->antenas[i].conexoes;
        while (atual != NULL) {
            Adjacencia* temp = atual;
            atual = atual->proxima;
            free(temp);
        }
    }
}