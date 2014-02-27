##########################
Starting the Hyrise Server
##########################

Before starting the server, you have to configure Hyrise and tell it where to find the table data directory. This directory has to exist already. In this Hyrise version most tables are in the /test directory of the project ::

      cd hyrise
      export HYRISE_DB_PATH=`pwd`/test

Export the library directory with the 'export' command ::

      export LD_LIBRARY_PATH=./build/:$LD_LIBRARY_PATH

Finally you can start the server :: 

      ./build/hyrise_server -l ./build/log.properties -p 5000