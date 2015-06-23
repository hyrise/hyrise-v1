Clone the hyrise git repository into ~/workspace/
run run.sh

the folders ~/workspace and ~/.ssh of your host system will be mounted at /home/dev/workspace and /home/dev/.ssh
This way, you can use your favorite text editor to change code on the host system while being able to complile hyrise within the container and use your ssh keys inside the container for pushing changes to git.

To start the container without the run.sh script (e.g. to change the default paths and ports) simply modify and execute the commands from the run.sh script.

Per default, the ports 2222, 8888, 5000 are forwarded to the container.

To connect to a running container, simply execute the ssh command from the run.sh file.
