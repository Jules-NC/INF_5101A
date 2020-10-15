mpicc -o read mpi_parallel_read.c -O3
mpicc -o write mpi_parallel_write.c -O3 &&  
mpirun -np 4 ./write
mpirun -np 4 ./read
