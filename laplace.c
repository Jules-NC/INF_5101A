
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <math.h>

#include "mpi.h"

void matrix_pload( char file[], double* tab, int N, int NPROC, int nb_lignes, int rank, MPI_Status status) {
	int i,j;
	if ( rank == 0 ){		//si je suis main
		FILE *f;
		if ((f = fopen (file, "r")) == NULL) {
			perror ("matrix_load : fopen ");
			printf("THEREIS AN ERRORE INZE LODING\n");
		}	//ouvre le fichier

		for ( int i=0; i<NPROC; i++) {		//pour chaque ligne i de la matrice M
			double buff[N*nb_lignes];
			for(int j=0; j<N*nb_lignes; j++){
				fscanf (f, "%lf", (buff +j));
			}
			for(int j=0; j<N*nb_lignes; j++){
				printf("%lf  ", buff[j]);
			}
			printf("%d\n", N*nb_lignes);


			MPI_Send(buff, N*nb_lignes, MPI_DOUBLE, i, 99, MPI_COMM_WORLD);
			
		}
		fclose (f);
	}
	MPI_Recv(tab, N*nb_lignes, MPI_DOUBLE, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, &status);
}


int main(int argc, char *argv[])
{
	char filename[250]="mat4";
	int N = 4;

	MPI_Status status;

	MPI_Init(&argc, &argv);
	int RANK;
	MPI_Comm_rank(MPI_COMM_WORLD, &RANK);
	int NPROC;
	MPI_Comm_size(MPI_COMM_WORLD, &NPROC);
	
	int nb_lignes = N/NPROC;

	double * localMatrix ;
	if(RANK==0 || RANK==NPROC-1){	
		localMatrix = malloc(sizeof(double)*N*(nb_lignes+1));
		memset(localMatrix, -1, N*nb_lignes);
	} else {
		localMatrix = malloc(sizeof(double)*N*(nb_lignes+2));
	}
	int displacement = N;
	if(RANK==0){
		displacement = 0;
	}

	matrix_pload(filename, localMatrix+displacement, N, NPROC, nb_lignes, RANK, status);


	
	if (RANK == 0)
	{
		printf("P \n");
	}
	MPI_Finalize();
	return 0;
}
