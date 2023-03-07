#!/bin/bash

# Setup mysql
groupadd mysql
useradd -r -g mysql -s /bin/false mysql
mkdir -p /usr/local/mysql
cd /usr/local/mysql

mkdir /var/lib/mysql
mkdir /var/lib/mysql-files
chown -R mysql /var/lib/mysql
chown -R mysql /var/lib/mysql-files
mkdir mysql-files
chmod 750 mysql-files
chown -R mysql .
chgrp -R mysql .

# Copy libs to /lib/x86_64-linux-gnu/
cp library_output_directory/libprotobuf-lite.so.3.11.4 /lib/x86_64-linux-gnu/
cp _deps/trustdble-adapters-build/fabric/adapter/src/../extern/go_client/libclient.so /lib/x86_64-linux-gnu/

# Create trustdble folders in volume
cd /trustdble
mkdir -p data/configs
chown -R mysql .
chgrp -R mysql .

mkdir -p /stub_data
chown -R mysql /stub_data
chgrp -R mysql /stub_data

chmod 664 data/keys/tdb-server-key.pem