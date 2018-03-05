#include <stdio.h>
#include "mpi.h"
#define n 100
#define INIT 0
#define RING 1
#define RESU 2


int main(int argc, char **argv) {

	// Initialiser  l ’environnement d’exécution
	MPI_Init(&argc, &argv);

	// Initialisation du nombre de proc et le rang du proc courrant
	int size, rank;	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Status status;
	
	// R correspond au nombre de lignes par processus
	int R = (int) n / size;
	if(size > n) R = 1;

	// 
	MPI_Datatype LIGNE;
	MPI_Type_contiguous(n, MPI_INT, &LIGNE);
	MPI_Type_commit(&LIGNE);

	int A[n][n], B[n][n], C[n][n], colonne[n], ligne[n], local_result[n], i, j;

	if(rank == 0){
		// Initialisation de la matrice
		for(i = 0; i < n; i++){
			for(j = 0; j < n; j++){
				A[i][j] = rand() % 10;
				B[i][j] = rand() % 10;
			}
		}
		
		printf("Matrice A : \n");
		for(i = 0; i < n; i++){
			for(j = 0; j < n; j++)
				printf("%d ", A[i][j]);
			printf("\n");
		}
		
		printf("===========\nMatrice B : \n");
		for(i = 0; i < n; i++){
			for(j = 0; j < n; j++)
				printf("%d ", B[i][j]);
			printf("\n");
		}
		printf("===========\nResult : \n");

		// On envoie une colonne de B à chaque proc
		for(i = 1; i < n; i++){
			for(j = 0; j < n; j++){
				colonne[j] = B[j][i];
			}
			MPI_Send(colonne, n, MPI_INT, i, INIT, MPI_COMM_WORLD);
		}
		for(j = 0; j < n; j++){
			colonne[j] = B[j][0];
		}
	}
	else{ 
		MPI_Recv(colonne, n, MPI_INT, 0, INIT, MPI_COMM_WORLD, &status);
	}

	// On envoie une ligne de A à chaque proc
	MPI_Scatter(&A[0][0], 1, LIGNE, &ligne[0], 1, LIGNE, 0, MPI_COMM_WORLD);

	// Anneau de clacul
	int prec = (rank - 1 + size) % size;
	int suiv = (rank + 1) % size;

	for(i = 0; i < size; i++){
		local_result[(rank + i) % size] = 0;
		for(j = 0; j < n; j++)
			local_result[(rank + i) % size] += ligne[j] * colonne[j];

		MPI_Send(colonne, n, MPI_INT, prec, RING, MPI_COMM_WORLD);
		MPI_Recv(colonne, n, MPI_INT, suiv, RING, MPI_COMM_WORLD, &status);
	}

	// On remonte au proc 0 le résulta de notre ligne en charge
	if(rank != 0)
		MPI_Send(local_result, n, MPI_INT, 0, RESU, MPI_COMM_WORLD);

	if(rank == 0){
		// On récupère chaque ligne de C auprès de chaque proc
		for(i = 1; i < n; i++){
			MPI_Recv(C[i], n, MPI_INT, i, RESU, MPI_COMM_WORLD, &status);
		}

		for(i = 0; i < n; i++)
			C[0][i] = local_result[i];

		// On écrit le résultat
		for(i = 0; i < n; i++){
			for(j = 0; j < n; j++)
				printf("%d ", C[i][j]);
			printf("\n");
		}
	}

	// Terminer proprement
	MPI_Finalize();

	return 0;
}