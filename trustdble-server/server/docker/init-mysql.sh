#!/bin/bash
# Copyright (c) 2017, 2021, Oracle and/or its affiliates.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
set -e

# We faced a permission issue when trying to access the docker deamon socket inside the host
# The root cause was a different GID of the docker group in the container and host
# See this github issue for details https://github.com/jenkinsci/docker/issues/263
# In order to fix the issue we have to make sure that the docker group on both host and container
# has the same GID, the below code is taken from (MIT license): https://github.com/sudo-bmitch/jenkins-docker/blob/main/entrypoint.sh

# the following has to be done in the entry point (and not while building the image)
# since only when the container is created we have access to the socket

# check the GID of the docker group on the host
SOCK_DOCKER_GID=$(ls -ng /var/run/docker.sock | cut -f3 -d' ')

# get group id of docker inside container
CUR_DOCKER_GID=$(getent group docker | cut -f3 -d: || true)

# if they don't match, adjust GIDs
if [ ! -z "$SOCK_DOCKER_GID" -a "$SOCK_DOCKER_GID" != "$CUR_DOCKER_GID" ]; then
	groupmod -g ${SOCK_DOCKER_GID} -o docker
fi

# add the mysql user to the docker group to be able to execute docker commands
usermod -aG docker mysql

# When we run a docker command as mysql user we get the error message that the /root/.docker/config.json file doesn't exist so we are creating this file.
# We do this change in this file because we have to know the new GID of the docker group that is set above
mkdir -p /root/.docker
touch /root/.docker/config.json
chgrp -R docker /root
chmod g+x /root # for some reason docker needs x access when accessing the file


# Create configuration.ini for server
TRUSTDBLE_SERVER_CONFIG_FILE=/trustdble/data/configs/configuration.ini
# write adapter and manager configuration to file
tee ${TRUSTDBLE_SERVER_CONFIG_FILE} <<EOF
[Adapter]
# ETHEREUM
#The connection-url value is only used if we dont use a network-config when initializing the adapter
connection-url=http://127.0.0.1:8000
max-waiting-time=300
script-path=/usr/local/mysql/_deps/trustdble-adapters-src/ethereum/scripts
contract-path=/usr/local/mysql/_deps/trustdble-adapters-src/ethereum/contract/truffle/build/contracts/SimpleStorage.json

# STUB
data-path=/stub/
blocksize=10

[Manager]
# ETHEREUM
#The ports rpc-port and peer-port are the starting points for the manager to find free ports on the host
rpc-port=8000
peer-port=30303
join-ip=${ETH_JOIN_IP}
start-script-path=/usr/local/mysql/_deps/blockchain-manager-src/ethereum/scripts/start-network.sh
docker-file-path=/usr/local/mysql/_deps/blockchain-manager-src/ethereum/scripts/ethereum-docker

EOF

# change owner to mysql
chown mysql ${TRUSTDBLE_SERVER_CONFIG_FILE}
chgrp mysql ${TRUSTDBLE_SERVER_CONFIG_FILE}

# the below code is taken from the offical mysql docker image

echo "[Entrypoint] MySQL Docker Image 8.0.25-1.2.3-server"
# Fetch value from server config
# We use mysqld --verbose --help instead of my_print_defaults because the
# latter only show values present in config files, and not server defaults
_get_config() {
	local conf="$1"; shift
	"$@" --verbose --help 2>/dev/null | grep "^$conf" | awk '$1 == "'"$conf"'" { print $2; exit }'
}

# Generate a random password
_mkpw() {
	letter=$(cat /dev/urandom| tr -dc a-zA-Z | dd bs=1 count=16 2> /dev/null )
	number=$(cat /dev/urandom| tr -dc 0-9 | dd bs=1 count=8 2> /dev/null)
	special=$(cat /dev/urandom| tr -dc '=+@#%^&*_.,;:?/' | dd bs=1 count=8 2> /dev/null)

	echo $letter$number$special | fold -w 1 | shuf | tr -d '\n'
}

# If command starts with an option, prepend mysqld
# This allows users to add command-line options without
# needing to specify the "mysqld" command
if [ "${1:0:1}" = '-' ]; then
	set -- mysqld "$@"
fi

# Check if entrypoint (and the container) is running as root
if [ $(id -u) = "0" ]; then
	is_root=1
	install_devnull="install /dev/null -m0600 -omysql -gmysql"
	MYSQLD_USER=mysql
else
	install_devnull="install /dev/null -m0600"
	MYSQLD_USER=$(id -u)
fi

