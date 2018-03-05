#include <stdio.h>
#include "mpi.h"
#define n 6
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

	int A[n][n], B[n][n], C[n][n], colonne[R][n], ligne[R][n], local_result[R][n], i, j, k, l, m;
	for(i = 0; i < R; i++)
		for(j = 0; j < n; j++)
			local_result[i][j] = 0;

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
				printf("%d,", A[i][j]);
			printf("\n");
		}

		printf("===========\nMatrice B : \n");
		for(i = 0; i < n; i++){
			for(j = 0; j < n; j++)
				printf("%d,", B[i][j]);
			printf("\n");
		}
		printf("===========\nResult : \n");

		// On envoie R colonne de B à chaque proc
		for(i = 1; i < size; i++){
			for(k = 0; k < R; k++){
				for(j = 0; j < n; j++){
					colonne[k][j] = B[j][i*R+k];
				}
			}
			MPI_Send(colonne, n * R, MPI_INT, i, INIT, MPI_COMM_WORLD);
		}
		for(k = 0; k < R; k++){
			for(j = 0; j < n; j++){
				colonne[k][j] = B[j][k];
			}
		}
	}
	else{
		MPI_Recv(colonne, n * R, MPI_INT, 0, INIT, MPI_COMM_WORLD, &status);
	}

	// On envoie une ligne de A à chaque proc
	MPI_Scatter(&A[0][0], R, LIGNE, &ligne[0][0], R, LIGNE, 0, MPI_COMM_WORLD);

	/*printf("RANG %d : \n", rank);
	printf("ligne %d : \n", rank);
	for(i = 0; i < R; i++){
		for(j = 0; j < n; j++)
			printf("%d,", ligne[i][j]);
		printf("\n");
	}
	printf("colonne %d : \n", rank);
	for(i = 0; i < R; i++){
		for(j = 0; j < n; j++)
			printf("%d,", colonne[i][j]);
		printf("\n");
	}*/

	// Anneau de clacul
	int prec = (rank - 1 + size) % size;
	int suiv = (rank + 1) % size;

	for(i = 0; i < n; i+=R){
		for(k = 0; k < R; k++)
			for(l = 0; l < R; l++)
				for(j = 0; j < n; j++)
					local_result[l][(i + rank*R + k) %n] += ligne[l][j] * colonne[k][j];

		MPI_Send(colonne, n * R, MPI_INT, prec, RING, MPI_COMM_WORLD);
		MPI_Recv(colonne, n * R, MPI_INT, suiv, RING, MPI_COMM_WORLD, &status);
	}

	// On remonte au proc 0 le résulta de notre ligne en charge
	if(rank != 0)
		MPI_Send(local_result, n * R, MPI_INT, 0, RESU, MPI_COMM_WORLD);

	if(rank == 0){
		for(k = 0; k < R; k++)
			for(i = 0; i < n; i++)
				C[k][i] = local_result[k][i];

		// On récupère chaque ligne de C auprès de chaque proc
		for(i = 1; i < size; i++){
			MPI_Recv(local_result, n * R, MPI_INT, i, RESU, MPI_COMM_WORLD, &status);
			for(j = 0; j < R; j++)
				for(k = 0; k < n; k++)
					C[i*R+j][k] = local_result[j][k];
		}

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