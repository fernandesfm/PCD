#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 
#include <unistd.h>

#define MAX_THREADS 5
#define MAX_REQUESTS 1000001

int soma=0, respond=0, request=0, id;
unsigned int num_requests = 1;

// Rotina de execucao do cliente
void Cliente(int id){
    while(num_requests <= MAX_REQUESTS){
        while(respond!=id){
            if(num_requests == MAX_REQUESTS) return;
            request=id;
        }
        // Entra na sessao critica
        int local = soma;
        usleep(rand()%2);
        soma = local + 1;
        // Sai da sessao critica
        printf("Thread %d: Soma = %d Requests: %d\n",id,soma,num_requests);
        respond=0;
        num_requests++;
    }
}

// Rotina de execucao do servidor
void Servidor(int id){
    while(num_requests <= MAX_REQUESTS){
        while(request==0){
            if(num_requests == MAX_REQUESTS) return;
        }
        respond=request;
        while(respond!=0);
        request=0;
    }
}

int main(){
    int i;

    #pragma omp parallel private(id) num_threads(MAX_THREADS)
    #pragma omp for
    for(i=0;i<MAX_THREADS;i++){
        if(i == 0) Servidor(i);
        else Cliente(i);
    }
    printf("Fim da execucao!\n");
    return 0;
}