#!/bin/sh
cmd="mysql --user=$HYRISE_MYSQL_USER --password=$HYRISE_MYSQL_PASS --port=$HYRISE_MYSQL_PORT --host=$HYRISE_MYSQL_HOST"
$cmd -e "drop database if exists cbtr$1";
$cmd -e "create database cbtr$1 CHARACTER SET utf8 COLLATE utf8_bin;";
$cmd cbtr$1 -e "`cat test/sap_data/mysql_createtables.sql`";
$cmd cbtr$1 -e "LOAD DATA INFILE '`pwd`/`dirname $0`/kna1.txt' INTO TABLE KNA1
CHARACTER SET utf8
FIELDS TERMINATED BY '\t'";
$cmd cbtr$1 -e "LOAD DATA INFILE '`pwd`/`dirname $0`/vbap_base.txt' INTO TABLE VBAP
CHARACTER SET utf8
FIELDS TERMINATED BY '\t'";
$cmd cbtr$1 -e "LOAD DATA INFILE '`pwd`/`dirname $0`/vbak_base.txt' INTO TABLE VBAK
CHARACTER SET utf8
FIELDS TERMINATED BY '\t'";
