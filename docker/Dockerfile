FROM ubuntu:14.04
MAINTAINER Marvin Keller <marv@ramv.de>

RUN apt-get update && apt-get install -y build-essential git openssh-server htop screen sudo man doxygen bzip2 vim nano git-flow wget curl telnet

#For C/C++
RUN apt-get install -y cmake ccache binutils-dev

#For Python
RUN apt-get install -y python python-pip

# For debugging
RUN apt-get install -y gdb strace tcpdump

# For 32-bit
RUN apt-get -y install gcc-multilib

#Hyrise dependencies
RUN apt-get -y install liblog4cxx10 liblog4cxx10-dev libmysqlclient-dev libunwind8-dev libev-dev libtbb-dev libboost-all-dev libhwloc-dev libgoogle-perftools-dev gfortran libmetis-dev libpapi-dev nodejs-legacy npm bison flex

RUN pip install sphinx virtualenv sphinxcontrib-seqdiag webcolors funcparserlib

# HTTP Server for Web front ends
RUN npm install http-server -g


ENV tmpDir /tmp/hyrise
RUN mkdir $tmpDir

WORKDIR $tmpDir
RUN wget http://downloads.sourceforge.net/project/libcsv/libcsv/libcsv-3.0.1/libcsv-3.0.1.tar.gz
RUN tar -xf libcsv-3.0.1.tar.gz

WORKDIR $tmpDir/libcsv-3.0.1
RUN make install -j 4

WORKDIR /tmp
RUN rm -rf $tmpDir

RUN mkdir /var/run/sshd
RUN /usr/bin/ssh-keygen -A

# Set up my user
RUN useradd dev -u 1000 -s /bin/bash -m

USER dev

#scm_breeze
RUN git clone git://github.com/ndbroadbent/scm_breeze.git ~/.scm_breeze
RUN ~/.scm_breeze/install.sh

USER root

# Remove default motd
RUN rm /etc/update-motd.d/*
RUN rm /var/run/motd.dynamic
ADD motd /etc/motd
RUN sed -i '/motd.dynamic/d' /etc/pam.d/sshd
RUN sed -i '/motd.dynamic/d' /etc/pam.d/login
RUN rm /etc/legal

RUN gpasswd -a dev sudo
RUN echo 'dev:dev123' | chpasswd

ENTRYPOINT /usr/sbin/sshd -D

VOLUME ["/home/dev/workspace"]
VOLUME ["/home/dev/.ssh"]

EXPOSE 22
EXPOSE 5000
EXPOSE 8888