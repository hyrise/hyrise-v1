FROM ubuntu:14.04
MAINTAINER Marvin Keller <marv@ramv.de>

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y \
	autoconf \
	binutils-dev \
	bison \
	build-essential \
	bzip2 \
	ccache \
	cmake \
	curl \
	doxygen \
	flex \
	gcc-multilib \
	gdb \
	gfortran \
	git \
	git-flow \
	htop \
	libboost-all-dev \
	libev-dev \
	libgoogle-perftools-dev \
	libhwloc-dev \
	liblog4cxx10 \
	liblog4cxx10-dev \
	libmetis-dev \
	libmysqlclient-dev \
	libpapi-dev \
	libtbb-dev \
	libtool \
	libunwind8-dev \
	man \
	mysql-server \
	nano \
	nodejs-legacy \
	npm \
	openssh-server \
	python \
	python-pip \
	screen \
	strace \
	sudo \
	tcpdump \
	telnet \
	vim \
	wget

RUN pip install sphinx virtualenv sphinxcontrib-seqdiag webcolors funcparserlib

# HTTP Server for Web front ends
RUN npm install http-server -g

ENV tmpDir /tmp/hyrise
RUN mkdir $tmpDir

WORKDIR $tmpDir

RUN wget http://downloads.sourceforge.net/project/libcsv/libcsv/libcsv-3.0.1/libcsv-3.0.1.tar.gz; \
	tar -xf libcsv-3.0.1.tar.gz; \
	cd $tmpDir/libcsv-3.0.1; \
	make install -j 4;

RUN cd $tmpdir; \
	git clone https://github.com/nanomsg/nanomsg.git; \
    cd nanomsg; \
    ./autogen.sh; \
    ./configure; \
    make; \
    make check; \
    make install; \
    rm -rf /tmp/nanomsg; \
    ldconfig;

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

# Use non-shared directory for persistency. (Virtualbox shared folder don't support fsync - Problem with boot2docker)
RUN echo "export HYRISE_PERSISTENCY_PATH=/home/dev/hyrise_persistency" >> /etc/profile

#Fix OSX locale problem
RUN locale-gen en_US en_US.UTF-8
RUN dpkg-reconfigure locales

ENTRYPOINT /usr/sbin/sshd -D

VOLUME ["/home/dev/workspace"]
VOLUME ["/home/dev/.ssh"]

EXPOSE 22
EXPOSE 5000
EXPOSE 8888
