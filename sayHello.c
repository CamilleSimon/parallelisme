#include "mpi.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv){
	int rank,nb_proc,i,count;
	char msg[100];
	char hostname[100];
	MPI_Status status;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

	if(rank == 0){
		for(i = 1; i<nb_proc; i++){
			sprintf(msg, "Hello%d!", i);
			MPI_Send(msg, strlen(msg)+1, MPI_CHAR, i, 42, MPI_COMM_WORLD);
		}
	}
	else {
		gethostname(hostname,100);
		MPI_Recv(msg, 100, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		MPI_Get_count(&status, MPI_CHAR, &count);
		printf("Reçu %s sur %s ", msg, hostname);
	}
	MPI_Finalize();
	return 0;
}