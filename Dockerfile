# Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

FROM ubuntu:18.04

LABEL authors="Estelle Maudet, Pierre Jackman, Noël Martin, Christophe Servan"

ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

ENV TZ=Europe/Paris
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get -y update && \
    apt-get -y install \
	curl \
 	vim
	

#RUN echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list
#RUN curl https://bazel.build/bazel-release.pub.gpg | apt-key add -


RUN apt-get -y update && \
    apt-get -y install \
        openjdk-11-jdk \
        bash-completion \
        libgrpc++-dev \
        libgrpc-dev \
        protobuf-compiler-grpc \
        golang \
        python3-numpy \
        python3-scipy \
        python3-pip \
	libtool \
        cmake \
        g++ \
        libboost-locale1.65-dev \
        libboost-regex1.65-dev \
        libyaml-cpp-dev \
        git \
        automake

#ADD https://github.com/bazelbuild/bazel/releases/download/0.11.0/bazel_0.11.0-linux-x86_64.deb /tmp/bazel_0.11.0-linux-x86_64.deb
#RUN dpkg -i /tmp/bazel_0.11.0-linux-x86_64.deb

# COPY vendor/bazel/bazel_0.11.0-linux-x86_64.deb /tmp 
# RUN dpkg -i /tmp/bazel_0.11.0-linux-x86_64.deb

# N cd vendor/bazel && dpkg -i bazel_0.11.0-linux-x86_64.deb

# RUN python -m pip install tensorflow-serving-api
 
ADD https://cmake.org/files/v3.9/cmake-3.9.0-Linux-x86_64.sh /tmp/cmake-3.9.0-Linux-x86_64.sh
RUN mkdir /opt/cmake
RUN sh /tmp/cmake-3.9.0-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
RUN ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
RUN cmake --version


#RUN git clone --recursive https://github.com/QwantResearch/qtranslate.git /opt/text-translate
COPY . /opt/text-translate

WORKDIR /opt/text-translate

RUN git checkout develop

RUN ln -s /usr/bin/python3 /usr/bin/python

#RUN bash ./tfinstall.sh

RUN bash ./install.sh

#RUN ./install.sh

#RUN groupadd -r qnlp && useradd --system -s /bin/bash -g qnlp qnlp

#USER qnlp 

#ENTRYPOINT ["/usr/local/bin/text-translate"]
