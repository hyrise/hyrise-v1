### Prerequisites
* Linux: Install docker and add your user to the docker group
* OSX: Install [boot2docker](http://boot2docker.io/). You can also install it using homebrew by running 'brew install boot2docker'.

### Usage
* Clone the hyrise git repository to your local machine
* Run 'git submodule update --init' within your repository. This can be done either on your host machine or within the docker container.
* Per default, the folder ~/workspace of your host system will be mounted at /home/dev/workspace. To change that, copy docker.conf.sample to docker.conf and change the parameters
* Execute run.sh (again, make sure boot2docker/dockerd is started)
* You should automatically be connected to your container via SSH and you should be able to build and run hyrise within this container.
* You can use screen to start multiple processes (e.g. hyrise and the sql web frontend). You can also open another SSH session by running run.sh again.
* If you need to reset the container, simply run reset.sh

### MySQL
* To setup MySQL with the credentials from the default config (vagrant:vagrant), execute the setup_mysql.sh script (The script will start and configure mysqld)
* If you just want to start the the mysql server without configuration, simply run 'sudo mysqld &'. You can connect to mysqld via 'mysql -uroot'

### Troubleshooting
* If you're having trouble with boot2docker, try running reset.sh. In most cases this should resolve your problems
