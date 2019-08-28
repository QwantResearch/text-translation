# Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

FROM ubuntu:18.04

LABEL authors="Estelle Maudet, Pierre Jackman, Noël Martin, Christophe Servan"

ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

ENV TZ=Europe/Paris
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone


RUN apt-get -y update && \
    apt-get -y install \
        bash-completion \
	libtool \
        cmake \
        g++ \
        libboost-locale1.65-dev \
        libboost-regex1.65-dev \
	libboost-thread1.65-dev \
	libboost-system1.65-dev \
        libyaml-cpp-dev \
        git \
        automake \
	curl \
	vim

#ADD https://github.com/bazelbuild/bazel/releases/download/0.11.0/bazel_0.11.0-linux-x86_64.deb /tmp/bazel_0.11.0-linux-x86_64.deb
#RUN dpkg -i /tmp/bazel_0.11.0-linux-x86_64.deb

# COPY vendor/bazel/bazel_0.11.0-linux-x86_64.deb /tmp 
# RUN dpkg -i /tmp/bazel_0.11.0-linux-x86_64.deb

# N cd vendor/bazel && dpkg -i bazel_0.11.0-linux-x86_64.deb

# RUN python -m pip install tensorflow-serving-api
 
#ADD https://cmake.org/files/v3.9/cmake-3.9.0-Linux-x86_64.sh /tmp/cmake-3.9.0-Linux-x86_64.sh
#RUN mkdir /opt/cmake
#RUN sh /tmp/cmake-3.9.0-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
#RUN ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
#RUN cmake --version


RUN git clone --recursive https://github.com/QwantResearch/qtranslate.git /opt/text-translation 

WORKDIR /opt/text-translation

COPY resources /opt/text-translation/resources

RUN git checkout marian
RUN git checkout marian

RUN bash ./install.sh

RUN groupadd -r qnlp && useradd --system -s /bin/bash -g qnlp qnlp

USER qnlp 

ENTRYPOINT ["/usr/local/bin/text-translation"]
