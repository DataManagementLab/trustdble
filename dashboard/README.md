# TrustDBle Dashboard
This repo contains a Node.js backend webserver providing an API to execute SQL-Statements on the trustDBle server (database server) and a frontend using Vue.js. Both components (backend & frontend) together are called trustdble-dashboard or short dashboard.

You can either start the dashboard manually or by building a docker image & running the image within a docker container.

## Prerequisites
The following packages needs to be installed on your computer for the dashboard: nodejs, npm
```
sudo apt-get update
sudo apt-get install nodejs
sudo apt-get install npm
```
Version of npm should be 7.x or latest and version of nodejs should be 12.x or latest
<br>
To start the dashboard you also need a running trustdble-server instance. In this explanation we assume that the trustdble-server instance was started with docker-compose and is running in a docker container on 3305

## Option 1. Start dashboard manually
1. Adjust the values in the app.properties file
   ```
    app.port=8081

    mysql.host=127.0.0.1
    mysql.port=3305
    mysql.user=root
    mysql.password=

    inviteFiles.path=${/path to mysql-server invite file directory}
   ```
2. Open a terminal at root directory 
3. Make sure the version of npm >= 7 and the version of node >= 12
   ```
   npm -v
   node -v
   ```
4. Install all node packages/dependencies for backend, frontend and blockchain-monitor
   ```
   npm install
   ```
5. Run backend, frontend and blockchain-monitor simultaneously with single command
   ```
   npm run dev
   ```
6. Run backend, frontend and Blockchain-monitor separately
   ```
   npm run backend
   npm run frontend
   npm run blockchain-monitor
   ```
   We are using lerna with npm for MonoRepo and lerna command are using in ```./package.json```. For further details and commands of lerna are on [lerna github](https://github.com/lerna/lerna).

## Option 2. Start dashboard via Docker

To start the dashboard with docker, you first need to build the docker image and then run it within a docker container.

1. In app.properties, adjust mysql.host & mysql.port property
   ```
   app.port=8081

   mysql.host = 172.20.1.1
   mysql.port = 3305
   mysql.user=root
   mysql.password=password

   inviteFiles.path=/trustdble/data/invite
   ```
   The mysql.host for docker should be same as the IP of docker0. IP of docker0 can be found by this command: ```ip addr```.
2. Build docker image
   ```
   docker build -t trustdble-dashboard .
   ```
3. Run docker image
   ```
   docker run -it -p 8081:8081 -v <YOUR_PATH>/trustdble-dashboard/configs:/trustdble-dashboard/configs/ --name dashboard trustdble-dashboard
   ```

   If you get some permission denied errors, in error details you can find which host denied "Host '172.17.0.2' is not allowed to connect"  then you can start a mysql-client that is connected to the trustdble-server and use the following commands and replace host which you get from error to allow your docker-container the access to the mysql-server
   ```
   CREATE USER 'root'@'172.17.0.2' IDENTIFIED BY 'password';
   GRANT ALL ON *.* TO 'root'@'172.17.0.2' WITH GRANT OPTION;
   GRANT PROXY ON ''@'' TO 'root'@'172.17.0.2' WITH GRANT OPTION;
   ```
## How to test
There are two types of tests:
1. Test backend API endpoints with mock database
2. TRUSTDBLE integration tests for shared databases

There are three types of shared database, So TRUSTDBLE integration tests can be run separately for each database:
1. STUB
2. ETHEREUM
3. FABRIC

### Requirements for TRUSTDBLE integration tests
1. make sure All node dependencies are installed ```npm install```
2. Make sure two TRUSTDBLE servers 0 and 1 are started, on default ports 3305 and 3306. Ports can be changed from scripts at /trustdble-dashboard/backend/package.json 
3. Copy public key of TRUSTDBLE server 1 
```
cp <path-to-trustdble-server>/src/mysql_home1/keys/tdb-server-key.pub <path-to-trustdble-dashboard>/backend/tests/
```
If you want to run all tests together:
```
npm run test
```

If you want to run individually:
* Change directory to Backend 
```
cd backend/
```
* Test only API endpoints of backend APP 
```
npm run test:app
```
* Test TRUSTDBLE integration for STUB database 
```
npm run test:trustdble:stub
```
* Test TRUSTDBLE integration for ETHEREUM database 
```
npm run test:trustdble:ethereum
```
* Test TRUSTDBLE integration for FABRIC database
```
npm run test:trustdble:fabric
```

Before run above command, make sure All node dependencies are install.
## Further information

### Backend
The backend uses following modules:
* Express for Webserver
* MySQL2 for Database connection

### Frontend
The frontend is build with this template: https://github.com/vikdiesel/admin-one-vue-bulma-dashboard
