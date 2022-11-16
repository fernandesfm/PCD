#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 
#include <unistd.h>

#define MAX_THREADS 4
#define MAX_REQUESTS 20000

int soma=0, respond=0, request=0, id;
unsigned int num_requests = 1;

// Rotina de execucao do cliente
void Cliente(int id){
    unsigned int num_requests = 1;
    while(num_requests <= MAX_REQUESTS){
        // Entra na sessao critica
        int local = soma;
        usleep(rand()%2);
        soma = local + 1;
        // Sai da sessao critica
        printf("Thread %d: Soma = %d Requests: %d\n",id,soma,num_requests);
        num_requests++;
    }
}

int main(){
    int i;
    #pragma omp parallel private(id) num_threads(MAX_THREADS)
    #pragma omp for
        for(i=0;i<MAX_THREADS;i++) Cliente(i);
    printf("Fim da execucao!\n");
    return 0;
}
