# Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -fpermissive -L/usr/local/lib")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
# set(LD_PRELOAD "libcurl.so.3")


include_directories(/usr/include /usr/local/include include)

link_directories(/usr/lib /usr/local/lib)

set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -O2 -std=c++11 -lpthread -fpermissive ")

# set(NMT_EXECUTABLE_API api_nmt_local)
# set(NMT_SOURCE_API 
#   ${PROJECT_SOURCE_DIR}/src/api_local_nmt.cpp 
#   ${PROJECT_SOURCE_DIR}/src/qtokenizer.cpp 
#   ${PROJECT_SOURCE_DIR}/src/qnmt.cpp 
#   ${PROJECT_SOURCE_DIR}/src/bpe.cpp 
#   ${PROJECT_SOURCE_DIR}/src/utils.cpp
#   )


set(NMT_REMOTE_API text-translate)
set(NMT_SOURCE_REMOTE_API 
  ${PROJECT_SOURCE_DIR}/src/text-translate.cpp 
  ${PROJECT_SOURCE_DIR}/src/rest_server.cpp 
  ${PROJECT_SOURCE_DIR}/src/tokenizer.cpp 
  ${PROJECT_SOURCE_DIR}/src/nmt.cpp 
  ${PROJECT_SOURCE_DIR}/src/smt.cpp 
  ${PROJECT_SOURCE_DIR}/src/bpe.cpp 
  ${PROJECT_SOURCE_DIR}/src/spm.cpp 
  ${PROJECT_SOURCE_DIR}/src/utils.cpp
  )
# add_executable(${NMT_EXECUTABLE_API} ${NMT_SOURCE_API})
add_executable(${NMT_REMOTE_API} ${NMT_SOURCE_REMOTE_API})


set(TEST_CLIENT_MOSES client_moses)
set(TEST_CLIENT_MOSES_SRC 
  ${PROJECT_SOURCE_DIR}/src/client_moses.cpp 
  )
set(TEST_CLIENT_MARIAN client_marian)
set(TEST_CLIENT_MARIAN_SRC 
  ${PROJECT_SOURCE_DIR}/src/client_marian.cpp 
  ${PROJECT_SOURCE_DIR}/src/utils.cpp
  )
# add_executable(${NMT_EXECUTABLE_API} ${NMT_SOURCE_API})
add_executable(${TEST_CLIENT_MOSES} ${TEST_CLIENT_MOSES_SRC})
add_executable(${TEST_CLIENT_MARIAN} ${TEST_CLIENT_MARIAN_SRC})

# target_link_libraries(${NMT_EXECUTABLE_API} pistache pthread qnlp fasttext tensorflow_framework tensorflow_qnlp predict_proto model_proto optional grpc++ prediction_service_proto classification_proto get_model_metadata_proto regression_proto input_proto inference_proto boost_regex)
target_link_libraries(${NMT_REMOTE_API} 
    pistache 
    pthread 
    qnlp     
    yaml-cpp
    fasttext 
    boost_regex 
    boost_system 
    boost_thread 
    pthread 
    sentencepiece
    easywsclient)

target_link_libraries(${TEST_CLIENT_MOSES} boost_system boost_thread pthread)
target_link_libraries(${TEST_CLIENT_MARIAN} boost_system boost_thread pthread easywsclient)

install(TARGETS ${TEST_CLIENT_MOSES} ${TEST_CLIENT_MARIAN} ${NMT_REMOTE_API}  DESTINATION bin/)
