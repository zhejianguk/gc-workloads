#!/bin/bash

source_file="Null"
null="Null"

# Input flags
while getopts c:r:o:m: flag
do
	case "${flag}" in
		s) source_file=${OPTARG};;
	esac
done


set -x

echo "Running workload: $source_file.riscv"

cd root/$source_file
/usr/bin/time -f "%S,%M,%F" ./$source_file.riscv 10000 2> ../run_result.csv
poweroff