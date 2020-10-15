
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <math.h>

#include "mpi.h"

typedef struct {
	int NPROCS;
	int rank;
	MPI_Status status;
	int N; // matrix length
	int lines;
	int internal_lines;
	double * matrix;
	double total_error;

} NodeInfo;

void laplace(NodeInfo * node){
	/** 
	* Here we do the laplace function ONE TIME, and write the total error in the NodeInfo structure of ALL NODES (bc why not) 
	**/
	 // we modify a non-pointer value (node.total_error), so need struct pointer to avoid copy
	int N = node->N, lignes = node->lines;
	double* tab = node->matrix;

	double fnew[N*lignes];
	double error = 0;
	for(int i=N; i<N*(lignes-1); i++){ 
		if((i+1)%N*i%N == 0) { // If end or beginning of the line (i%N = begining and (i+1)%N = end)
			fnew[i] = tab[i];
			continue;
		} 
		double left = tab[i-1];
		double right = tab[i+1];
		double up = tab[i-N];
		double down = tab[i+N];

		double result = 0.25*(left+right+up+down);
		fnew[i] = result;

		error += (tab[i]-fnew[i])*(tab[i]-fnew[i]) ;
	}

	//memcpy(tab+N, fnew, N*(lignes-2)*sizeof(double));
	for(int i=N; i<N*(lignes-1); i++){
		tab[i] = fnew[i];	
	}
	
	MPI_Allreduce(&error, &(node->total_error), 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
}



void share(NodeInfo node){
	/**
	The order here is very important to avoid deadlocks !!!
	Receive/send comes in pairs.

	We only modify the POINTER tab, so no need to pass by pointer, the size of NodeInfo is limited
	**/
	int N = node.N, rank = node.rank, lines = node.lines;
	double * tab = node.matrix;

	if(node.NPROCS == 1) return;
	if(rank == 0){
		MPI_Send(node.matrix+N*(lines-2), N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD);
		MPI_Recv(tab+N*(lines-1), N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD, &node.status);
	}
	else if(rank == node.NPROCS-1){
		MPI_Recv(tab, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD, &node.status);
		MPI_Send(tab+N, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD);
	}
	else{
		// Receive from left, send to right | send to right, receive from left
		MPI_Recv(tab, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD, &node.status);
		MPI_Send(tab+N, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD);
		MPI_Send(tab+N*(lines-2), N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD);
		MPI_Recv(tab+N*(lines-1), N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD, &node.status);
	}

	// wait for all the messages of all the processes to be received
	MPI_Barrier(MPI_COMM_WORLD);
}

void matrix_pload( char file[], double* tab, NodeInfo node) {
	int N = node.N, rank = node.rank, nb_lignes = node.internal_lines;
	if (rank == 0){		//si je suis main
		FILE *f;
		if ((f = fopen (file, "r")) == NULL) {
			perror ("matrix_load : fopen ");
			printf("THEREIS AN ERRORE INZE LODING\n");
		}	//ouvre le fichier

		for ( int i=0; i<node.NPROCS; i++) {
			double buff[N*nb_lignes];
			for(int j=0; j<N*nb_lignes; j++){
				fscanf (f, "%lf", (buff +j));
			}
			if(i!=0) MPI_Send(buff, N*nb_lignes, MPI_DOUBLE, i, 99, MPI_COMM_WORLD);
			if(i==0) memcpy(tab, buff, N*nb_lignes*sizeof(double));

			printf("%d -> %d\n", rank, i);

		}
		fclose (f);
	}
	if(rank!=0) MPI_Recv(tab, N*nb_lignes, MPI_DOUBLE, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, &node.status);
}


void setLineToConst(double* tab, int size, double value){
	for(int i=0; i<size; i++){
		tab[i] = value;
	}
}

void init_load(char filename[], NodeInfo * node){
	int N, nb_lignes;
	// Init and set MPI rank and size
	MPI_Comm_rank(MPI_COMM_WORLD, &(node->rank));
	MPI_Comm_size(MPI_COMM_WORLD, &(node->NPROCS));

	N = node->N;
	nb_lignes = node->N/node->NPROCS;

	node->internal_lines = nb_lignes;

	if(node->internal_lines<2) {
		printf("Hey mon ami ! La taille des blocs aux extremites est de %d, donc %d avec le recouvrement, ce qui ne permet aps d'acceder a la valeur 'en dessous', il faut minimum 3 lignes en tout. BISOUS\n", nb_lignes, nb_lignes+1);
	}

	// different malloc if first or last block (different internal matrix size)
	if(node->rank==0 || node->rank==node->NPROCS-1){
		node->matrix = malloc(sizeof(double)*N*(node->internal_lines+1));
		node->lines = node->internal_lines+1;
		memset(node->matrix, 0, sizeof(double)*N*(node->internal_lines+1));
	} else {
		node->matrix = malloc(sizeof(double)*N*(node->internal_lines+2));
		node->lines = node->internal_lines+2;
		memset(node->matrix, 0, sizeof(double)*N*(node->internal_lines+2));
	}

	// load matrix to localMatrix with a stride of N except for rank 0 (a*0 = 0)
	matrix_pload(filename, (node->matrix+(N*(node->rank!=0))), *node);

	// set first and last line to -1 
	if(node->rank==0) setLineToConst(node->matrix, N, -1);
	if(node->rank==node->NPROCS-1) setLineToConst(node->matrix+(N*(node->lines-1)), N, -1);
}

int main(int argc, char *argv[])
{
	char filename[250]="mat4";
	NodeInfo node = {.N = 4, .total_error = 0};

	MPI_Init(&argc, &argv);
	
	init_load(filename, &node);
	share(node); 

	if(node.rank==1) node.matrix[16] = 420;


	laplace(&node);

	if(node.rank==0){
		node.total_error = sqrt(node.total_error);
		printf("LOLI 42 DIT: %f\n", node.total_error);
	}
	share(node);


	if (node.rank == node.NPROCS-1)
	{
		printf("Size local matrix: %d\n", node.lines);
		for(int i=0; i<node.N*node.lines; i++){
			printf("%f ", node.matrix[i]);
		}
		printf("\n");

	}
	MPI_Finalize();
	return 0;
}
