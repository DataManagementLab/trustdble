#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
OPCOUNT=
RECCOUNT=
PORT=3305
WORKLOAD=
SHARDS=
ENCRYPT=
# store path to the benchmark directory since it is needed later
BENCHMARK_PATH=$(pwd)

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
    shift
    shift
    ;;
    -w|--workload)
    WORKLOAD="$2"
    shift # past argument
    shift # past value
    ;;
    *) shift ;;     # not used parameters
esac
done

echo "Start TrustDBle server. This may take approx. one minute."

# start trustdble server in the background => don't show the output of the command
cd benchmarks_src/references
TDB_RELEASE=0.2.1 docker-compose up > /dev/null 2>&1 &

# wait some time until the server is running
sleep 45

# open MySQL client and run commands for setting up the databases and tables that are needed by YCSB
echo "Start database set-up. This may take a few minutes."

mysql --user root -ppassword -P 3305 -h 172.20.1.1 -e "create shared database testYCSB type=ETHEREUM shards=${SHARDS} encrypt=${ENCRYPT}; use testYCSB; create shared table usertable (YCSB_KEY varchar(255) primary key, FIELD0 text, FIELD1 text, FIELD2 text, FIELD3 text, FIELD4 text, FIELD5 text, FIELD6 text, FIELD7 text, FIELD8 text, FIELD9 text);" > /dev/null 2>&1

echo "Database set-up finished!"

# move back to the benchmark directory
cd $BENCHMARK_PATH

# create folder benchmark_results (if it not exists already) where the results of the benchmark procedures are stored
sudo mkdir -p benchmark_results
# move back to the YCSB directory
cd benchmarks_src/YCSB

# load workload A using the provided parameters => don't show the output of the command
echo "Load workload $WORKLOAD. This may take a few minutes."
sudo bin/ycsb load jdbc -P workloads/workload${WORKLOAD} -P ../benchmark_properties.properties -cp ../references/mysql-connector-java-8.0.24.jar -p recordcount=${RECCOUNT} -p operationcount=${OPCOUNT} -p exportfile=../../benchmark_results/YCSB_workload_${WORKLOAD}_S_${SHARDS}_E_${ENCRYPT}_LOAD.txt > /dev/null 2>&1
echo "Workload $WORKLOAD loaded successfully!"


# run a workload A using the provided parameters => don't show the output of the command
echo "Run workload $WORKLOAD."
sudo bin/ycsb run jdbc -P workloads/workload${WORKLOAD} -P ../benchmark_properties.properties -cp ../references/mysql-connector-java-8.0.24.jar -p recordcount=${RECCOUNT} -p operationcount=${OPCOUNT} -p exportfile=../../benchmark_results/YCSB_workload_${WORKLOAD}_S_${SHARDS}_E_${ENCRYPT}_RUN.txt > /dev/null 2>&1
echo "Workload $WORKLOAD was run successfully!"

# navigate back to the trustdble-server directory and shut down the server => don't show the output of the command
echo "Clear resources, this may take some time."

cd ~/../../trustdble-benchmark/benchmarks_src/references
docker-compose down -v > /dev/null 2>&1

# remove all docker containers, incl. the containers that include the Ethereum instances
docker stop $(docker ps -q) > /dev/null 2>&1
docker container prune -f > /dev/null 2>&1

echo "Benchmarking finished! Find the performance evaluation in folder trustdble-benchmark/benchmark_results."