
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <math.h>

#include "mpi.h"

/*
void my_sum_function(void* inputBuffer, void* outputBuffer, int* len, MPI_Datatype* datatype)
{
    int* input = (int*)inputBuffer;
    int* output = (int*)outputBuffer;
 
    for(int i = 0; i < *len; i++)
    {
        output[i] += input[i];
    }
}
*/

float f(float x){
	return 1/(1+x*x);
}

int main(int argc, char *argv[])
{
	int NPROC;
	float PAS = 1000;
	int myrank;
	float result;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &NPROC);

	// Create the operation handle
    //MPI_Op operation;
    //MPI_Op_create(&my_sum_function, 1, &operation);
	float sum = 0;

	float start = myrank/NPROC;
	float end = (myrank+1)/NPROC;
	float increment = (end-start)/PAS;

	for(float x=start; x<end; x+=increment){
		sum += f(x)*increment;
	}

	
	MPI_Reduce(&sum, &result, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

	if (myrank == 0)
	{
		printf("P%d NPROC result:%f expected:%f\n", myrank, result, 3.14/4);
	}
	MPI_Finalize();
	return 0;
}
