###########################################################################
# MIT License
# 
# Copyright (c) 2018 Christophe SERVAN, Qwant Research, 
# email: christophe[dot]servan[at]qwantresearch[dot]com
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
###########################################################################
cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -fpermissive ")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
# set(LD_PRELOAD "libcurl.so.3")

set(TF_DIR ${PROJECT_SOURCE_DIR}/../tensorflow/)
include_directories(${TF_DIR} ${TF_DIR}/bazel-genfiles ${TF_DIR}/bazel-tensorflow/external/eigen_archive ${TF_DIR}/bazel-tensorflow/external/protobuf_archive/src ${TF_DIR}/bazel-tensorflow/external/nsync/public/ ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/../cpr/include /usr/local/include/)
link_directories(${TF_DIR}/bazel-bin/tensorflow ${PROJECT_SOURCE_DIR}/../cpr/build/lib/ /usr/local/lib/)

set(CMAKE_CXX_FLAGS "-O2 -std=c++11 -lpthread -fpermissive ")


set(API_EXECUTABLE api_classification)
set(API_SOURCE ${PROJECT_SOURCE_DIR}/src/api_classification.cpp ${PROJECT_SOURCE_DIR}/src/qtokenizer.cpp)
set(NLU_EXECUTABLE nlu_local)
set(NLU_SOURCE ${PROJECT_SOURCE_DIR}/src/local_nlu.cpp ${PROJECT_SOURCE_DIR}/src/qtokenizer.cpp ${PROJECT_SOURCE_DIR}/src/qnlu.cpp)
set(NLU_EXECUTABLE_API api_nlu_local)
set(NLU_SOURCE_API ${PROJECT_SOURCE_DIR}/src/api_local_nlu.cpp ${PROJECT_SOURCE_DIR}/src/qtokenizer.cpp ${PROJECT_SOURCE_DIR}/src/qnlu.cpp)
add_executable(${API_EXECUTABLE} ${API_SOURCE})
add_executable(${NLU_EXECUTABLE} ${NLU_SOURCE})
add_executable(${NLU_EXECUTABLE_API} ${NLU_SOURCE_API})
target_link_libraries(${API_EXECUTABLE} pistache pthread qnlp fasttext)
target_link_libraries(${NLU_EXECUTABLE} pistache pthread qnlp fasttext tensorflow_framework tensorflow_qnlp)
target_link_libraries(${NLU_EXECUTABLE_API} pistache pthread qnlp fasttext tensorflow_framework tensorflow_qnlp cpr mongoose gtest gtest_main test_server curl)
