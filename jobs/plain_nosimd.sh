#!/bin/bash

#$ -M jtakeshi@nd.edu
#$ -m abe
#$ -q *@@jung
#$ -pe smp 1
#$ -N plain_nosimd
#

#SCHEMES=("NS", "MS")
BACKENDS="-p"
#Run from root directory
EXECUTABLE=./slap
PLAIN_SIZE=16
declare -a K_PRIME_VALS=("1 2048 16" "1 4096 16" "2 2048 16" "2 4096 16" "4 8192 16" "4 16384 16" "7 8192 16" "7 16384 16")
NUM_USERS=1000 #About 10 bits
ITERATIONS=50

SUBFOLDER=plain_nosimd

mkdir -p ./results/${SUBFOLDER}/
module load gcc/9.1.0

for i in "${K_PRIME_VALS[@]}"
  do
    set -- $i
    time $EXECUTABLE -t $3 -n $NUM_USERS -i $ITERATIONS $BACKENDS -k $1 -N $2 > ./results/${SUBFOLDER}/plain_nosimd_${3}_${1}_${2}_NS.txt
    #time $EXECUTABLE -t $PLAIN_MODULUS -n $NUM_USERS -i $ITERATIONS $BACKENDS -c MS -k $1 -N $2 > ./results/${SUBFOLDER}/plain_only_${1}_${2}_MS.txt
done

python stats.py ./results/${SUBFOLDER}/*.txt > ./results/${SUBFOLDER}/plain_nosimd.rep

