#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define TOTAL_ARRAY_SIZE 100000


void InitVector( int * vector, int size )
{
   
   //vector = malloc(sizeof(int)*size);

   for( int i = 0; i < size; i++ )
   {
      vector[i] = size-i-1;
   }

}

void findFatherSonPID( int my_id, int total_proc, int* fid, int* s1id, int* s2id )
{
   if( my_id == 0 ) //raiz
      *fid = -1;
   else
      *fid = (my_id-1)/2;

   if( my_id >= total_proc/2 ) //folha
   { 
      *s1id = -1;
      *s2id = -1;
   }
   else
   {
      *s1id = 2*my_id+1;
      *s2id = *s1id+1;
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

int * Interleaving( int * vetor1, int * vetor2, int vetor1size, int vetor2size )
{
	int *vetor_auxiliar;
	int i1, i2, i_aux;
   int total_tam = vetor1size+vetor2size;

	vetor_auxiliar = (int *)malloc(sizeof(int) * total_tam );

	i1 = 0;
	i2 = 0;

	for (i_aux = 0; i_aux < total_tam; i_aux++) {
		if (((vetor1[i1] <= vetor2[i2]) && (i1 < vetor1size))
		    || (i2 == vetor2size))
			vetor_auxiliar[i_aux] = vetor1[i1++];
		else
			vetor_auxiliar[i_aux] = vetor2[i2++];
	}

   return vetor_auxiliar;
}

int isVectorOrdered( int * vector )
{
   
   for( int i = 0; i < TOTAL_ARRAY_SIZE; i++ )
   {
      if( vector[i] != i ) return 0;
   }

   return 1;
   
}

int main(int argc, char** argv)
{
   MPI_Status status; /* Status de retorno */
   
   int my_rank, total_proc;
   int localVectorSize, fatherPID, son1PID, son2PID;
   int * localVector = NULL;

   double t1,t2;
   // int auxVector[TOTAL_ARRAY_SIZE];
   // int localVector[TOTAL_ARRAY_SIZE];
   //int * interleavedVector;
    
   MPI_Init (&argc , & argv);
    
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &total_proc);

   // recebo vetor

   findFatherSonPID( my_rank, total_proc, &fatherPID, &son1PID, &son2PID );

   if ( my_rank != 0 )
   {
      MPI_Probe(fatherPID, 1, MPI_COMM_WORLD, &status);
      MPI_Get_count(&status, MPI_INT, &localVectorSize);
      
      localVector = (int *)malloc(sizeof(int) * localVectorSize );
      
      MPI_Recv( localVector, localVectorSize, MPI_INT, 
               fatherPID, 1, MPI_COMM_WORLD, &status );

   }
   else
   {
      t1 = MPI_Wtime();
      
      localVector = (int *)malloc(sizeof(int) * TOTAL_ARRAY_SIZE );
      InitVector( localVector, TOTAL_ARRAY_SIZE );      // sou a raiz e portanto gero o vetor - ordem reversa
      localVectorSize = TOTAL_ARRAY_SIZE;
   }

   // dividir ou conquistar?

   if ( son1PID == -1 )
   {
      BubbleSort( localVector, localVectorSize );  // conquisto
   }
   else
   {
      // dividir
      // quebrar em duas partes e mandar para os filhos

      int son1ArraySize, son2ArraySize;
      
      if( localVectorSize % 2 == 0 )
      {
         son1ArraySize = localVectorSize/2;
         son2ArraySize = localVectorSize/2;
      }
      else
      {
         son1ArraySize = localVectorSize/2+1;
         son2ArraySize = localVectorSize/2;
      }
      MPI_Send( &localVector[0], son1ArraySize, MPI_INT, son1PID, 1, MPI_COMM_WORLD );  // mando metade inicial do vetor
      MPI_Send( &localVector[son1ArraySize], son2ArraySize, MPI_INT, son2PID, 1, MPI_COMM_WORLD );  // mando metade inicial do vetor

      // receber dos filhos


      MPI_Recv( &localVector[0], son1ArraySize, MPI_INT, 
               son1PID, 1, MPI_COMM_WORLD, &status );
      MPI_Recv( &localVector[son1ArraySize], son2ArraySize, MPI_INT, 
               son2PID, 1, MPI_COMM_WORLD, &status );


      // intercalo vetor inteiro 
 
      //memcpy( localVector, Interleaving( localVector, localVectorSize ), localVectorSize);
   
      localVector = Interleaving( &localVector[0], &localVector[son1ArraySize], son1ArraySize, son2ArraySize );
     
   }

   // mando para o pai

   if ( my_rank !=0 )
      MPI_Send ( localVector, localVectorSize, MPI_INT, fatherPID, 1, MPI_COMM_WORLD );  // mando meu vetor para o pai
      
   else //raiz, mostra o vetor
   {
      t2 = MPI_Wtime(); // termina a contagem do tempo
      if( isVectorOrdered( localVector ) )
      {
         printf( "Ordenacao correta!\n" );
         printf("\nTempo de execucao: %f\n\n", t2-t1);
      }
      else
      {
         printf( "Array original nao esta ordenado corretamente!\n" );
      }
         
                         
   }

   MPI_Finalize();
}