#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main( void )
{
   MPI_Init();

   my_rank = MPI_Comm_rank();  // descobre o numero do processo atual (rank)
   proc_n  = MPI_Comm_size();  // descobre o numero total de processos (np usado na execução)

   // gero parte local do vetor (1/np avos do vetor global)

   Inicializa ( vetor );     // ordem reversa 

   pronto = FALSE;

   while ( !pronto )
   {

   // ordeno vetor local

      BubbleSort (vetor); 

   // verifico condição de parada

      // se não for np-1, mando o meu maior elemento para a direita

      // se não for 0, recebo o maior elemento da esquerda

      // comparo se o meu menor elemento é maior do que o maior elemento recebido (se sim, estou ordenado em relação ao meu vizinho)

      // compartilho o meu estado com todos os processos

      MPI_Bcast ( estado )
      /*
         tenho q dar um recieve pra todos?
         e preciso de uma "lista" para saber o estado de cada? ou so precisa saber se ta tudo ok ou nao
      */

      // se todos estiverem ordenados com seus vizinhos, a ordenação do vetor global está pronta ( pronto = TRUE, break)

      // senão continuo

   // troco valores para convergir

      // se não for o 0, mando os menores valores do meu vetor para a esquerda

      // se não for np-1, recebo os menores valores da direita

         // ordeno estes valores com a parte mais alta do meu vetor local --> 

         // devolvo os valores que recebi para a direita

      // se não for o 0, recebo de volta os maiores valores da esquerda
      

   }

   MPI_Finalize();


}