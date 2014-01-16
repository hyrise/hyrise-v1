#!/bin/sh
cmd="mysql --user=$HYRISE_MYSQL_USER --password=$HYRISE_MYSQL_PASS --port=$HYRISE_MYSQL_PORT --host=$HYRISE_MYSQL_HOST"
$cmd -e "drop database if exists cbtr$1";
$cmd -e "create database cbtr$1 CHARACTER SET utf8 COLLATE utf8_bin;";
$cmd cbtr$1 -e "`cat test/sap_data/mysql_createtables.sql`";

mi="mysqlimport --user=$HYRISE_MYSQL_USER --password=$HYRISE_MYSQL_PASS --port=$HYRISE_MYSQL_PORT --host=$HYRISE_MYSQL_HOST -s --local cbtr$1"
$mi `pwd`/`dirname $0`/KNA1.txt
$mi `pwd`/`dirname $0`/VBAK.txt
$mi `pwd`/`dirname $0`/VBAP.txt

