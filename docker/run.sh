#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

if [ -e "$DIR/docker.conf" ]
then
	source $DIR/docker.conf
else
	cp $DIR/docker.conf.sample $DIR/docker.conf
	source $DIR/docker.conf
fi;

eval workspace_dir=$workspace_dir

if hash boot2docker 2>/dev/null; then
	eval "$(boot2docker shellinit)"
fi;

# Add ssh key if it doesnt exist already
KEY=$(cat ~/.ssh/id_rsa.pub)
grep -q -F "$KEY" ~/.ssh/authorized_keys || echo $KEY >> ~/.ssh/authorized_keys


RUNNING=$(docker inspect --format="{{ .State.Running }}" $container_name 2> /dev/null)

if [ $? -eq 1 ]; then
  echo "Container doesn't exist - Starting..."
  docker run -d -p $ssh_port:22 -p $hyrise_port:5000 -p $http_port:8888 --name $container_name -h $container_name -v $workspace_dir:/home/dev/workspace -v ~/.ssh:/home/dev/.ssh hyrise/hyrisedev:latest
  sleep 3
elif [ "$RUNNING" == "false" ]; then
  echo "Container is not running - Starting..."
  docker rm $container_name
  docker run -d -p $ssh_port:22 -p $hyrise_port:5000 -p $http_port:8888 --name $container_name -h $container_name -v $workspace_dir:/home/dev/workspace -v ~/.ssh:/home/dev/.ssh hyrise/hyrisedev:latest
  sleep 3
fi;


if hash boot2docker 2>/dev/null; then
    ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no dev@`boot2docker ip` -p$ssh_port
else
    ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no dev@localhost -p$ssh_port
fi

