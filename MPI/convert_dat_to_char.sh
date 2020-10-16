# $1 = size matrix, $2=file to read
rm -f "$2.result"
mpirun -np 1 ./read $1 $2 0 >> "$2.result"
echo "Done !"
