#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>

int getNeighbors(int **grid, int i, int j, int n)
{
    //contador que guardará a quantidade de células vizinhas
    int cont = 0;

    //variáveis de apoio, caso a célula esteja na borda do tabuleiro
    int x1, y1, x2, y2;

//verificações das extremidades do tabuleiro
    //1. se a célula está na borda direita do tabuleiro
    //2. se não 1, verifica se está na borda esquerda
    //3. posição da coluna ok
    if(j == (n-1))
    {
        x1 = j - 1;
        x2 = 0;
    }
    else if(j == 0)
    {
        x1 = n - 1;
        x2 = j + 1;
    }
    else
    {
        x1 = j - 1;
        x2 = j + 1;
    }

    //4. se a célula está na borda superior
    //5. se não 4, verifica se está na borda inferior
    //6. posição da linha ok
    if(i == 0)
    {
        y1 = n - 1;
        y2 = i + 1;
    }
    else if(i == (n-1))
    {
        y1 = i - 1;
        y2 = 0;
    }
    else
    {
        y1 = i - 1;
        y2 = i + 1;
    }
// fim da verificação dos vizinhos da célula

//conta quantos vizinhos estão vivos
    if(grid[y1][x1] == 1) cont++; // superior esquerdo
    if(grid[y1][x2] == 1) cont++; // superior direito
    if(grid[y1][j]  == 1) cont++; // superior central
    if(grid[y2][x1] == 1) cont++; // inferior esquerdo
    if(grid[y2][x2] == 1) cont++; // inferior direito
    if(grid[y2][j]  == 1) cont++; // inferior central
    if(grid[i][x1]  == 1) cont++; // central esquerdo
    if(grid[i][x2]  == 1) cont++; // central direito

// retorna a quantidade de vizinhos vivos
    return cont;
}

int main()
{
    struct timeval inicio, fim;
    gettimeofday(&inicio, NULL);

    int n;
    int i, x, y, g;
    int **grid;
    int **newGrid;
    int generations;
    int neighbors;
    int alive = 0;
    int threads;
    int tmili;

    printf("Insira a dimensao do tabuleiro: ");
    scanf("%d", &n);

    printf("Quantas threads para execucao?\n");
    scanf("%d", &threads);
    omp_set_num_threads(threads);

    //Alocação dinâmica para as matrizes dos tabuleiros
    grid = (int**) malloc(sizeof(int*) * n);
    newGrid = (int**) malloc(sizeof(int*) * n);

    for(i = 0; i < n; i++)
    {
        grid[i] = (int*) malloc(sizeof(int) * n);
        newGrid[i] = (int*) malloc(sizeof(int) * n);
    }

    //GLIDER
    int lin = 1, col = 1;
    grid[lin][col+1] = 1;
    grid[lin+1][col+2] = 1;
    grid[lin+2][col] = 1;
    grid[lin+2][col+1] = 1;
    grid[lin+2][col+2] = 1;

    //R-pentomino
    lin =10; col = 30;
    grid[lin][col+1] = 1;
    grid[lin][col+2] = 1;
    grid[lin+1][col] = 1;
    grid[lin+1][col+1] = 1;
    grid[lin+2][col+1] = 1;

    alive = 10;

    printf("Insira a quantidade de geracoes (iteracoes): ");
    scanf("%d", &generations);

//inicia o jogo, realiza X iteracoes
    double start;
    double end;

    start = omp_get_wtime();
    for(g = 0; g < generations; g++)
    {
        //percorre cada posicao do tabuleiro verificando os vizinhos vivos
        //grid[i][j]
        #pragma omp parallel for private(i, x, y, neighbors) reduction(+:alive) shared(grid, newGrid)
        for(i = 0; i < (n * n); i++)
        {
            y = (i / n);
            x = (i % n);
            //obtem os vizinhos vivos da celula grid[i][j]
            neighbors = getNeighbors(grid, y, x, n);

            //1. se a celula ja esta viva
            //2. se nao 1, celula esta morta
            if(grid[y][x] == 1)
            {
                //1. tem menos que dois vizinhos = MORRE
                //2. tem mais que 3 vizinhos = MORRE
                //3. 2 ou 3 vizinhos = VIVE
                if(neighbors < 2)
                {
                    alive = alive - 1;
                    newGrid[y][x] = 0;
                }
                else if(neighbors > 3)
                {
                    alive = alive - 1;
                    newGrid[y][x] = 0;
                }
                else
                {
                    newGrid[y][x] = 1;
                }
            }
            else
            {
                //1. se tem 3 vizinhos = REVIVE
                //2. se tem 6 vizinhos = REVIVE (HighLife mode)
                if(neighbors == 3)
                {
                    alive++;
                    newGrid[y][x] = 1;
                }
                if(neighbors == 6)
                {
                    alive++;
                    newGrid[y][x] = 1;
                }
            }
        }
        // fim da geração, nova geração se torna a atual para próxima iteração
        #pragma omp parallel for private(i, x, y)
        for(i = 0; i < (n * n); i ++)
        {
            y = (i / n);
            x = (i % n);
            grid[y][x] = newGrid[y][x];
        }
        // geração atual atualizada, exibe quantas celulas vivas
        printf("\n---------GEN %d : %d celulas vivas---------\n", g+1, alive);
    }
    end = omp_get_wtime();

    printf("\nTempo decorrido no laco de geracao das sucessoes: %fms\n", (end - start));

    gettimeofday(&fim, NULL);

    tmili = (int) (1000 * (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000);

    printf("\nTempo decorrido no SO: %dms\n", tmili);

    return 0;
}
