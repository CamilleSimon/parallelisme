#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv) {
	int rank, size, left, right;
	int k, m, i;
	MPI_Status status;
	char *buf;
	double start, end;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	left = rank == 0 ? size - 1 : rank - 1;
	right = rank == size - 1 ? 0 : rank + 1;

	k = atoi(argv[1]);
	m = atoi(argv[2]);
	buf = malloc(k);

	if (rank == 0){  
		start = MPI_Wtime();
		for (i = 0; i < m; i++) {
			MPI_Send(buf, k, MPI_CHAR, right, 66, MPI_COMM_WORLD);
			MPI_Recv(buf, k, MPI_CHAR, left, 66, MPI_COMM_WORLD, &status);
		}
		end = MPI_Wtime();
		printf("%f seconds", end - start);
	} else {
		for (i = 0; i < m; i++) {  
			MPI_Recv(buf, k, MPI_CHAR, left, 66, MPI_COMM_WORLD, &status);
			MPI_Send(buf, k, MPI_CHAR, right, 66, MPI_COMM_WORLD);
		}
	}

	MPI_Finalize();
	return 0;
}