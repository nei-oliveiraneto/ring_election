#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define TOTAL_ARRAY_SIZE 1000
#define EXGANGE_DELTA 20 //in percentage

void printVector( int* vector, int size, int my_rank )
{
   printf( "Process %d: ", my_rank );
   for( int i = 0; i < size; i++ )
      printf( "%d, ", vector[i]);
   printf("\n");
}

int * InitVector( int np, int rank )
{
   int localPartSize = TOTAL_ARRAY_SIZE/np;
   int exchangeDelta = localPartSize*EXGANGE_DELTA/100;
   int localVectorSize = localPartSize+exchangeDelta; 

   int startingPoint = TOTAL_ARRAY_SIZE - localPartSize*rank -1;

   if( rank != np-1 )
   {
      int * vector = (int *)malloc(sizeof(int) * localVectorSize );
      for( int i = 0; i < localPartSize; i ++ )
      {
         vector[i] = startingPoint-i;
      }
      return vector;
   }
   else
   {
      
      int lastVectorSize = localPartSize + TOTAL_ARRAY_SIZE%np; 
      int * vector = (int *)malloc(sizeof(int) * lastVectorSize );
      for( int i = 0; i < lastVectorSize; i ++ )
      {
         vector[i] = lastVectorSize-i-1;
      }
      return vector;
   }

}

void BubbleSort( int * vetor, int n )
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

int main(int argc, char** argv)
{
   MPI_Status status; /* Status de retorno */
   
   int my_rank, total_proc;
   int * localVector = NULL;

   MPI_Init (&argc , & argv);
    
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &total_proc);

   int localPartSize = TOTAL_ARRAY_SIZE/total_proc;
   int exchangeDelta = localPartSize*EXGANGE_DELTA/100;
   int localVectorSize = localPartSize+exchangeDelta;

   // Gero parte local do vetor
   localVector = InitVector( total_proc, my_rank );

   int isReady = 0;
   int isOrdenedNeighbour, leftBiggerElement;
   if( my_rank == total_proc -1 )
   {
      localPartSize = localPartSize + TOTAL_ARRAY_SIZE%total_proc; 
   }

   while( !isReady )
   {

      // ordeno vetor local
      BubbleSort( localVector, localPartSize ); 

      printVector(localVector, localPartSize, my_rank);
      

      // verifico condição de parada

      // se não for np-1, mando o meu maior elemento para a direita
      if( my_rank != total_proc-1 )
      {
         MPI_Send( &localVector[localPartSize-1], 1, MPI_INT, my_rank+1, 1, MPI_COMM_WORLD ); 
      }

      // se não for 0, recebo o maior elemento da esquerda
      if( my_rank != 0 )
      {
         MPI_Recv( &leftBiggerElement, 1, MPI_INT, 
               my_rank-1, 1, MPI_COMM_WORLD, &status );
        
         // comparo se o meu menor elemento é maior do que o maior elemento recebido (se sim, estou ordenado em relação ao meu vizinho)
         if( localVector[0] > leftBiggerElement )
            isOrdenedNeighbour = 1;
         else
            isOrdenedNeighbour = 0;
         
      }
      else
         isOrdenedNeighbour = 1;

      //printf( "Processo %d esta %d com o vizinho\n", my_rank, isOrdenedNeighbour );

      // compartilho o meu estado com todos os processos
      int receivedState;
      isReady = isOrdenedNeighbour;
      for( int i = 0; i < total_proc; i++ )
      {
         if( my_rank == i )
         {
            //printf( "%d is sending %d to all\n", my_rank, isOrdenedNeighbour );
            MPI_Bcast( &isOrdenedNeighbour, 1, MPI_INT, my_rank, MPI_COMM_WORLD ); 
         }
         else
         {
            //printf( "%d is receiving from %d\n", my_rank, i );
            MPI_Bcast( &receivedState, 1, MPI_INT, i, MPI_COMM_WORLD );
            isReady = isReady && receivedState;
         }
      }

      // se todos estiverem ordenados com seus vizinhos, a ordenação do vetor global está pronta ( pronto = TRUE, break)
      // senão continuo
      printf( "%d: Estou pronto: %d\n", my_rank, isReady );
      if( isReady )
         break;

      // troco valores para convergir

      // se não for o 0, mando os menores valores do meu vetor para a esquerda
      if( my_rank != 0 )
      {
         MPI_Send( &localVector[0], exchangeDelta, MPI_INT, my_rank-1, 1, MPI_COMM_WORLD ); 
      }

      // se não for np-1, recebo os menores valores da direita
      if( my_rank != total_proc-1 )
      {
         MPI_Recv( &localVector[localPartSize], exchangeDelta, MPI_INT, 
               my_rank+1, 1, MPI_COMM_WORLD, &status );
      
         // ordeno estes valores com a parte mais alta do meu vetor local 
         BubbleSort( &localVector[localPartSize-exchangeDelta], exchangeDelta*2 );

         // devolvo os valores que recebi para a direita
         MPI_Send( &localVector[localPartSize], exchangeDelta, MPI_INT, my_rank+1, 1, MPI_COMM_WORLD ); 

      }

      // se não for o 0, recebo de volta os maiores valores da esquerda
      if( my_rank != 0 )
      {
          MPI_Recv( &localVector[0], exchangeDelta, MPI_INT, 
               my_rank-1, 1, MPI_COMM_WORLD, &status );
      }
      

   }

   MPI_Finalize();


}