/*
Simple MPI-IO program that demonstrate parallel reading from a file.
Compile the program with 'mpicc -O2 readfile1.c -o readfile1'
Check if the number of elements read per process is correct !!!
*/

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"


int main(int argc, char* argv[]) {
    int N = atoi(argv[1]);
    int toprint = atoi(argv[3]);
  int np, myid;
  int bufsize, nrchar;
  double *buf;          /* Buffer for reading */
  MPI_Offset filesize;
  MPI_File myfile;    /* Shared file */
  MPI_Status status;  /* Status returned from read */

  /* Initialize MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  MPI_File_open (MPI_COMM_WORLD, argv[2], MPI_MODE_RDONLY, MPI_INFO_NULL, &myfile);

  /* Calculate how many elements each processor gets */
  MPI_File_get_size(myfile, &filesize);
  filesize = filesize/sizeof(double);
  bufsize = filesize/np;
  buf = (double *) malloc((bufsize)*sizeof(double));
  MPI_File_set_view(myfile, myid*bufsize*sizeof(double), MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
  MPI_File_read(myfile, buf, bufsize, MPI_DOUBLE, &status);
  MPI_Get_count(&status, MPI_DOUBLE, &nrchar);
  //printf("Process %2d read %d characters\n", myid, nrchar);

  /* Close the file */
  MPI_File_close(&myfile);

  if (myid==toprint) {
    for(int i=0; i<bufsize; i++){
      if(buf[i] >= 0) printf("%06.3f ", buf[i]);
		  if(buf[i] <= 0) printf("%06.3f ", buf[i]);

      if((i+1)%N == 0 ) puts("");
    }
    //printf("Done\n");
  }
  MPI_Finalize();
  exit(0);
}