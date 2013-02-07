#!/bin/bash

sudo apt-get update
sudo apt-get dist-upgrade

./hyrisePackageDeps.sh
./hyriseExternalDeps.sh

