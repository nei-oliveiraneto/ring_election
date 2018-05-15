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
   
   if(my_id==3)
   {
      sentMessage.sender = -1;
      sentMessage.receiver = 4;
      sentMessage.data  = 5;
      sentMessage.msgType = PROCESS_DIED_MSG;
      MPI_Send( &sentMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
   }
   
   // ********* TEST AREA END *********** //
   while(1)
   {
      MPI_Recv( &rcvdMessage, sizeof(struct Message), MPI_CHAR, 
               MPI_ANY_SOURCE, TEMP_TAG, MPI_COMM_WORLD, &status );
               
      switch( rcvdMessage.msgType )
      {
         case DATA_MSG: //print message if I'm the destination, redirect if I'm not
            if( rcvdMessage.receiver == my_id )
            {
               printf("Process %d says: Data found its destination, I've received: '%d' from process %d\n", 
                        my_id, rcvdMessage.data, rcvdMessage.sender );
            }
            else
            {
               printf("Process %d says: Redirecting DATA message", my_id );
               MPI_Send( &rcvdMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
            }
         break;
      
         case PROCESS_DIED_MSG: //Update the list with the dead process, repass the massage, unless it is the coordenator
            
            if( allProcessStatus[rcvdMessage.data] == COORDENATOR_PROC )
            {
               //start the start_election process
               printf("Process %d says: Coordinator Process %d DIED, I will start the ELECTION_CANDIDATURE\n", my_id, rcvdMessage.data );
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
                  printf("Process %d says: Process %d DIED, I will start the message spreading\n", my_id, rcvdMessage.data );
                  sentMessage.sender = my_id;
                  sentMessage.receiver = my_id;
                  sentMessage.data = rcvdMessage.data;
                  sentMessage.msgType = PROCESS_DIED_MSG;
                  
                  MPI_Send( &sentMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
               }
               else if( rcvdMessage.receiver == my_id ) // cycle completed
               {
                  printf("Proces %d says: All the processes were informed of process %d DEAD\n", my_id, rcvdMessage.data );
               }
               else //repass the message
               {
                  printf("Process %d says: Repassing the DEATH of process %d to process %d\n", my_id, rcvdMessage.data, getNextActiveProcess() );
                  MPI_Send( &rcvdMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
               }
            }
         break;
         
         
         case START_ELECTION_MSG:
            
            disableCurrentCoordenator( );
            if( rcvdMessage.receiver == my_id ) //cycle completed
            {
               printf("Process %d says: ELECTION_CANDIDATURE completed the cycle, the winner, and next coordenator process will be %d, spreading the info\n", my_id, rcvdMessage.data);
               sentMessage.sender = my_id;
               sentMessage.receiver = my_id;
               sentMessage.data = rcvdMessage.data;
               sentMessage.msgType = ELECT_COORDENATOR_MSG;
               MPI_Send( &sentMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
            }
            else
            {
               if( my_id > rcvdMessage.data )
               {
                  rcvdMessage.data = my_id;
                  printf("Process %d says: My rank is higher than the current candidate, I'll be the new candidate\n", my_id);
               }
               
               printf("Process %d says: Redirecting the candidature message\n", my_id);
               
               MPI_Send( &rcvdMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
            }
         
         break;
         
         
         case ELECT_COORDENATOR_MSG:
         
            allProcessStatus[rcvdMessage.data] = COORDENATOR_PROC;
            if( rcvdMessage.receiver == my_id ) //cycle completed
            {
               printf("Process %d says: ELECT_COORDINATOR complete the cycle, everyone knows the new coordenator \n", my_id);
            }
            else
            {
               printf("Process %d says: Updating the new coordenator \n", my_id);
               MPI_Send( &rcvdMessage, sizeof(struct Message), MPI_CHAR, getNextActiveProcess( ), TEMP_TAG, MPI_COMM_WORLD);
            }
         break;
      }
}
   
   MPI_Finalize();
   
}
