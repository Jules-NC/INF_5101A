/*
Simple MPI-IO program that demonstrate parallel reading from a file.
Check if the number of elements read per process is correct !!!
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mpi.h"

typedef struct {
	int NPROCS;
	int rank;
	MPI_Status status;
	int N; // matrix length
	int lines;
	double * matrix;

} NodeInfo;


void matrix_pload( char file[], NodeInfo node) {
    double * tab = node.matrix;
	int N = node.N, rank = node.rank, nb_lignes = node.lines;
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


void print_node(NodeInfo node, int root){

	MPI_Barrier(MPI_COMM_WORLD); // avoid interleaved prints

	if(node.rank != root) return;

	printf("|========[Node %d]========\n", node.rank);
	printf("|N: %d\n", node.N);
	printf("|ltotal lines: %d\n", node.lines);
	printf("|--------MATRIX--------\n|");
	for(int i=0; i<node.N*node.lines; i++){
		if(node.matrix[i] >= 0) printf("%06.3f ", node.matrix[i]);
		if(node.matrix[i] <= 0) printf("%06.3f ", node.matrix[i]);

		if( (i+1) % node.N == 0) printf("\n|");
	}
	printf("\n\n");


}

void parallel_write(char filename[], NodeInfo node){
    MPI_File myfile; 
    int BUFFSIZE = node.lines*node.N;
    double * buf = node.matrix;     

    // Open the file
    MPI_File_open (MPI_COMM_WORLD, filename, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &myfile);
    // Set the file view
    MPI_File_set_view(myfile, node.rank*BUFFSIZE*sizeof(double), MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
    // Write buf to the file
    MPI_File_write(myfile, buf, BUFFSIZE, MPI_DOUBLE, MPI_STATUS_IGNORE);
    // Close the file
    MPI_File_close(&myfile);

    if(node.rank==0){
        printf("%d processes wrote %'d double\n", node.NPROCS, node.NPROCS*BUFFSIZE);
    }
}

int main(int argc, char* argv[]){
    int N = atoi(argv[1]);
    char filename[255];
    char destname[255];
    strcpy(filename, argv[2]);
    strcpy(destname, filename);
    strcat(destname, ".dat");

    MPI_Init(&argc, &argv);

    NodeInfo node;
    node.N = N;

    MPI_Comm_rank(MPI_COMM_WORLD, &(node.rank));
    MPI_Comm_size(MPI_COMM_WORLD, &(node.NPROCS));

    node.lines = node.N/node.NPROCS;
    node.matrix = (double*) malloc(node.N*node.lines*sizeof(double));

    matrix_pload(filename, node);
    print_node(node, 3);

    parallel_write(destname, node);

    if(node.rank == 0){
        printf("Done\n");
        printf("Done %s\n", destname);

    }

    MPI_Finalize();
    exit(0);
}