#!/bin/sh
cmd="mysql --user=$HYRISE_MYSQL_USER --password=$HYRISE_MYSQL_PASS --port=$HYRISE_MYSQL_PORT --host=$HYRISE_MYSQL_HOST"
$cmd -e "drop database if exists cbtr$1";
