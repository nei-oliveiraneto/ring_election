#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

int main(int argc, char** argv)
{
   MPI_Status status; /* Status de retorno */
   
   int my_rank, total_proc;
   int * localVector = NULL;

   MPI_Init (&argc , & argv);
    
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &total_proc);

   int data = 0;
   if( my_rank == 0 )data = 27; 

   printf("%d - Data before broadcast: %d\n", my_rank, data);

   MPI_Bcast(&data, 1, MPI_INT, 0, MPI_COMM_WORLD);

   printf("%d - Data after broadcast: %d\n", my_rank, data);


   MPI_Finalize();


}