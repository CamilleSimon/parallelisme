#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

int main(int argc, char **argv) {  
	int rank, size;
	char hostname[100];

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	gethostname(hostname, 100);

	printf("Hello from process %d of %d on host %s", rank, size, hostname);

	MPI_Finalize();
	return 0;
}