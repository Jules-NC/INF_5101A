
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <math.h>

#include "mpi.h"

void laplace(int N, int lignes, double * tab){
	double fnew[N*lignes];
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
	}
	//memcpy(tab+N, fnew, N*(lignes-2)*sizeof(double));
	for(int i=N; i<N*(lignes-1); i++){
		tab[i] = fnew[i];	
	}
}


void share(int NPROC, int rank, int N, int lines, double* tab, MPI_Status status){
	/**
	The order here is very important to avoid deadlocks !!!
	Receive/send comes in pairs.
	**/
	if(NPROC == 1) return;
	if(rank == 0){
		MPI_Send(tab+N*(lines-2), N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD);
		MPI_Recv(tab+N*(lines-1), N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD, &status);
	}
	else if(rank == NPROC-1){
		MPI_Recv(tab, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD, &status);
		MPI_Send(tab+N, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD);
	}
	else{
		// Receive from left, send to right | send to right, receive from left
		MPI_Recv(tab, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD, &status);
		MPI_Send(tab+N, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD);
		MPI_Send(tab+N*(lines-2), N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD);
		MPI_Recv(tab+N*(lines-1), N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD, &status);

		printf("proc %d receiveid all the thingy\n", rank);
	}

	// wait for all the messages of all the processes to be received
	MPI_Barrier(MPI_COMM_WORLD);
}

void matrix_pload( char file[], double* tab, int N, int NPROC, int nb_lignes, int rank, MPI_Status status) {
	if (rank == 0){		//si je suis main
		FILE *f;
		if ((f = fopen (file, "r")) == NULL) {
			perror ("matrix_load : fopen ");
			printf("THEREIS AN ERRORE INZE LODING\n");
		}	//ouvre le fichier

		for ( int i=0; i<NPROC; i++) {
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
	if(rank!=0) MPI_Recv(tab, N*nb_lignes, MPI_DOUBLE, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, &status);
}


void setLineToConst(double* tab, int size, double value){
	for(int i=0; i<size; i++){
		tab[i] = value;
	}
}


int main(int argc, char *argv[])
{
	char filename[250]="mat5000";
	int N = 512;
	int NPROC;
	int RANK;
	int nb_lignes;
	double * localMatrix ;
	int localMatrixLinesNumber;	

	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &RANK);
	MPI_Comm_size(MPI_COMM_WORLD, &NPROC);
	
	nb_lignes = N/NPROC;
	if(nb_lignes<2) {
		printf("Hey mon ami ! La taille des blocs aux extremites est de %d, donc %d avec le recouvrement, ce qui ne permet aps d'acceder a la valeur 'en dessous', il faut minimum 3 lignes en tout. BISOUS\n", nb_lignes, nb_lignes+1);
		return -1;
	}

	// localmatrix init by NPROC ID (0 and NPROC-1 are 1 line smaller)
	if(RANK==0 || RANK==NPROC-1){
		localMatrix = malloc(sizeof(double)*N*(nb_lignes+1));
		localMatrixLinesNumber = nb_lignes+1;
		memset(localMatrix, 0, sizeof(double)*N*(nb_lignes+1));
	} else {
		localMatrix = malloc(sizeof(double)*N*(nb_lignes+2));
		localMatrixLinesNumber = nb_lignes+2;
		memset(localMatrix, 0, sizeof(double)*N*(nb_lignes+2));
	}

	int displacement = N;
	if(RANK==0) displacement = 0;

	// load matrix to localMatrix
	matrix_pload(filename, (localMatrix+displacement), N, NPROC, nb_lignes, RANK, status);

	// set first and last line to -1
	if(RANK==0) setLineToConst(localMatrix, N, -1);
	if(RANK==NPROC-1) setLineToConst(localMatrix+N*(nb_lignes), N, -1);

	share(NPROC, RANK, N, localMatrixLinesNumber, localMatrix, status);

	if(RANK==1) localMatrix[16] = 420;

	laplace(N, localMatrixLinesNumber, localMatrix);

	share(NPROC, RANK, N, localMatrixLinesNumber, localMatrix, status);


	if (RANK == 0)
	{
		printf("Size local matrix: %d\n", localMatrixLinesNumber);
		for(int i=0; i<N*localMatrixLinesNumber; i++){
			printf("%f ", localMatrix[i]);
		}
		printf("\n");
	}
	MPI_Finalize();
	return 0;
}
