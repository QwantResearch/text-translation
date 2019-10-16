# Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
project(text-translation)

cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -fPIC")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

# Protobuf compiler dependency.
include(CompileProto.cmake)

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Protobuf sources of the TensorFlow Serving to be compiled without a gRPC plugin.
file(GLOB_RECURSE TF_PROTOS protos/*.proto)

# Compiling CPP sources from proto files.
compile_proto(0 "${CMAKE_SOURCE_DIR}/protos" "${CMAKE_SOURCE_DIR}/compiled" PB_SOURCES PB_HEADERS ${TF_PROTOS})

# Compiling CPP sources with gRPC plugin.
compile_proto(1 "${CMAKE_SOURCE_DIR}/protos" "${CMAKE_SOURCE_DIR}/compiled" PB_GRPC_SOURCES PB_GRPC_HEADERS
        protos/tensorflow_serving/apis/prediction_service.proto
        protos/tensorflow_serving/apis/model_service.proto
        protos/grpc_nmt.proto
)

# Including compiled files.
include_directories(compiled)


include_directories(${PROJECT_SOURCE_DIR}/include/ /usr/local/include/ /usr/)

link_directories(/usr/local/lib/ /usr/lib/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -lpthread")

set(NMT_REMOTE_API text-translation)
set(NMT_SOURCE_REMOTE_API 
                    ${PROJECT_SOURCE_DIR}/src/text-translation.cpp 
                    ${PROJECT_SOURCE_DIR}/src/rest_server.cpp 
                    ${PROJECT_SOURCE_DIR}/src/tokenizer.cpp 
                    ${PROJECT_SOURCE_DIR}/src/nmt.cpp 
                    ${PROJECT_SOURCE_DIR}/src/smt.cpp 
                    ${PROJECT_SOURCE_DIR}/src/bpe.cpp 
                    ${PROJECT_SOURCE_DIR}/src/spm.cpp 
                    ${PROJECT_SOURCE_DIR}/src/utils.cpp
                    ${PROJECT_SOURCE_DIR}/src/abstract_server.cpp
                    ${PROJECT_SOURCE_DIR}/src/grpc_route_nmt_impl.cpp
                    ${PROJECT_SOURCE_DIR}/src/grpc_server.cpp

                    ${PB_GRPC_HEADERS}
                    ${PB_GRPC_SOURCES}
                    ${PB_HEADERS}
                    ${PB_SOURCES}
  )
# add_executable(${NMT_EXECUTABLE_API} ${NMT_SOURCE_API})
add_executable(${NMT_REMOTE_API} ${NMT_SOURCE_REMOTE_API})


set(TEST_CLIENT client_moses)
set(TEST_CLIENT_SRC 
  ${PROJECT_SOURCE_DIR}/src/client_moses.cpp 
  )
set(TEST_CLIENT_MARIAN client_marian)
set(TEST_CLIENT_MARIAN_SRC 
  ${PROJECT_SOURCE_DIR}/src/client_marian.cpp 
  ${PROJECT_SOURCE_DIR}/src/utils.cpp
  )
# add_executable(${NMT_EXECUTABLE_API} ${NMT_SOURCE_API})
add_executable(${TEST_CLIENT} ${TEST_CLIENT_SRC})
add_executable(${TEST_CLIENT_MARIAN} ${TEST_CLIENT_MARIAN_SRC})

# target_link_libraries(${NMT_EXECUTABLE_API} pistache pthread qnlp fasttext tensorflow_framework tensorflow_qnlp predict_proto model_proto optional grpc++ prediction_service_proto classification_proto get_model_metadata_proto regression_proto input_proto inference_proto boost_regex)
target_link_libraries(${NMT_REMOTE_API} 
    ${_GRPC_GRPCPP_UNSECURE}
    ${_PROTOBUF_LIBPROTOBUF}
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

target_link_libraries(${TEST_CLIENT} boost_system boost_thread pthread)
target_link_libraries(${TEST_CLIENT_MARIAN} boost_system boost_thread pthread easywsclient)

install(TARGETS ${NMT_REMOTE_API} DESTINATION bin)


