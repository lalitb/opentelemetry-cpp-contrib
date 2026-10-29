# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.24)
project(geneva_etw_benchmark LANGUAGES CXX)

if(NOT WIN32)
  message(FATAL_ERROR "This benchmark requires Windows ETW")
endif()

set(OPENTELEMETRY_CPP_ROOT "" CACHE PATH "Path to an opentelemetry-cpp v1.30.0 source checkout")
if(NOT EXISTS "${OPENTELEMETRY_CPP_ROOT}/api/include/opentelemetry/version.h")
  message(FATAL_ERROR "Set OPENTELEMETRY_CPP_ROOT to the opentelemetry-cpp v1.30.0 source directory")
endif()

include(FetchContent)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(nlohmann_json)

# The tracer's virtual AddEvent overloads reference this non-inline SDK helper.
add_executable(
  geneva_etw_benchmark main.cc
  "${OPENTELEMETRY_CPP_ROOT}/sdk/src/common/empty_attributes.cc")
set(benchmark_targets geneva_etw_benchmark)
option(GENEVA_ETW_BREAKDOWN "Build an instrumented Span::End stage benchmark" OFF)
if(GENEVA_ETW_BREAKDOWN)
  include(breakdown.cmake)
  add_executable(
    geneva_etw_breakdown breakdown.cc
    "${OPENTELEMETRY_CPP_ROOT}/sdk/src/common/empty_attributes.cc")
  target_include_directories(
    geneva_etw_breakdown SYSTEM BEFORE PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/instrumented")
  list(APPEND benchmark_targets geneva_etw_breakdown)
endif()

foreach(target IN LISTS benchmark_targets)
  target_compile_features(${target} PRIVATE cxx_std_17)
  target_compile_definitions(${target} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
  target_compile_options(${target} PRIVATE /W4 /WX /external:anglebrackets /external:W0)
  target_include_directories(
    ${target} SYSTEM PRIVATE
    "${OPENTELEMETRY_CPP_ROOT}/api/include"
    "${OPENTELEMETRY_CPP_ROOT}/sdk/include"
    "${OPENTELEMETRY_CPP_ROOT}/exporters/etw/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include")
  target_link_libraries(${target} PRIVATE nlohmann_json::nlohmann_json advapi32 ole32)
endforeach()
