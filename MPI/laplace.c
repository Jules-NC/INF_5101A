
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <math.h>
#include <time.h>

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


void print_node(NodeInfo node, int root);

void laplace(NodeInfo * node){
	/** 
	* Here we do the laplace function ONE TIME, and write the total error in the NodeInfo structure of ALL NODES (bc why not) 
	**/
	int N = node->N, lignes = node->lines;
	double* tab = node->matrix;

	double * fnew = malloc(N*lignes*sizeof(double));
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

	// TODO: TEST SPEED DIFFERENCE	
	for(int i=N; i<N*(lignes-1); i++){
		tab[i] = fnew[i];	
	}
	// Copy buffer into matrix, faster than a for

	//memcpy(tab+N, fnew+N, N*(lignes-1)*sizeof(double));
	free(fnew);
	
	MPI_Allreduce(&error, &(node->total_error), 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	node->total_error = sqrt(node->total_error);
}


void share(NodeInfo * node){
	/**
	The order here is very important to avoid deadlocks !!!
	Receive/send comes in pairs.

	We only modify the POINTER tab, so no need to pass by pointer, the size of NodeInfo is limited
	**/
	int N = node->N, rank = node->rank, lines = node->lines;
	double * tab = node->matrix;

	if(node->NPROCS == 1) return;
	if(rank == 0){
		MPI_Send(tab+N*(lines-2), N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD);
		MPI_Recv(tab+N*(lines-1), N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD, &(node->status));
	}
	else if(rank == node->NPROCS-1){
		MPI_Recv(tab, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD, &(node->status));
		MPI_Send(tab+N, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD);
	}
	else{
		// Receive from left, send to right | send to right, receive from left
		MPI_Recv(tab, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD, &(node->status));
		MPI_Send(tab+N, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD);
		MPI_Send(tab+N*(lines-2), N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD);
		MPI_Recv(tab+N*(lines-1), N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD, &(node->status));
	}

	// wait for all the messages of all the processes to be received
	MPI_Barrier(MPI_COMM_WORLD);
}


void matrix_pload( char file[], double* tab, NodeInfo node) {
	int N = node.N, rank = node.rank, nb_lignes = node.internal_lines;
	if (rank == 0){		//si je suis main

		fflush(0);
		FILE *f;
		if ((f = fopen (file, "r")) == NULL) {
			perror ("matrix_load : fopen ");
			printf("THEREIS AN ERRORE INZE LODING\n");
		}	//ouvre le fichier

		for ( int i=0; i<node.NPROCS; i++) {
			printf("Loading block %d/%d\n", (i+1), node.NPROCS);
			double * buff = malloc(N*nb_lignes*sizeof(double));
			for(int j=0; j<N*nb_lignes; j++){
				fscanf (f, "%lf", (buff +j));
			}
			// If rank==myself, don't send to avoid buffer deadlock
			if(i!=0) MPI_Send(buff, N*nb_lignes, MPI_DOUBLE, i, 99, MPI_COMM_WORLD);
			if(i==0) memcpy(tab, buff, N*nb_lignes*sizeof(double));

			free(buff);
			//printf("%d -> %d\n", rank, i);
		}
		fclose (f);
	}
	// If rank==myself, don't receive bc no send
	if(rank!=0) MPI_Recv(tab, N*nb_lignes, MPI_DOUBLE, 0, 99, MPI_COMM_WORLD, &node.status);
}


void setLineToConst(double* tab, int size, double value){
	for(int i=0; i<size; i++){
		tab[i] = value;
	}
}


int init_load(char filename[], NodeInfo * node){
	/*
		1: Init the values of the NodeInfo struct
		2: Set the right size for each internal matrix of all nodes
		3: load the data into those matrix
		4: set the first and last lines to -1
		5: do a share() to set the recoverement lines
	*/

	int N, nb_lignes;
	// Init and set MPI rank and size
	MPI_Comm_rank(MPI_COMM_WORLD, &(node->rank));
	MPI_Comm_size(MPI_COMM_WORLD, &(node->NPROCS));

	N = node->N;
	nb_lignes = node->N/node->NPROCS;

	node->internal_lines = nb_lignes;

	if(node->internal_lines<2) {
		printf("Hey mon ami ! La taille des blocs aux extremites est de %d, donc %d avec le recouvrement, ce qui ne permet aps d'acceder a la valeur 'en dessous', il faut minimum 3 lignes en tout. BISOUS\n", nb_lignes, nb_lignes+1);
		return -1;
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

	share(node);

	return 0;
}


void print_node(NodeInfo node, int root){

	MPI_Barrier(MPI_COMM_WORLD); // avoid interleaved prints

	if(node.rank != root) return;

	printf("|========[Node %d]========\n", node.rank);
	printf("|N: %d\n", node.N);
	printf("|internal_lines: %d\n", node.internal_lines);
	printf("|ltotal lines: %d\n", node.lines);
	printf("|total_error: %f\n", node.total_error);
	printf("|--------MATRIX--------\n|");
	for(int i=0; i<node.N*node.lines; i++){
		if(node.matrix[i] >= 0) printf("%06.3f ", node.matrix[i]);
		if(node.matrix[i] <= 0) printf("%06.3f ", node.matrix[i]);

		if( (i+1) % node.N == 0) printf("\n|");
	}
	printf("\n\n");


}


int main(int argc, char *argv[])
{
	printf("LOL2rir: %ld", sizeof(float));
	printf("LOL2rir: %ld", sizeof(int));

	
	struct timeval tv1, tv2;	/* for timing */
	int status, duration;
	double MIN_ERROR = atoi(argv[3]);
	NodeInfo node = {.N = atoi(argv[1]), .total_error = MIN_ERROR*2};  // we will enter in a while so i set total_error > MIN_ERROR
	char filename[250];

	// set args
	if(node.N < 0){printf("N must be greater than 3\n"); return -1;}
	strcpy(filename, argv[2]);

	// MPI-BEGIN !
	MPI_Init(&argc, &argv);

	// load and error check
	status = init_load(filename, &node);
	if(status < 0) return status;

	// laplace algorithm
	gettimeofday( &tv1, (struct timezone*)0 );
	int i = 0;
	while(node.total_error>MIN_ERROR){
		laplace(&node);
		share(&node);
		i++;
		if(node.rank == 0) printf("iteration %d, error: %f\n", i, node.total_error);
	}
	gettimeofday( &tv2, (struct timezone*)0 );
	duration = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;

	// print to verif
	//print_node(node, 0);
	//print_node(node, 1);

	MPI_Barrier(MPI_COMM_WORLD);
	if(node.rank==0)   	printf ("computation time: %10.8f sec.\n", duration/1000000.0);
	
	free(node.matrix); // release tabs
	MPI_Finalize();
	return 0;
}
