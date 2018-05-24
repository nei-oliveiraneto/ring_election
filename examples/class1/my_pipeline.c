#include <stdio.h>
#include "mpi.h"
 
main(int argc, char** argv)
{
    int my_rank;  /* Identificador do processo */
    int proc_n;   /* Número de processos */
    int source;   /* Identificador do proc.origem */
    int dest;     /* Identificador do proc. destino */
    int tag = 50; /* Tag para as mensagens */
    
    int message[5]; /* inteiro para as mensagens */
    int received[5];
    int fuck;
    MPI_Status status; /* Status de retorno */
    
    MPI_Init (&argc , & argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);
    
    if (my_rank == 0)
    {
    //double t1,t2;
    //t1 = MPI_Wtime();  // inicia a contagem do tempo
    message[0] = 1;
    message[1] = 2;
    message[2] = 3;
    message[3] = 4;
    message[4] = 5;

    int i;
    for( i=0; i < 5; i++ )
    {
            printf("Process %d is sending the %d item\n", my_rank, i);
            MPI_Send( &message[i], 1, MPI_INT, my_rank+1, tag, MPI_COMM_WORLD);
            MPI_Recv( &received[i],1, MPI_INT, proc_n-1, tag, MPI_COMM_WORLD, &status );
            printf("Process received the message from the last: \n");
    }

    printf( "Finished sending everything" );
    //t2 = MPI_Wtime(); // termina a contagem do tempo
    //printf("\nTempo de execucao: %f\n\n", t2-t1);
    }
    else
    {
        MPI_Recv( &fuck, 1, MPI_INT, my_rank-1, tag, MPI_COMM_WORLD, &status );
        printf("Repassing\n");
        MPI_Send( &fuck, 1, MPI_INT, (my_rank+1)%proc_n, tag, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}
