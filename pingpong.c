#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define COUP_SUIVANT 100
#define PERDU 101
#define BRAVO 102

void ping() {
	int score_ping, score_pong, coup;
	MPI_Status status;

	score_ping = score_pong = 0;
	while (score_ping < 21 && score_pong < 21) {
		coup = random() % 3;
		if (coup == 0) {
			score_pong++;
		} else {
			MPI_Send(&coup, 1, MPI_INT, 1, COUP_SUIVANT, MPI_COMM_WORLD);
			MPI_Recv(&coup, 1, MPI_INT, 1, COUP_SUIVANT, MPI_COMM_WORLD, &status);
			if (coup == 0)
				score_ping++;
		}
		printf("ping : score %d:%d", score_ping, score_pong);
	}
	if (score_ping == 21)
		MPI_Send(&coup, 1, MPI_INT, 1, PERDU, MPI_COMM_WORLD);
	else
		MPI_Send(&coup, 1, MPI_INT, 1, BRAVO, MPI_COMM_WORLD);
}

void pong() {
	int coup;
	MPI_Status status;

	while (1) {
		MPI_Recv(&coup, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (status.MPI_TAG == BRAVO || status.MPI_TAG == PERDU) {
			break;
		} else {
			coup = random() % 2;
			MPI_Send(&coup, 1, MPI_INT, 0, COUP_SUIVANT, MPI_COMM_WORLD);
		}
	}
	if (status.MPI_TAG == BRAVO)
		printf("pong : j'ai gagné
");
	else
		printf("pong : j'ai perdu
");
}

int main(int argc, char **argv) {  
	int rank;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	srandom(time(NULL));

	if (rank == 0)
		ping();
	else
		pong();
	MPI_Finalize();
	return 0;
}