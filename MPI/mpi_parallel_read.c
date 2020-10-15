/*
Simple MPI-IO program that demonstrate parallel reading from a file.
Compile the program with 'mpicc -O2 readfile1.c -o readfile1'
Check if the number of elements read per process is correct !!!
*/

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

#define FILENAME "mat_result"

int main(int argc, char* argv[]) {
  int np, myid;
  int bufsize, nrchar;
  float *buf;          /* Buffer for reading */
  MPI_Offset filesize;
  MPI_File myfile;    /* Shared file */
  MPI_Status status;  /* Status returned from read */

  /* Initialize MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  /* Open the file */
  MPI_File_open (MPI_COMM_WORLD, FILENAME, MPI_MODE_RDONLY,
		 MPI_INFO_NULL, &myfile);
  /* Get the size of the file */
  MPI_File_get_size(myfile, &filesize);
  /* Calculate how many elements that is */
  filesize = filesize/sizeof(float);
  /* Calculate how many elements each processor gets */
  bufsize = filesize/np;
  /* Allocate the buffer to read to, one extra for terminating null char */
  buf = (float *) malloc((bufsize+1)*sizeof(float));
  /* Set the file view */
  MPI_File_set_view(myfile, myid*bufsize*sizeof(float), MPI_FLOAT, MPI_FLOAT, 
		    "native", MPI_INFO_NULL);
  /* Read from the file */
  MPI_File_read(myfile, buf, bufsize, MPI_FLOAT, &status);
  /* Find out how many elemyidnts were read */
  MPI_Get_count(&status, MPI_FLOAT, &nrchar);
  /* Set terminating null char in the string */
  printf("Process %2d read %d characters\n", myid, nrchar);

  /* Close the file */
  MPI_File_close(&myfile);

  if (myid==0) {
    for(int i=0; i<bufsize; i++){
        //printf("%f ", buf[i]);
    }
    printf("Done\n");
  }
  MPI_Finalize();
  exit(0);
}