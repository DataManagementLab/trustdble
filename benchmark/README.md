# TRUSTDBLE-Benchmark
The TRUSTDBLE-Benchmark repository is used for evaluating the performance of TRUSTDBLE by using standard evaluation benchmarks, e.g., YCSB.

## Repository Structure
The structure of the repositoray is as follows:
```
trustdble-benchmark
├── benchmark_results   # Used for storing the results of the performance evaluation; folder will appear only after benchmarking was performed
├── benchmark_scripts   # Stores shell-scripts that allow to run evaluation procedures automatically
    ├── YCSB_azure      # Shell script that can be used to run YCSB benchmarking procedures on an azure instance
    ├── YCSB_local      # Shell script that can be used to run YCSB benchmarking procedures locally
├── benchmarks_src      # Stores the source code of the benchmarks that can be used for evaluation
    ├── references      # Folder used to store important references, e.g., MySQL connector
    ├── YCSB            # Stores the source code of YCSB
├── .gitignore          # Defines directories and files that git should ignore, e.g., the benchmark_results directory
├── .gitmodules         # Needed since the source code within benchmarks_src is added by using the git submodule functionality
├── README.md           # The file you are currently looking up
├── run_benchmark.sh    # Main helper script that is a wrapper for the benchmark_scripts
```

## Perform benchmarking procedures
Make sure to clone the repository and its submodules by executing the following command at the intended location:
```
git clone --recurse-submodules --remote-submodules git@bitbucket.org:trustdble/trustdble-benchmark.git
```
The following sections provide detailed information for various performance benchmarks.

### Benchbase
1. to build the benchmark follow the instructions in the README under the benchbase repository (when working with shared databases create the database and tables using the create_shared_tables script sinstead of using the --create flag)

if you get an error while building the benchmark you might have to manually export the JAVA_HOME path.
to so run thhis command:
```
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
```
2. to adjust the amount of the data loaded ,you can edit the benchbase/src/main/java/com/oltpbenchmark/benchmarks/tpcc/TPCCConfig.java file
3. before running the benchmark , edit the file benchbase/target/benchbase-mysql/config/mysql/sample_tpcc_config.xml with your server port, username and password

### YCSB LOCAL
1. Check if your system fulfills the following preqrequisites:
    * Python 2 (Check by running the following command in your terminal: **python -V**)
    * Maven 3 (Check by running the following command in your terminal: **mvn -V**)
2. Navigate to the YCSB folder and run the following Maven-command. Then go back to the highest level of the folder structure. **NOTE:** This only needs to be done when this is the first time performing YCSB benchmarking procedures. 
    ```
    cd benchmarks_src/YCSB
    mvn clean package
    cd ../..
    ```
3. Leverage the helper script for performing the evaluation. Therefore, use the following commands (**NOTE:** Make sure that no server is running on port 3305 before running the command below.):
    ```
    ./run_benchmark YCSB_local --workload {a|b|c|d|e} -o INT -r INT -s INT -e {true|false}
    Example: ./run_benchmark YCSB_local --workload a -o 1000 -r 1000 -s 1 -e true
    ```
    For the workload parameter, please refer to the table below for detailed information:

    |Workload|Description|
    |--------|-----------|
    |a |Read: 50%, Update: 50%|
    |b |Read: 95%, Update: 5%|
    |c |Read: 100%|
    |d |Read: 95%, Insert: 5%|
    |e |Scan: 95%, Insert: 5%|

    For help with the command shown above, please use the following functionality:
    ```
    ./run_benchmark YCSB_local --help
    ```

### YCSB ON AZURE

0. Before following the steps below, make sure that you can connect to the VM bench01 via SSH. For setting up the SSH connection, please refer to the following [presentation](https://trustdble.sharepoint.com/:p:/s/Development/Ebi5bewM1gVCjdZSor7mN6cBvCpPd6P_191cN_dHWgT_bA?e=rddVVb&isSPOFile=1). Make also sure that you name the VM correctly in the .ssh/config file: bench01 should be accessible by **t-b01**. Furthermore, make sure that you import a docker image of the trustdble server to **t-b01**. For creating this image, refer to the documentation of the [TrustDBle Demo repository](https://bitbucket.org/trustdble/trustdble-demo/src/master/). Upload the created image via sftp to **t-b01** and import it by using the **docker load** command. Depending on the name of the image, it might be necessary to update the docker compose file located at **benchmarks_src/references/docker-compose.yaml**.
1. Make sure that the VPN connection is established.
2. Start the bench01 VM on the Azure [website](https://portal.azure.com/#home).
3. Use the helper script for performing the evaluation. Therefore, run the following command:
    ```
    ./run_benchmark YCSB_azure -w {a|b|c|d|e} -o INT -r INT -s INT -e {true|false}
    Example: ./run_benchmark YCSB_azure -w a -o 1000 -r 1000 -s 1 -e true
    ```

    For the workload parameter, please refer to the table below for detailed information:

    |Workload|Description|
    |--------|-----------|
    |a |Read: 50%, Update: 50%|
    |b |Read: 95%, Update: 5%|
    |c |Read: 100%|
    |d |Read: 95%, Insert: 5%|
    |e |Scan: 95%, Insert: 5%|

    For help with the command shown above, please use the following functionality:
    ```
    ./run_benchmark YCSB_azure --help
    ``` 
4. Shut down the bench01 VM on the Azure website [website](https://portal.azure.com/#home).    