#!/bin/bash

TARGET_RUN="./"
INPUT_TYPE=test # THIS MUST BE ON LINE 4 for an external sed command to work!
                # this allows us to externally set the INPUT_TYPE this script will execute

BENCHMARKS=(400.perlbench 401.bzip2 403.gcc 429.mcf 445.gobmk 456.hmmer 458.sjeng 462.libquantum 464.h264ref 471.omnetpp 473.astar 483.xalancbmk 410.bwaves 416.gamess 433.milc 434.zeusmp 435.gromacs 436.cactusADM 437.leslie3d 444.namd 447.dealII 450.soplex 453.povray 454.calculix 459.GemsFDTD 465.tonto 470.lbm 481.wrf 482.sphinx3)

base_dir=$PWD/CPU2006
command_dir=$PWD/commands
for b in ${BENCHMARKS[@]}; do

   echo " -== ${b} ==-"
   mkdir -p ${base_dir}/output

   cd ${base_dir}/${b}
   SHORT_EXE=${b##*.} # cut off the numbers ###.short_exe
   if [ $b == "483.xalancbmk" ]; then 
      SHORT_EXE=Xalan #WTF SPEC???
   fi

   if [ $b == "482.sphinx3" ]; then 
      SHORT_EXE=sphinx_livepretend #WTF SPEC???
   fi
   
   # read the command file
   IFS=$'\n' read -d '' -r -a commands < ${command_dir}/${b}.${INPUT_TYPE}.cmd

   # run each workload
   count=0
   for input in "${commands[@]}"; do
      if [[ ${input:0:1} != '#' ]]; then # allow us to comment out lines in the cmd files
         cd ${base_dir}/${b}/run/run_base_test_riscv.0000
         cmd="time taskset -c 0 ./${SHORT_EXE}_base.riscv ${input} > ${base_dir}/output/${SHORT_EXE}.${count}.out"
         echo "workload=[${cmd}]"
         eval ${cmd}
         ((count++))
      fi
   done
   echo ""

done


echo ""
echo "Done!"
