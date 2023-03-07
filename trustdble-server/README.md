# TRUSTDBLE-Server
TRUSTDBLE (pronounced trustable) is a novel trusted database management system. It combines Blockchain and Database technology to provide trustworthy and auditable data management.

The current code base is build on top of Oracle MySQL and introduces new plugins and a special storage engine that allows to store data in a blockchain. More detailed architectural information can be found in the documentation.

## Project Structure
Since this project adds plugins to the [mysql-server](https://github.com/mysql/mysql-server) repository we use the same structure for building.
Some additional files/folders we added are highlighted in the following:
```
├── blockchain-adapter              # An adapter to connect to different blockchain and write data on the blockchain or read from it
├── blockchain-manager              # The blockchain-manager provides a simple interface to deploy different blockchain networks
├── example                         # This folder contains a data model with some data contracts that implement a demo use-case
├── server/                         # This folder contains the additional plugins for mysql-server and some scripts for building and running.
|   ├── docker                      # Scripts to build a docker image of trustdble
|   ├── ...
|   ├── plugin                      # Plugin folder in which mysql-server plugins are stored
|   │   ├── ...
|   |   ├── data_contracts          # Component to call and execute data contracts
|   │   ├── rewrite_trustdble       # Custom plugin for TRUSTDBLE that recognizes TRUSTDBLE SQL extensions (e.g. CREATE SHARED TABLE) and translates them to default SQL commands.
|   │   ├── ...
|   ├── storage                     # Mysql folder in which the storage engines are defined
|   │   ├── ...
|   │   ├── blockchain              # TRUSTDBLE storage engine that writes data to a blockchain.
|   │   ├── ...
|   ├── ...
|   ├── trustdble_scripts           # Collection of helper scripts that can be used to manage the repo, orchestrated via the trustdble script (see below).
|   ├── commands.md                 # List of all additional commands
|   └── trustdble                   # Main helper script that facilitates working with this repository, e.g. setting up the dev environment by installing dependencies, building the project etc.
├── .gitignore                      # Top-level gitignore
└── README.md                       # The file you are looking at currently
```
## Contribution guidelines
* Please write clean and self explanatory code. Document your code where required.
* Please adhere to the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) when writing new code.

## Requirements
We inherit most of the requirements and dependencies from the mysql repo see https://dev.mysql.com/doc/refman/8.0/en/source-installation-prerequisites.html.
All requirements can be installed by using the `init` command of the `trustdble` helper script (see step 1) in [Getting Started](#Getting%20Started)).

Specific requirements
- We use the git [subrepo](https://github.com/ingydotnet/git-subrepo) command to manage the original mysql code as a repository
- We use [docker](https://docs.docker.com/engine/install/ubuntu/) to package our code as a container. **Note: Make sure to perform the docker post-installation steps for Managing Docker as a non-root user!**

## Getting Started

1. Clone this repo with
```
git clone git@github.com:DataManagementLab/trustdble.git
```
2. Add the mysql-server source files to the server folder (https://github.com/mysql/mysql-server)
```
cd server
```
3. Install the requirements using the `trustdble` helper script (currently, we only support linux as dev environment):
```
./trustdble init -s linux
```
4. Build the repo with the help of the helper script. NOTE: This might take very long!
```
./trustdble build # does not build tests
```
5. Run tests
```
./trustdble build -u # build the tests as well
./trustdble test
```
6. Use the `trustdble` helper script to start a server/client.

## Building the docker image
You can use an all-in-one docker image to run the server. To build the docker image use the following commands:

**Important: Please make sure that you installed docker correctly and performed the required [post-installation steps to manage docker as non-root user](https://docs.docker.com/engine/install/linux-postinstall/#manage-docker-as-a-non-root-user). You might need to reboot your system for changes to take effect.**


1. Build the docker image of the server with the same ubuntu variant as your dev system (At the first time this may take some time).
```
./trustdble docker build
```
The Optional arguments are
```
-h, --help                 show this help message and exit
-v, --variant              ubuntu variant to use for build, default=the same variant as the dev system
-c, --clean                remove previously created docker containers (both ethereum and trustdble server)
-t, --tag                  used for naming the docker image, default="trustdble-server"
```

2. Clean all trustdble specific docker container.
```
./trustdble docker clean
```


## Run server with docker
**Note**: To run the server with docker you have to build the code in release mode, i.e., using `./trustdble build --release` (see the previous section)

**Note**: When using ETHEREUM it is necessary to mount the host’s docker socket into the container. This is done by adding the `-v /var/run/docker.sock:/var/run/docker.sock` flag to the docker run command (done inside the `trustdble start server` script).
Be aware that this gives the container running your trustble-server full access to your docker host, i.e. even containers that are not created for trustdble could be managed/manipulated.

1. Start the server by running it in a container by using the `-d` flag (use the same value for NUMBER as above), check if there is an existing server if not it will automatically create one:
```shell
./trustdble start server -n NUMBER -d

# -c|--clean flag to remove the old one and create a new one
./trustdble start server -n NUMBER -d -c
```
2. If you want to stop the server temporarily, use the `docker stop` command and specify the name of the docker container (use the same value for NUMBER as above):
```
docker stop trustdble-server-NUMBER
```
3. You can remove the server (folders and docker containers) by using the following command
```
./trustdble remove server -n NUMBER -d
```

## Run server without docker
1. Generate required folders for data and config, pass a value between 0-9 as NUMBER
```shell
./trustdble create server -n NUMBER

# Use the following if you build in release mode
BUILD_TYPE=release ./trustdble create server -n NUMBER
```
2. Start the server (use the same value for NUMBER as above, check if there is an existing server if not it will automatically create one):
```shell
./trustdble start server -n NUMBER

# -c|--clean flag to remove the old one and create a new one
./trustdble start server -n NUMBER -c

# Use the following if you build in release mode
BUILD_TYPE=release ./trustdble start server -n NUMBER
```
3. There are two options to stop a starter server instance:
   1) The previous `start server` command output the process id of the server instance, look for an output like `starting as process 58182`. Use the process id to kill the server, using: `kill 58182`
   2) Connect to a server instance using the mysql-client and send a `SHUTDOWN` command to the server.
4. You can remove the server by using the following command
```shell
./trustdble remove server -n NUMBER
```

## Connecting to the server with mysql-client
To connect to the server use the trustdble script start client command. **Note**: If you want to connect to a server running in a docker container you also have to specify the `-d` option and type in a password (pw="password"):
```
./trustdble start client -n NUMBER
```

## About the trustdble helper script

The `src` directory of this repository contains a script called `trustdble` to support in the development process. It can be easily executed with

    ./trustdble <command> <arguments>

The script `trustdble` is a wrapper script that makes all scripts from the directory `trustdble_scripts` available (excluding hidden files e.g. `.template.sh`). Each script in `trustdble_scripts` represents a command which is the exact file name without the file extension and underscores replaced by space. All flag parameters are passed on to the subscript.

As an example

    ./trustdble start server --number 1

is equivalent to

    ./trustdble_scripts/start_server.sh --number 1

### Get Help

The script `trustdble` contains two predefined commands.

* **help** shows a general help message
* **list** lists all available script commands included in `trustdble_scripts`

Every script command of `trustdble` has an own help message, which can be accessed by using the `-h` or `--help` flag.
For example to get help for starting a server, you can run

    ./trustdble start server --help

### How to add a Command

To add a new command script to the `trustdble` script, follow the instructions stated [here](trustdble_scripts/README.md)
