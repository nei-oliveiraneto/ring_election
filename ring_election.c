#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define TEMP_TAG 50
enum process_type
{
   ACTIVE_PROC,
   DISABLED_PROC,
   COORDENATOR_PROC
};

enum message_type
{
   DATA_MSG,
   START_ELECTION_MSG,
   ELECT_COORDENATOR_MSG,
   PROCESS_DIED_MSG
};

struct Message
{
   enum message_type msgType;
   int sender;
   int receiver;
   int data;
};


int my_id;  /* Identificador do processo */
int total_proc;   /* Número de processos */
enum process_type * allProcessStatus; //list with each process status


//get the process which my_id should send the messages
int getNextActiveProcess( )
{
   int next_proc;
   int i;
   for( i = 1; i < total_proc; i++ )
   {
      next_proc = (my_id+i)%total_proc;
      if( (allProcessStatus[next_proc] == ACTIVE_PROC) || (allProcessStatus[next_proc] == COORDENATOR_PROC)  )
         return next_proc;
   }
   return -1; //all processes but the my_id are dead
}

int getPreviousActiveProcess( )
{
   int previous_proc;
   int i;
   for( i = 1; i < total_proc; i++ )
   {
      if( my_id-i < 0 )
         previous_proc = total_proc + my_id - i;
      else
         previous_proc = my_id - i;
      if( (allProcessStatus[previous_proc] == ACTIVE_PROC) || (allProcessStatus[previous_proc] == COORDENATOR_PROC)  )
         return previous_proc;
   }
   return -1; //all processes but the my_id are dead
}

void disableCurrentCoordenator( )
{
   int i;
   for( i = 0; i < total_proc; i++ )
   {
      if( allProcessStatus[i] == COORDENATOR_PROC )
      {
         allProcessStatus[i] = DISABLED_PROC;
         return;
      }
   }
}

main(int argc, char** argv)
{
   MPI_Status status; /* Status de retorno */
   
   struct Message rcvdMessage, sentMessage; //pretty self explanatory
    
   MPI_Init (&argc , & argv);
    
   MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   MPI_Comm_size(MPI_COMM_WORLD, &total_proc);
   
   //alloc the correct value for the processes status
   allProcessStatus = malloc(sizeof(enum process_type)*total_proc);
   
   //set all status to active, but the last
   int i;
   for(i = 0; i < total_proc-1; i++)
      allProcessStatus[i] = ACTIVE_PROC;
   
   //set the last process as the COORDENATOR
   allProcessStatus[total_proc-1] = COORDENATOR_PROC;
   
   // ********* TEST AREA *********** // 
   
   if(my_id==0)
   {
      allProcessStatus[3] = DISABLED_PROC;
      sentMessage.sender = -1;
      sentMessage.receiver = 0;
      sentMessage.data  = 3;
      sentMessage.msgType = PROCESS_DIED_MSG;
      MPI_Send( &sentMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
   }
   
   // ********* TEST AREA END *********** //
   while(1)
   {
      MPI_Recv( &rcvdMessage, sizeof(struct Message), MPI_CHAR, 
               MPI_ANY_SOURCE, TEMP_TAG, MPI_COMM_WORLD, &status );
      printf("Process %d received something\n",my_id );
      switch( rcvdMessage.msgType )
      {
         case DATA_MSG: //print message if I'm the destination, redirect if I'm not
            if( rcvdMessage.receiver == my_id )
            {
               printf("Data found its destination -- Process %d received: '%d' from process %d\n", 
                        my_id, rcvdMessage.data, rcvdMessage.sender );
            }
            else
            {
               printf("Redirecting DATA message" );
               MPI_Send( &rcvdMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
            }
         break;
      
         case PROCESS_DIED_MSG: //Update the list with the dead process, repass the massage, unless it is the coordenator
            
            if( allProcessStatus[rcvdMessage.data] == COORDENATOR_PROC )
            {
               //start the start_election process
               printf("*** COORDINATOR Process %d DIED, process %d will start the Election\n", rcvdMessage.data, my_id );
               sentMessage.sender = my_id;
               sentMessage.receiver = my_id;
               sentMessage.data  = my_id;
               sentMessage.msgType = START_ELECTION_MSG;
               MPI_Send( &sentMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
            }
            else
            {
               allProcessStatus[rcvdMessage.data] = DISABLED_PROC;
               if( rcvdMessage.sender == -1 ) //you received the first PROCESS_DIED msg
               {
                  printf("Process %d DIED, process %d will start the message spreading\n", rcvdMessage.data, my_id );
                  sentMessage.sender = my_id;
                  sentMessage.receiver = my_id;
                  sentMessage.data = rcvdMessage.data;
                  sentMessage.msgType = PROCESS_DIED_MSG;
                  
                  MPI_Send( &sentMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
               }
               else if( rcvdMessage.receiver == my_id ) // cycle completed
               {
                  printf("All the processes were informed of process %d DEAD\n", rcvdMessage.data );
               }
               else //repass the message
               {
                  printf("Process %d: Repassing the DEAD of process %d to process %d\n", my_id, rcvdMessage.data, getNextActiveProcess() );
                  MPI_Send( &rcvdMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
               }
            }
         break;
         
         
         case START_ELECTION_MSG:
            
            disableCurrentCoordenator( );
            if( rcvdMessage.receiver == my_id ) //cycle completed
            {
               printf("START_ELECTION_MSG completed the cycle, the next coordenator process is %d proc, spreading the info\n", rcvdMessage.data);
               sentMessage.sender = my_id;
               sentMessage.receiver = my_id;
               sentMessage.data = rcvdMessage.data;
               sentMessage.msgType = ELECT_COORDENATOR_MSG;
               MPI_Send( &sentMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
            }
            else
            {
               if( my_id > rcvdMessage.data )
                  rcvdMessage.data = my_id;
               
               MPI_Send( &rcvdMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
            }
         
         break;
         
         
         case ELECT_COORDENATOR_MSG:
         
            allProcessStatus[rcvdMessage.data] = COORDENATOR_PROC;
            if( rcvdMessage.receiver == my_id ) //cycle completed
            {
               printf("ELECT_COORDENATOR_MSG completed the cycle, everyone know the new coordenator");
            }
            else
            {
               MPI_Send( &rcvdMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
         
            }
         break;
      }
}
   
   MPI_Finalize();
   
}
