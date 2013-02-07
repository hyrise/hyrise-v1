#!/bin/sh
cmd="mysql --user=$HYRISE_MYSQL_USER --password=$HYRISE_MYSQL_PASS --port=$HYRISE_MYSQL_PORT --host=$HYRISE_MYSQL_HOST"
$cmd < `dirname $0`/mysql_createtables.sql;
$cmd cbtr -e "LOAD DATA LOCAL INFILE '`dirname $0`/kna1.txt' INTO TABLE KNA1
CHARACTER SET utf8
FIELDS TERMINATED BY '\t'";
$cmd -uroot -proot cbtr -e "LOAD DATA LOCAL INFILE '`dirname $0`/vbap_base.txt' INTO TABLE VBAP
CHARACTER SET utf8
FIELDS TERMINATED BY '\t'";
$cmd -uroot -proot cbtr -e "LOAD DATA LOCAL INFILE '`dirname $0`/vbak_base.txt' INTO TABLE VBAK
CHARACTER SET utf8
FIELDS TERMINATED BY '\t'";
