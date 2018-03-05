// Travail réalisé en binôme par Corentin Ballot et Camille Simon
//
// Pour le bien de vos yeux, il est fortement conseillé de modifier
// les couleurs avant exécution
//
// Pour chaque jeu de vitesse de diffusion nous avons prévu un
// jeu de couleur assorti
// Retirez les commantaires des vitesses de diffusion pour découvrir
// un affichage pensé rien que pour vous

#include "gpu_bitmap.h"

#define WIDTH 800
#define HEIGHT 600
#define DIM 16

#define TAUX_REACTION_A 0.04f
#define TAUX_REACTION_I (TAUX_REACTION_A / 200)

#define TAUX_RESORPTION_A 0.06f
#define TAUX_RESORPTION_I TAUX_RESORPTION_A

// Lampe à lave
//#define VITESSE_DIFFUSION_A 50
//#define VITESSE_DIFFUSION_I 55

// Léopard revisité
//#define VITESSE_DIFFUSION_A 2
//#define VITESSE_DIFFUSION_I 22

// Années yeahyeah
#define VITESSE_DIFFUSION_A 5
#define VITESSE_DIFFUSION_I 10

#define TAUX_DIFFUSION_A 0.065f
#define TAUX_DIFFUSION_I 0.04f

#define SEUIL 130


__global__ void color(float *t, uchar4 *buf) {
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;
	if (x < WIDTH && y < HEIGHT) {
		int offset = y * WIDTH + x;
		float t3 = t[offset];
		float r, g, b;

		switch(VITESSE_DIFFUSION_A){
			case 5 :
				if (t3 < SEUIL) {
					r = 255; g = 0; b = 127;
				} else {
					r = 0; g = 47;	b = 167;
				}
				break;
			case 2 :
				if (t3 < SEUIL) {
					r = 240; g = 195; b = 0;
				} else {
					r = 63; g = 34;	b = 4;
				}
				break;
			case 50 :
				if (t3 < SEUIL) {
					r = 128; g = 0; b = 128;
				} else {
					r = 223; g = 109;	b = 20;
				}
				break;
		}

		buf[offset].x = r;
		buf[offset].y = g;
		buf[offset].z = b;
		buf[offset].w = 255;
	}
}

__global__ void reaction(float *a, float *a1, float *i, float *i1) {
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;
	if (x < WIDTH && y < HEIGHT) {
		int offset = y * WIDTH + x;

		// A est catalysé par A et inhibé par I
		a1[offset] = a[offset] + TAUX_REACTION_A * a[offset] * a[offset] / i[offset];
		// I est catalysé par A
		i1[offset] = i[offset] + TAUX_REACTION_I * a[offset] * a[offset];
		// la réaction consomme une certaine quantité de A et de I
		a1[offset] = (1 - TAUX_RESORPTION_A) * a1[offset];
		i1[offset] = (1 - TAUX_RESORPTION_I) * i1[offset];
	}
}

__global__ void diffusion(float *grille, float taux_diffusion) {
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;
	if (x < WIDTH && y < HEIGHT) {
		int offset = y * WIDTH + x;
		int top = y == HEIGHT - 1 ? offset : offset + WIDTH;
		int bottom = y == 0 ? offset : offset - WIDTH;
		int left = x == 0 ? offset : offset - 1;
		int right = x == WIDTH - 1 ? offset : offset + 1;

		grille[offset] = (1 - 4 * taux_diffusion) * grille[offset] +
		taux_diffusion * (grille[top] + grille[bottom] + grille[left] + grille[right]);
	}
}

struct Data {
	float *A;
	float *A_next;
	float *I;
	float *I_next;
	dim3 blocks;
	dim3 threads;
};

void render_callback(uchar4 *buf, Data *data, int ticks) {
	reaction<<<data->blocks, data->threads>>>(data->A, data->A_next, data->I, data->I_next);
	reaction<<<data->blocks, data->threads>>>(data->A_next, data->A, data->I_next, data->I);

	for (int s = 0; s < VITESSE_DIFFUSION_A; s++)
		diffusion<<<data->blocks, data->threads>>>(data->A, TAUX_DIFFUSION_A);
	for (int s = 0; s < VITESSE_DIFFUSION_I; s++)
		diffusion<<<data->blocks, data->threads>>>(data->I, TAUX_DIFFUSION_I);

	color<<<data->blocks, data->threads>>>(data->A_next, buf);
}

void clean_callback(Data *data) {
	HANDLE_CUDA_ERR(cudaFree(data->A));
	HANDLE_CUDA_ERR(cudaFree(data->I));
}

int main() {
	Data data;
	GPUBitmap bitmap(WIDTH, HEIGHT, &data, "Heat");

	size_t size = WIDTH * HEIGHT * sizeof(float);

	float *a_initial = (float *)calloc(WIDTH * HEIGHT, sizeof(float));
	float *i_initial = (float *)calloc(WIDTH * HEIGHT, sizeof(float));

	for (int y = 0; y <  HEIGHT * WIDTH + WIDTH; y++) {
			a_initial[y] = (rand() % 100) + 1;
			i_initial[y] = (rand() % 100) + 1;
	}

	data.blocks = dim3((WIDTH + DIM - 1) / DIM, (HEIGHT + DIM - 1) / DIM);
	data.threads = dim3(DIM, DIM);
	HANDLE_CUDA_ERR(cudaMalloc(&data.A, size));
	HANDLE_CUDA_ERR(cudaMalloc(&data.I, size));
	HANDLE_CUDA_ERR(cudaMalloc(&data.A_next, size));
	HANDLE_CUDA_ERR(cudaMalloc(&data.I_next, size));
	HANDLE_CUDA_ERR(cudaMemcpy(data.A, a_initial, size, cudaMemcpyHostToDevice));
	HANDLE_CUDA_ERR(cudaMemcpy(data.I, i_initial, size, cudaMemcpyHostToDevice));

	bitmap.animate((void (*)(uchar4*, void*, int))render_callback, (void (*)(void*))clean_callback);
	return 0;
}