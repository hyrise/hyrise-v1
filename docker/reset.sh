#!/bin/bash


DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

if [ -e "$DIR/docker.conf" ]
then
	source $DIR/docker.conf
else
	$container_name="hyriseDev"
fi;

if hash boot2docker 2>/dev/null; then
	boot2docker stop
	boot2docker delete
	boot2docker init
	boot2docker start
else
	docker kill $container_name
	docker rm $container_name
fi;