#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"
#define NUMBER_OF_ARRAYS 20
#define ARRAY_SIZE 10
//#define BUBBLE_SORT 0 // comentar para usar quicksort

void bs(int n, int * vetor)
{
    int c=0, d, troca, trocou =1;

    while (c < (n-1) & trocou )
    {
        trocou = 0;
        for (d = 0 ; d < n - c - 1; d++)
            if (vetor[d] > vetor[d+1])
            {
                troca      = vetor[d];
                vetor[d]   = vetor[d+1];
                vetor[d+1] = troca;
                trocou = 1;
            }
        c++;
    }
}

int cmpfunc (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}

main(int argc, char** argv)
{
    int i, j;
    int my_rank;       // Identificador deste processo
    int total_process;        // Numero de processos disparados pelo usuario na linha de comando (np)  
    int message[ARRAY_SIZE];       // Buffer para as mensagens         
    int saco[NUMBER_OF_ARRAYS][ARRAY_SIZE];      // saco de trabalho    
    MPI_Status status; // estrutura que guarda o estado de retorno          


    // inicializo o saco de trabalho
    for( i=0; i < NUMBER_OF_ARRAYS; i++ )
      for( j=0; j < ARRAY_SIZE; j++ )
        saco[i][j] = ARRAY_SIZE-j-1;
        
    MPI_Init(&argc , &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &total_process);  // pega informacao do numero de processos (quantidade total)

    if( my_rank == 0 ) // qual o meu papel: sou o mestre ou um dos escravos?
    {
        double t1,t2;
        t1 = MPI_Wtime();  // inicia a contagem do tempo
        // papel do mestre

        
        //wait for slave to call
        int total_sent = 0;
        int total_received = 0;
        while(total_received<NUMBER_OF_ARRAYS)
        {
            //recebe mensagem do escravo, pode ser dizendo que esta livre, pode ser com array ordenado
            MPI_Recv(message, ARRAY_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            if( message[0] == -1 ) //mensagem de escravo livre
            {
                if( total_sent < NUMBER_OF_ARRAYS ) //se ainda nao enviou todos os arrays do saco
                {
                    memcpy(message, saco[total_sent++], sizeof(int)*ARRAY_SIZE);
                    MPI_Send(message, ARRAY_SIZE, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
                }
            }
            else
            {
                memcpy( saco[total_received], message, sizeof(int)*ARRAY_SIZE );
                total_received++;
            }
        }
        
        printf("\nKilling all processes");
        for( i=1; i < total_process; i++ ) //kill all processes
        {
            message[0] = -1;
            MPI_Send(message, ARRAY_SIZE, MPI_INT, i, 1, MPI_COMM_WORLD);
        }

        // mostro o saco
        printf("\nMestre[%d]: ", my_rank);               
        for( i=0; i < NUMBER_OF_ARRAYS; i++ )
        {
            printf("\n\tItem[%d]: ",i );
            for( j=0; j < ARRAY_SIZE; j++ )
            {
                printf("%d  ", saco[i][j]);
            }
        }
        printf("\n\n");
        
        t2 = MPI_Wtime(); // termina a contagem do tempo
        printf("\nTempo de execucao: %f\n\n", t2-t1);
    }   

    else                 
    {
        // papel do escravo
        // fica em loop infinito ateh receber mensagem de suicidio
        while(1)
        {
            message[0] = -1; //primeira mensagem que envia eh avisando que esta livre
            MPI_Send(message, ARRAY_SIZE, MPI_INT, 0, 1, MPI_COMM_WORLD);
            
            MPI_Recv(message, ARRAY_SIZE, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

            if( message[0] == -1 ) // mensagem de suicidio
            {
                printf("\nEscravo[%d]: Goodbye world!", my_rank);
                break;
            } 
            
            else
            {
                printf("\nEscravo[%d]: recebi pacote", my_rank);
                for( i = 0; i < ARRAY_SIZE; i++ )
                {
                  printf(" %d", message[i]);
                }
                
                #ifdef BUBBLE_SORT 
                    bs(ARRAY_SIZE, message);
                #endif

                #ifndef BUBBLE_SORT
                    qsort(message, ARRAY_SIZE, sizeof(int), cmpfunc);
                #endif
               
                // retorno resultado para o mestre
                MPI_Send(message, ARRAY_SIZE, MPI_INT, 0, 1, MPI_COMM_WORLD);

            }
        }
    }
        
    MPI_Finalize();
}
