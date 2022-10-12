/* Autor: Felipe Fernandes Mendonça, RA: 111788 */

#define HAVE_STRUCT_TIMESPEC

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int **grid;
int **newGrid;

struct thread_data
{
    int n;
    int generations;
};
struct thread_data thread_data_struct;

//inicia o jogo, realiza X iteracoes
void run_game(void *thread_arg)
{
    struct thread_data *my_data;
    my_data = (struct thread_data *) thread_arg;
    int n = my_data->n;
    int generations = my_data->generations;

    pthread_mutex_lock(&mutex);

    int g, i, x, y;
    int alive=10;
    int neighbors;

    for(g = 0; g < generations; g++)
    {
        //percorre cada posicao do tabuleiro verificando os vizinhos vivos
        //grid[i][j]
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
                if(neighbors == 3)
                {
                    alive++;
                    newGrid[y][x] = 1;
                }
            }
        }
        // fim da geração, nova geração se torna a atual para próxima iteração
        for(i = 0; i < (n * n); i ++)
        {
            y = (i / n);
            x = (i % n);
            grid[y][x] = newGrid[y][x];
        }
        // geração atual atualizada, exibe quantas celulas vivas
        printf("\n---------GEN %d : %d celulas vivas---------\n", g+1, alive);

        pthread_mutex_unlock(&mutex);
    }
}

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
    struct timeval inicio, fim, inicio_SO, fim_SO;
    gettimeofday(&inicio_SO, NULL);

    sem_init(&mutex, 0, 1);
    pthread_t t[8];
    int tmili, tmili_SO;
    int n;
    int i, x, y, g;
    int generations;
    int neighbors;
    int threads;

    printf("Insira a dimensao do tabuleiro: ");
    scanf("%d", &n);

    printf("Quantas threads para execucao?\n");
    scanf("%d", &threads);

    printf("Insira a quantidade de geracoes (iteracoes): ");
    scanf("%d", &generations);

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
    lin =10;
    col = 30;
    grid[lin][col+1] = 1;
    grid[lin][col+2] = 1;
    grid[lin+1][col] = 1;
    grid[lin+1][col+1] = 1;
    grid[lin+2][col+1] = 1;

    for(i = 0; i < threads; i++)
    {
        thread_data_struct.n = n;
        thread_data_struct.generations = generations;
    }

    gettimeofday(&inicio, NULL);
    for(i = 0; i < threads; i++)
    {
        pthread_create(&t[i], NULL, &run_game, (void *) &thread_data_struct);
    }

    for(i = 0; i < threads; i++)
    {
        pthread_join(t[i],NULL);
    }
    gettimeofday(&fim, NULL);

    sem_destroy(&mutex);

    tmili = (int) (1000 * (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1000);

    gettimeofday(&fim_SO, NULL);

    tmili_SO = (int) (1000 * (fim_SO.tv_sec - inicio_SO.tv_sec) + (fim_SO.tv_usec - inicio_SO.tv_usec) / 1000);

    printf("\nTempo decorrido no SO: %dms\n", tmili_SO);

    printf("\nTempo decorrido no laco de geracao das sucessoes: %dms\n", tmili);

    return 0;
}