if [ "$1" = 'mysqld' ]; then
	# Test that the server can start. We redirect stdout to /dev/null so
	# only the error messages are left.
	result=0
	output=$("$@" --validate-config) || result=$?
	if [ ! "$result" = "0" ]; then
		echo >&2 '[Entrypoint] ERROR: Unable to start MySQL. Please check your configuration.'
		echo >&2 "[Entrypoint] $output"
		exit 1
	fi

	# Get config
	DATADIR="$(_get_config 'datadir' "$@")"
	SOCKET="$(_get_config 'socket' "$@")"

	if [ ! -d "$DATADIR/mysql" ]; then
		# If the password variable is a filename we use the contents of the file. We
		# read this first to make sure that a proper error is generated for empty files.
		if [ -f "$MYSQL_ROOT_PASSWORD" ]; then
			MYSQL_ROOT_PASSWORD="$(cat $MYSQL_ROOT_PASSWORD)"
			if [ -z "$MYSQL_ROOT_PASSWORD" ]; then
				echo >&2 '[Entrypoint] Empty MYSQL_ROOT_PASSWORD file specified.'
				exit 1
			fi
		fi
		if [ -z "$MYSQL_ROOT_PASSWORD" -a -z "$MYSQL_ALLOW_EMPTY_PASSWORD" -a -z "$MYSQL_RANDOM_ROOT_PASSWORD" ]; then
			echo >&2 '[Entrypoint] No password option specified for new database.'
			echo >&2 '[Entrypoint]   A random onetime password will be generated.'
			MYSQL_RANDOM_ROOT_PASSWORD=true
			MYSQL_ONETIME_PASSWORD=true
		fi
		if [ ! -d "$DATADIR" ]; then
			mkdir -p "$DATADIR"
			chown mysql:mysql "$DATADIR"
		fi

		echo '[Entrypoint] Initializing database'
		"$@" --user=$MYSQLD_USER --initialize-insecure

		echo '[Entrypoint] Database initialized'
		"$@" --user=$MYSQLD_USER --daemonize --skip-networking --socket="$SOCKET"

		# To avoid using password on commandline, put it in a temporary file.
		# The file is only populated when and if the root password is set.
		PASSFILE=$(mktemp -u /var/lib/mysql-files/XXXXXXXXXX)
		$install_devnull "$PASSFILE"
		# Define the client command used throughout the script
		# "SET @@SESSION.SQL_LOG_BIN=0;" is required for products like group replication to work properly
		mysql=( mysql --defaults-extra-file="$PASSFILE" --protocol=socket -uroot -hlocalhost --socket="$SOCKET" --init-command="SET @@SESSION.SQL_LOG_BIN=0;")

		for i in {30..0}; do
			if mysqladmin --socket="$SOCKET" ping &>/dev/null; then
				break
			fi
			echo '[Entrypoint] Waiting for server...'
			sleep 1
		done
		if [ "$i" = 0 ]; then
			echo >&2 '[Entrypoint] Timeout during MySQL init.'
			exit 1
		fi

		mysql_tzinfo_to_sql /usr/share/zoneinfo | "${mysql[@]}" mysql

		if [ ! -z "$MYSQL_RANDOM_ROOT_PASSWORD" ]; then
			MYSQL_ROOT_PASSWORD="$(_mkpw)"
			echo "[Entrypoint] GENERATED ROOT PASSWORD: $MYSQL_ROOT_PASSWORD"
		fi
		if [ -z "$MYSQL_ROOT_HOST" ]; then
			ROOTCREATE="ALTER USER 'root'@'localhost' IDENTIFIED BY '${MYSQL_ROOT_PASSWORD}';"
		else
			ROOTCREATE="ALTER USER 'root'@'localhost' IDENTIFIED BY '${MYSQL_ROOT_PASSWORD}'; \
			CREATE USER 'root'@'${MYSQL_ROOT_HOST}' IDENTIFIED BY '${MYSQL_ROOT_PASSWORD}'; \
			GRANT ALL ON *.* TO 'root'@'${MYSQL_ROOT_HOST}' WITH GRANT OPTION ; \
			GRANT PROXY ON ''@'' TO 'root'@'${MYSQL_ROOT_HOST}' WITH GRANT OPTION ;"
		fi
		"${mysql[@]}" <<-EOSQL
			DELETE FROM mysql.user WHERE user NOT IN ('mysql.infoschema', 'mysql.session', 'mysql.sys', 'root') OR host NOT IN ('localhost');
			CREATE USER 'healthchecker'@'localhost' IDENTIFIED BY 'healthcheckpass';
			${ROOTCREATE}
			FLUSH PRIVILEGES ;

			SOURCE /usr/local/mysql/plugin/rewrite_trustdble/scripts/setup_trustdble.sql;
		EOSQL
		if [ ! -z "$MYSQL_ROOT_PASSWORD" ]; then
			# Put the password into the temporary config file
			cat >"$PASSFILE" <<EOF
