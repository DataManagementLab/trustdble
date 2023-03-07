ARG VARIANT="latest"
# Pull base image
FROM ubuntu:${VARIANT}

# Define working directory
WORKDIR /trustdble

RUN mkdir -p /scripts

# Copy script
COPY install-dep.sh /scripts

# Install dependencies
RUN bash /scripts/install-dep.sh

# Copy mysql binaries
COPY resources/build-release /usr/local/mysql/

# Copy script
COPY setup-mysql.sh /scripts

# Add keys
COPY tdb-server-key.pem /trustdble/data/keys/tdb-server-key.pem
COPY tdb-server-key.pub /trustdble/data/keys/tdb-server-key.pub

# Setup mysql
RUN bash /scripts/setup-mysql.sh

# Copy script
COPY init-mysql.sh /scripts

ENV PATH="/usr/local/mysql/bin:${PATH}"

ENV MYSQL_UNIX_PORT /var/lib/mysql/mysql.sock

COPY my.cnf /trustdble/server_config/my.cnf

ENTRYPOINT ["/scripts/init-mysql.sh"]
CMD ["mysqld"]