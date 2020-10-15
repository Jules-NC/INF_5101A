/*
Simple MPI-IO program that demonstrate parallel writing to a file.
Compile the program with 'mpicc -O2 writefile1.c -o writefile1'
*/

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

#define BUFSIZE 10
#define FILENAME "mat_result"

int main(int argc, char* argv[]) {
  int i, np, me;
  const int nrchar=26;   /* Processes use the 25 characters a - z */ 
  double buf[BUFSIZE];     /* The buffer to write */
  MPI_File myfile;       /* Shared file */

  /* Initialize MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &me);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  /* Initialize buf with characters. Process 0 uses 'a', process 1 'b', etc. */
  for (i=0; i<BUFSIZE; i++) {
    buf[i] = me*BUFSIZE+i;
  }

  /* Open the file */
  MPI_File_open (MPI_COMM_WORLD, FILENAME, MPI_MODE_CREATE | MPI_MODE_WRONLY,
		 MPI_INFO_NULL, &myfile);
  /* Set the file view */
  MPI_File_set_view(myfile, me*BUFSIZE*sizeof(double), MPI_DOUBLE, MPI_DOUBLE, 
		    "native", MPI_INFO_NULL);
  /* Write buf to the file */
  MPI_File_write(myfile, buf, BUFSIZE, MPI_DOUBLE, MPI_STATUS_IGNORE);
  /* Close the file */
  MPI_File_close(&myfile);

  if (me==0) {
    printf("%d processes wrote %d double\n", np, np*BUFSIZE);
    printf("Done\n");
  }
  MPI_Finalize();
  exit(0);
}