#include <stdio.h>
#include <string.h>
#include "mpi.h"
#define MAX_TASKS 32
#define VECTOR_SIZE 8

main(int argc, char** argv)
{
    int i, j;
    int my_rank;       // Identificador deste processo
    int total_process;        // Numero de processos disparados pelo usuario na linha de comando (np)  
    int message[VECTOR_SIZE];       // Buffer para as mensagens         
    int saco[MAX_TASKS][VECTOR_SIZE];      // saco de trabalho    
    MPI_Status status; // estrutura que guarda o estado de retorno          


    // inicializo o saco de trabalho
    for( i=0; i < MAX_TASKS; i++ )
    {
      for( j=0; j < VECTOR_SIZE; j++ )
        saco[i][j] = j;
    }
        
    MPI_Init(&argc , &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &total_process);  // pega informacao do numero de processos (quantidade total)

    if ( my_rank == 0 ) // qual o meu papel: sou o mestre ou um dos escravos?
    {
        double t1,t2;
        t1 = MPI_Wtime();  // inicia a contagem do tempo
        // papel do mestre

        for ( i=1 ; i < total_process ; i++)
        {
            memcpy(message, saco[i-1], VECTOR_SIZE);
            MPI_Send(message, VECTOR_SIZE, MPI_INT, i, 1, MPI_COMM_WORLD); // envio trabalho saco[i-1] para escravo com id = i
        }

        printf("Pegando um dado lhoco: %d\n", saco[2][5]);
        
        // recebo o resultado

        for ( i=1 ; i < total_process ; i++)
        {
            // recebo mensagens de qualquer emissor e com qualquer etiqueta (TAG)

            MPI_Recv(message,     /* buffer onde ser� colocada a mensagem */
                VECTOR_SIZE,                 /* uma unidade do dado a ser recebido */
                MPI_INT,           /* dado do tipo inteiro */
                MPI_ANY_SOURCE,    /* ler mensagem de qualquer emissor */
                MPI_ANY_TAG,       /* ler mensagem de qualquer etiqueta (tag) */
                MPI_COMM_WORLD,    /* comunicador padr�o */
                &status);          /* estrtura com informa��es sobre a mensagem recebida */

            // coloco o resultado no saco na poisi��o do emissor-1

        memcpy(saco[status.MPI_SOURCE-1], message, VECTOR_SIZE);   // status.MPI_SOURCE cont�m o ID do processo que enviou a mensagem que foi recebida
        }

        printf("\nKilling all processes");
        for ( i=1 ; i < total_process ; i++) //kill all processes
        {
            message[0] = -1;
            MPI_Send(message, VECTOR_SIZE, MPI_INT, i, 1, MPI_COMM_WORLD);
        }

        // mostro o saco

        printf("\nMestre[%d]: ", my_rank);               
        for ( i=0 ; i < total_process-1; i++)
        {
            printf("\n\tItem[%d]: ",i );
            for( j=0; j < VECTOR_SIZE; j++ )
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

        // recebo mensagem

        while(1)
        {
            MPI_Recv(message, VECTOR_SIZE, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

            if( message[0] == -1 ) // suicide message
            {
                printf("\nEscravo[%d]: Goodbye world!", my_rank);
                break;
            } 
            
            else
            {
                printf("\nEscravo[%d]: recebi pacote", my_rank);
                for( i = 0; i < VECTOR_SIZE; i++ )
                {
                  printf(" %d", message[i]);
                  message[i] = message[i]*message[i];

                }
                // retorno resultado para o mestre

                MPI_Send(message, VECTOR_SIZE, MPI_INT, 0, 1, MPI_COMM_WORLD);

            }
        }
    }
        
    MPI_Finalize();
}
