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

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -fpermissive -L/usr/local/lib")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
# set(LD_PRELOAD "libcurl.so.3")


set(TF_DIR ${PROJECT_SOURCE_DIR}/../tensorflow/)
set(TF_SERVING_DIR ${PROJECT_SOURCE_DIR}/../serving/)
include_directories(${TF_DIR} ${TF_DIR}/bazel-genfiles ${TF_DIR}/bazel-tensorflow/external/eigen_archive ${TF_DIR}/bazel-tensorflow/external/protobuf_archive/src ${TF_DIR}/bazel-tensorflow/external/nsync/public/ ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/../cpr/include /usr/local/include/ ${TF_SERVING_DIR} ${TF_SERVING_DIR}/bazel-genfiles /usr/local/include)
link_directories(${TF_DIR}/bazel-bin/tensorflow ${PROJECT_SOURCE_DIR}/../cpr/build/lib /usr/local/lib ${TF_SERVING_DIR}/bazel-bin/tensorflow_serving/model_servers/ ${TF_SERVING_DIR}/bazel-bin/tensorflow_serving/apis/ ${TF_SERVING_DIR}/bazel-bin/tensorflow_serving/util/ /usr/lib /usr/local/lib)

set(CMAKE_CXX_FLAGS "-O2 -std=c++11 -lpthread -fpermissive ")

# set(NMT_EXECUTABLE_API api_nmt_local)
# set(NMT_SOURCE_API 
#   ${PROJECT_SOURCE_DIR}/src/api_local_nmt.cpp 
#   ${PROJECT_SOURCE_DIR}/src/qtokenizer.cpp 
#   ${PROJECT_SOURCE_DIR}/src/qnmt.cpp 
#   ${PROJECT_SOURCE_DIR}/src/bpe.cpp 
#   ${PROJECT_SOURCE_DIR}/src/utils.cpp
#   )


set(NMT_REMOTE_API api_nmt_remote)
set(NMT_SOURCE_REMOTE_API 
  ${PROJECT_SOURCE_DIR}/src/api_remote_nmt.cpp 
  ${PROJECT_SOURCE_DIR}/src/qtokenizer.cpp 
  ${PROJECT_SOURCE_DIR}/src/qnmt.cpp 
  ${PROJECT_SOURCE_DIR}/src/bpe.cpp 
  ${PROJECT_SOURCE_DIR}/src/utils.cpp
  )
# add_executable(${NMT_EXECUTABLE_API} ${NMT_SOURCE_API})
add_executable(${NMT_REMOTE_API} ${NMT_SOURCE_REMOTE_API})


set(TEST_CLIENT client_moses)
set(TEST_CLIENT_SRC 
  ${PROJECT_SOURCE_DIR}/src/client_moses.cpp 
  )
# add_executable(${NMT_EXECUTABLE_API} ${NMT_SOURCE_API})
add_executable(${TEST_CLIENT} ${TEST_CLIENT_SRC})

# target_link_libraries(${NMT_EXECUTABLE_API} pistache pthread qnlp fasttext tensorflow_framework tensorflow_qnlp http_rest_prediction_handler predict_proto model_proto optional grpc++ prediction_service_proto classification_proto get_model_metadata_proto regression_proto input_proto inference_proto boost_regex)
target_link_libraries(${NMT_REMOTE_API} pistache pthread qnlp fasttext tensorflow_framework tensorflow_qnlp http_rest_prediction_handler predict_proto model_proto optional grpc++ prediction_service_proto classification_proto get_model_metadata_proto regression_proto input_proto inference_proto boost_regex)

target_link_libraries(${TEST_CLIENT} xmlrpc++ xmlrpc_client++ xmlrpc_util++)

