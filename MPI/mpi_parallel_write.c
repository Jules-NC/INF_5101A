/*
Simple MPI-IO program that demonstrate parallel writing to a file.
Compile the program with 'mpicc -O2 writefile1.c -o writefile1'
*/

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include "mpi.h"

#define FILENAME "mat_result"

int main(int argc, char* argv[]) {
    setlocale(LC_NUMERIC, "");  // print float with comma separator
    int i, np, me;
    int BUFFSIZE;
    float * buf;            // The buffer to write
    MPI_File myfile;        // Shared file
    fflush(0);

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    BUFFSIZE = 10000*10000/np;
    buf = (float *) malloc((BUFFSIZE)*sizeof(float));     
    
    // Initialize buf with characters. Process 0 uses 'a', process 1 'b', etc.
    for (i=0; i<BUFFSIZE; i++) {
        buf[i] = me*BUFFSIZE+i;
    }

    // Open the file
    MPI_File_open (MPI_COMM_WORLD, FILENAME, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &myfile);
    // Set the file view
    MPI_File_set_view(myfile, me*BUFFSIZE*sizeof(float), MPI_FLOAT, MPI_FLOAT, "native", MPI_INFO_NULL);
    // Write buf to the file
    MPI_File_write(myfile, buf, BUFFSIZE, MPI_FLOAT, MPI_STATUS_IGNORE);
    // Close the file
    MPI_File_close(&myfile);

    if (me==0) {
        printf("%d processes wrote %'d double\n", np, np*BUFFSIZE);
        printf("Done\n");
    }
    MPI_Finalize();
    exit(0);
}