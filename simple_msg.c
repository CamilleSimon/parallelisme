#include <stdio.h>
#include <string.h>
#include "mpi.h"

#define TAG 99

int main(int argc, char **argv) {  
   int rank, size;
   int i;
   char msg[100];
   MPI_Status status;

   MPI_Init(&argc, &argv);
   
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Comm_size(MPI_COMM_WORLD, &size);
   
   if (rank == 0) {  
      for (i = 1; i < size; i++) {
         sprintf(msg, "Hello %d!", i);
         MPI_Send(msg, strlen(msg) + 1, MPI_CHAR, i, TAG, MPI_COMM_WORLD);
      }
   }
   else {
      MPI_Recv(msg, 100, MPI_CHAR, 0, TAG, MPI_COMM_WORLD, &status);
      printf("%d received %s", rank, msg);
   }
   
   MPI_Finalize();
   return 0;
}