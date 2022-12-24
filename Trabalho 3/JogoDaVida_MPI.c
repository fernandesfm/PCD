#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include<sys/time.h>

#define NUM_GEN 2000 // Numero de geracoes
#define TAM 2048 // Tamanho N da matriz NxN
#define SRAND_VALUE 1985 // Seed para srand()
#define vivo 1
#define morto 0
#define PRIN_PROC 0

int **grid, **newgrid; // matrizes de geracao
int bufSnd[TAM], bufRcv[TAM]; // Buffers de comunicacao

typedef struct{
    int secs;
    int usecs;
}TIME_DIFF;

// Calcula o tempo decorrido entre um intervalo de tempo
TIME_DIFF * my_difftime (struct timeval *start, struct timeval *end){
    TIME_DIFF *diff = (TIME_DIFF*)malloc(sizeof(TIME_DIFF));
 
    if(start->tv_sec == end->tv_sec){
        diff->secs = 0;
        diff->usecs = end->tv_usec - start->tv_usec;
    }
    else{
        diff->usecs = 1000000 - start->tv_usec;
        diff->secs = end->tv_sec - (start->tv_sec + 1);
        diff->usecs += end->tv_usec;
        if(diff->usecs >= 1000000){
            diff->usecs -= 1000000;
            diff->secs += 1;
        }
    }
    return diff;
}

// Retorna a quantidade de vizinhos vivos de cada celula na posicao ​i,j
int getNeighbors(int i, int j) {
    int count=0;

    count += grid[i][((j+1)%TAM)]; // direita
    count += grid[((i+1)%TAM)][((j+1)%TAM)]; // direita baixo
    count += grid[((i+1)%TAM)][j]; //baixo
    count += grid[((i+1)%TAM)][(TAM+(j-1))%TAM]; // esquerda baixo
    
    count += grid[i][(TAM+(j-1))%TAM]; // esquerda
    count += grid[(TAM+(i-1))%TAM][(TAM+(j-1))%TAM]; //esquerda cima
    count += grid[(TAM+(i-1))%TAM][j]; // cima
    count += grid[(TAM+(i-1))%TAM][((j+1)%TAM)]; // direita cima

    return count;
}

// Cria uma nova geracao de acordo com as regras estabelecidas
void novaGeracao(int numProc, int part, int rank){
    int i, j;
    int ini = part*rank;
    
    for(i=ini;i<(ini+part); i++){     
        for(j = 0; j<TAM; j++){
            if (grid[i][j]){ // Se estiver vivo
                if (getNeighbors(i,j) < 2 || getNeighbors(i,j) > 3) // Regra A e C
                    newgrid[i][j] = morto;
                else // Regra B
                    newgrid[i][j] = vivo; 
            }
            else{ // Se estiver morto
                if(getNeighbors(i,j) == 3) // Regra D
                    newgrid[i][j] = vivo;
                else
                    newgrid[i][j] = morto;
            }
        }
    }

    for(i=0;i<TAM; i++){     
        for(j = 0; j<TAM; j++){
            grid[i][j] = newgrid[i][j];
        }
    }
}

// Conta quantas celulas estao vivas na geracao
int contaPopulacao(){
    int i,j,cont = 0;

    for(i=0;i<TAM; i++){     
        for(j = 0; j<TAM; j++){
            if (grid[i][j])
                cont++;
        }
    }

    return cont;
}

