#!/bin/bash

# DEFAULTS
OPCOUNT=1
PORT=$1
HOST=127.0.0.1
CONNECTION=8
RAMPUP_TIME=10
MEASURE_TIME=50
WORKLOAD=

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script can be used to run the TPC-C benchmark.

   Usage: $PROGNAME

   required arguments:
     -w, --workload CHAR  The type of workload that should be used for the benchmarking => possible parameters: a, b, c, d, e

   optional arguments:
     -h, --help                 show this help message and exit
     -o, --operationcount       number of operations that are executed (default: 1000)
     -r, --recordcount          number of records that are used (default: 1000)

HEREDOC
}

# open MySQL client and run commands for setting up the databases and tables that are needed by YCSB
echo "Start database set-up. This may take a few minutes."

mysql -u root -h127.0.0.1 -P $PORT -e "CREATE SHARED DATABASE shared_db2 type=STUB;"
mysql -u root -h127.0.0.1 -P $PORT shared_db2 < create_shared_table.sql
mysql -u root -h127.0.0.1 -P $PORT shared_db2 < add_fkey_idx.sql

echo "Database set-up finished!"

#populate the data
echo "load Data!"
./tpcc_load -h127.0.0.1 -P$PORT -d shared_db2 -u root -p ""  -w $OPCOUNT

#start Benchmark
echo "start Benchmark"
./tpcc_start -h127.0.0.1 -P $PORT -dshared_db2 -uroot  -w $OPCOUNT -c $CONNECTION -r $RAMPUP_TIME -l $MEASURE_TIME 