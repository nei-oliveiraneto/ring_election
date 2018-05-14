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
MPI_Status status; /* Status de retorno */
 
MPI_Init (&argc , & argv);
 
MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
MPI_Comm_size(MPI_COMM_WORLD, &proc_n);
 
if (my_rank%2 == 0)
{
   double t1,t2;
   t1 = MPI_Wtime();  // inicia a contagem do tempo
   message[0] = my_rank;
   message[1] = my_rank+1;
   message[2] = my_rank+2;
   message[3] = my_rank+3;
   message[4] = my_rank+4;
   printf("Process %d is sending the message\n", my_rank);
   MPI_Send( message, sizeof(message)+1, MPI_INT, my_rank+1, tag, MPI_COMM_WORLD);
   MPI_Recv( message, sizeof(message)+1, MPI_INT, my_rank+1, tag, MPI_COMM_WORLD, &status );
   printf("Process %d received the message from  next odd number: \n", my_rank);
   int i;
   for( i = 0; i<5;i++)
   {
      printf("%d\t\n", message[i] );
   }
   t2 = MPI_Wtime(); // termina a contagem do tempo
   printf("\nTempo de execucao: %f\n\n", t2-t1);
}
else
{
   MPI_Recv( message, sizeof(message)+1, MPI_INT, my_rank-1, tag, MPI_COMM_WORLD, &status );
   printf("Process %d received msg from process %d, incrementing it\n", my_rank, my_rank-1);
   int i;
   for( i = 0; i<5;i++)
   {
      message[i]++;
   }
   MPI_Send( message, sizeof(message)+1, MPI_INT, my_rank-1, tag, MPI_COMM_WORLD);
}
MPI_Finalize();
}
