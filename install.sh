#!/usr/bin/env bash
# Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
# license. See LICENSE in the project root.


export PREFIX=/usr/local/

echo "Prefix set to $PREFIX"

export CMAKE_PREFIX_PATH=$PREFIX

set -eou pipefail

# git submodule update --init --recursive

echo "Installing dependencies"

pushd vendor/qnlp-toolkit
	rm -rf build
	bash install.sh $PREFIX
popd
 
for dep in pistache json sentencepiece easywsclient
do
pushd vendor/$dep
	rm -rf build
	mkdir -p build
	pushd build
		cmake .. -DCMAKE_INSTALL_PREFIX="${PREFIX}" -DCMAKE_BUILD_TYPE=Release
		make -j && make install
	popd
popd
done


echo "Installing text-translate"
mkdir -p $PREFIX
rm -rf build
mkdir -p build
pushd build
	cmake -DCMAKE_INSTALL_PREFIX="${PREFIX}" -DCMAKE_BUILD_TYPE=Release  Protobuf_PROTOC_EXECUTABLE=/usr/local/bin/protoc ..
	make -j && make install
popd