[client]
password="${MYSQL_ROOT_PASSWORD}"
EOF
			#mysql+=( -p"${MYSQL_ROOT_PASSWORD}" )
		fi

		if [ "$MYSQL_DATABASE" ]; then
			echo "CREATE DATABASE IF NOT EXISTS \`$MYSQL_DATABASE\` ;" | "${mysql[@]}"
			mysql+=( "$MYSQL_DATABASE" )
		fi

		if [ "$MYSQL_USER" -a "$MYSQL_PASSWORD" ]; then
			echo "CREATE USER '"$MYSQL_USER"'@'%' IDENTIFIED BY '"$MYSQL_PASSWORD"' ;" | "${mysql[@]}"

			if [ "$MYSQL_DATABASE" ]; then
				echo "GRANT ALL ON \`"$MYSQL_DATABASE"\`.* TO '"$MYSQL_USER"'@'%' ;" | "${mysql[@]}"
			fi

		elif [ "$MYSQL_USER" -a ! "$MYSQL_PASSWORD" -o ! "$MYSQL_USER" -a "$MYSQL_PASSWORD" ]; then
			echo '[Entrypoint] Not creating mysql user. MYSQL_USER and MYSQL_PASSWORD must be specified to create a mysql user.'
		fi
		echo
		for f in /docker-entrypoint-initdb.d/*; do
			case "$f" in
				*.sh)  echo "[Entrypoint] running $f"; . "$f" ;;
				*.sql) echo "[Entrypoint] running $f"; "${mysql[@]}" < "$f" && echo ;;
				*)     echo "[Entrypoint] ignoring $f" ;;
			esac
			echo
		done

		# When using a local socket, mysqladmin shutdown will only complete when the server is actually down
		mysqladmin --defaults-extra-file="$PASSFILE" shutdown -uroot --socket="$SOCKET"
		rm -f "$PASSFILE"
		unset PASSFILE
		echo "[Entrypoint] Server shut down"

		# This needs to be done outside the normal init, since mysqladmin shutdown will not work after
		if [ ! -z "$MYSQL_ONETIME_PASSWORD" ]; then
			echo "[Entrypoint] Setting root user as expired. Password will need to be changed before database can be used."
			SQL=$(mktemp -u /var/lib/mysql-files/XXXXXXXXXX)
			$install_devnull "$SQL"
			if [ ! -z "$MYSQL_ROOT_HOST" ]; then
				cat << EOF > "$SQL"
ALTER USER 'root'@'${MYSQL_ROOT_HOST}' PASSWORD EXPIRE;
ALTER USER 'root'@'localhost' PASSWORD EXPIRE;
EOF
			else
				cat << EOF > "$SQL"
ALTER USER 'root'@'localhost' PASSWORD EXPIRE;
EOF
			fi
			set -- "$@" --init-file="$SQL"
			unset SQL
		fi

		echo
		echo '[Entrypoint] MySQL init process done. Ready for start up.'
		echo
	fi

	# Used by healthcheck to make sure it doesn't mistakenly report container
	# healthy during startup
	# Put the password into the temporary config file
	touch /var/lib/mysql-files/healthcheck.cnf
	cat >"/var/lib/mysql-files/healthcheck.cnf" <<EOF
[client]
user=healthchecker
socket=${SOCKET}
password=healthcheckpass
EOF
	touch /var/lib/mysql-files/mysql-init-complete

	if [ -n "$MYSQL_INITIALIZE_ONLY" ]; then
		echo "[Entrypoint] MYSQL_INITIALIZE_ONLY is set, exiting without starting MySQL..."
		exit 0
	else
		echo "[Entrypoint] Starting MySQL 8.0.25-1.2.3-server"
	fi
	# 4th value of /proc/$pid/stat is the ppid, same as getppid()
	export MYSQLD_PARENT_PID=$(cat /proc/$$/stat|cut -d\  -f4)
	# Set mysql_home to trustdble config folder
	export MYSQL_HOME=/trustdble/server_config
	exec "$@" --user=$MYSQLD_USER
else
	exec "$@"
fi

# #!/bin/bash

# echo '[Entrypoint] Initializing trustdble'

# # start server
# mysqld --user=mysql > /dev/null 2>&1 &

# # create trustdble user
# mysql -u root -h localhost << EOF
# CREATE USER 'trustdble'@'172.17.0.1' IDENTIFIED BY 'password';
# GRANT ALL ON *.* TO 'trustdble'@'172.17.0.1';
# SHUTDOWN;
# EXIT;
# EOF