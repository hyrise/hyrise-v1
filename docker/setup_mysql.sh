#!/bin/bash

echo dev123 | sudo -S mysqld > /dev/null 2>&1 &
sleep 2
mysql -uroot -e "CREATE USER 'vagrant'@'%' IDENTIFIED BY 'vagrant'"
mysql -uroot -e "GRANT ALL PRIVILEGES ON *.* TO 'vagrant'@'%' WITH GRANT OPTION"

