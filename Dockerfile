# This is a comment
FROM littlemole/devenv_gpp_make
MAINTAINER me <little.mole@oha7.org>

ARG CXX=g++
ENV CXX=${CXX}

ARG BACKEND=libevent
ENV BACKEND=${BACKEND}

ARG BUILDCHAIN=make
ENV BUILDCHAIN=${BUILDCHAIN}

RUN /usr/local/bin/install.sh repro 
RUN /usr/local/bin/install.sh prio

# build an install this project
RUN mkdir -p /usr/local/src/repro-curl
ADD . /usr/local/src/repro-curl

RUN SKIPTESTS=true /usr/local/bin/build.sh repro-curl 
