#!/bin/bash

#$ -M jtakeshi@nd.edu
#$ -m abe
#$ -q *@@jung
#$ -pe smp 1
#$ -N compare
#

#Run from root directory
EXECUTABLE=./compare

SUBFOLDER=others_compare

mkdir -p ./results/${SUBFOLDER}/
module load gcc/9.1.0

$EXECUTABLE ./results/${SUBFOLDER}/results.txt
