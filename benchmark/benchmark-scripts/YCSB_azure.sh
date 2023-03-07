#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
OPCOUNT=1000
RECCOUNT=1000
PORT=3305
WORKLOAD=
SHARDS=
ENCRYPT=
# store path to the benchmark directory since it is needed later

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script can be used to run the YCSB benchmark.

   Usage: $PROGNAME

   required arguments:
     -w, --workload CHAR      The type of workload that should be used for the benchmarking => possible parameters: a, b, c, d, e
     -o, --operationcount     Number of operations that are executed
     -r, --recordcount        Number of records that are used
     -s, --shards             Number of shards
     -e, --encrypt            Use encryption or not
   
   optional arguments:
     -h, --help               Show this help message and exit

HEREDOC
}

# ARGUMENTS
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -h|--help)
    usage
    exit 0
    ;;
    -o|--operationcount)
    OPCOUNT="$2"
    shift # past argument
    shift # past value
    ;;
    -r|--recordcount)
    RECCOUNT="$2"
    shift # past argument
    shift # past value
    ;;
    -s|--shards)
    SHARDS="$2"
    shift # past argument
    shift # past value
    ;;
    -e|--encrypt)
    ENCRYPT="$2"
    shift # past argument
    shift # past value
    ;;
    -w|--workload)
    WORKLOAD="$2"
    shift # past argument
    shift # past value
    ;;
    *) shift ;;     # not used parameters
esac
done

# make sure the benchmark VM is in a clean state before running the benchmarking procedures
ssh t-b01 'docker stop $(docker ps -q); docker container prune -f' > /dev/null 2>&1 &

# Connect to VM via SSH and run the YCSB_local command on this machine
ssh t-b01 "cd ~/../../trustdble-benchmark;./run_benchmark YCSB_local -w ${WORKLOAD} -r ${RECCOUNT} -o ${OPCOUNT} -s ${SHARDS} -e ${ENCRYPT}"
# Navigate to the Directory where the benchmarking results are stored and print the content of the .txt files
ssh t-b01 "cd ~/../../trustdble-benchmark/benchmark_results;echo "";echo 'Results for the loading phase of workload ${WORKLOAD}:';cat YCSB_workload_${WORKLOAD}_S_${SHARDS}_E_${ENCRYPT}_LOAD.txt"
ssh t-b01 "cd ~/../../trustdble-benchmark/benchmark_results;echo "";echo 'Results for the run phase of workload ${WORKLOAD}:';cat YCSB_workload_${WORKLOAD}_S_${SHARDS}_E_${ENCRYPT}_RUN.txt"