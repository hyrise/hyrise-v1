#!/bin/bash
boot2docker stop
boot2docker delete
boot2docker init
boot2docker start
eval "$(boot2docker shellinit)"