// Rotina do processo principal
void prinProc(int numProc){
    int i,j,origem,tag=0,div=0,rank,gen;
    int part = TAM/numProc;
    TIME_DIFF *time;
    struct timeval start, end;

    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    printf("Sistema principal iniciado! [%d]\n",rank);

    gettimeofday (&start, NULL);
    // Alocacao das matrizes
    grid = malloc(sizeof(int*)*TAM);
    newgrid = malloc(sizeof(int*)*TAM);
    for(i=0;i<TAM;i++){
        grid[i] = malloc(sizeof(int)*TAM);
        newgrid[i] = malloc(sizeof(int)*TAM);
    }

    // Gera a primeira metade da primeira geração pseudoaleatoriamente
    srand(SRAND_VALUE);
    for(i=0;i<(part); i++){     
        for(j = 0; j<TAM; j++){
            grid[i][j] = rand() % 2;  
        }
    }
    div = i;
    for(origem=1;origem<numProc;origem++){
        for(i=div;i<(origem+1)*part;i++){
            MPI_Recv(bufRcv,TAM,MPI_INT,origem,tag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            for(j=0;j<TAM;j++){
                grid[i][j] = bufRcv[j];
            }
        }
        div=i;
    }

    printf("Condicao Inicial: %d Celulas Vivas\n", contaPopulacao());

    // Gera NUM_GEN geracoes a partir da primeira
    for(gen=0;gen<NUM_GEN;gen++){
        for(i=0;i<TAM;i++){ // Envia tabela
            for(j=0;j<TAM;j++) bufSnd[j] = grid[i][j];
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Bcast(bufSnd,TAM,MPI_INT,PRIN_PROC,MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);
        }

        novaGeracao(numProc,part,rank);

        for(origem=1;origem<numProc;origem++){
            for(i=(part*origem);i<(part*(origem+1));i++){
                MPI_Recv(bufRcv,TAM,MPI_INT,origem,tag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                for(j=0;j<TAM;j++){
                    grid[i][j] = bufRcv[j];
                }
            }
        }
    }

    printf("Ultima Geracao: %d Celulas Vivas\n", contaPopulacao());

    gettimeofday (&end, NULL);
    time = my_difftime(&start, &end);
    printf("Tempo: %d,%ds\n",time->secs,time->usecs);
}

// Rotina do processo secundario
void secProc(int numProc){
    int tag=0,dest=0,i,j,gen,rank,part=TAM/numProc;

    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    printf("Sistema secundario iniciado! [%d]\n",rank);



    // Alocacao das matrizes
    grid = malloc(sizeof(int*)*TAM);
    newgrid = malloc(sizeof(int*)*TAM);
    for(i=0;i<TAM;i++){
        grid[i] = malloc(sizeof(int)*TAM);
        newgrid[i] = malloc(sizeof(int)*TAM);
    }

    // Gera a segunda metade da primeira geracao pseudoaleatoriamente
    srand(SRAND_VALUE);
    for(i=0;i<TAM; i++){     
        for(j = 0; j<TAM; j++){
            if(i >= rank*part && i < (rank+1)*part){
                grid[i][j] = rand() % 2;  
            }else{
                rand();
            }
        }
    }

    for(i=rank*part;i<(rank+1)*part;i++){
        for(j=0;j<TAM;j++) bufSnd[j] = grid[i][j];
        MPI_Send(bufSnd,TAM,MPI_INT,dest,tag,MPI_COMM_WORLD);
    }

    for(gen=0;gen<NUM_GEN;gen++){
        for(i=0;i<TAM;i++){ // recebe a tabela
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Bcast(bufRcv,TAM,MPI_INT,PRIN_PROC,MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);
            for(j=0;j<TAM;j++) grid[i][j] = bufRcv[j];
        }

        novaGeracao(numProc,part,rank);

        for(i=(part*rank);i<part*(rank+1);i++){ // envia tabela
            for(j=0;j<TAM;j++) bufSnd[j] = grid[i][j];
            MPI_Send(bufSnd,TAM,MPI_INT,dest,tag,MPI_COMM_WORLD);
        }
    }
}

int main(int argc, char *argv[]){
    int rank; // ID do processo
    int numProc; // numero de processos

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    if(rank == 0){ // caso seja o processo principal
        prinProc(numProc);
    }else{
        secProc(numProc);
    }

    MPI_Finalize();
    return 0;
}