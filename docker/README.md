### Prerequisites
* Linux: Install docker and add your user to the docker group
* OSX: Install [http://boot2docker.io/](boot2docker) and make sure it is running

### Usage
* Clone the hyrise git repository to your local machine
* Per default, the folder ~/workspace of your host system will be mounted at /home/dev/workspace. To change that, copy docker.conf.sample to docker.conf and change the parameters
* Execute run.sh (again, make sure boot2docker/dockerd is started)
* You should automatically be connected to your container via SSH and you should be able to build and run hyrise within this container.
* You can use screen to start multiple processes (e.g. hyrise and the sql web frontend)

### Trouble shooting
* If you're having trouble with boot2docker and its environment variables try running reset_boot2docker.sh. In most cases this should resolve your problems