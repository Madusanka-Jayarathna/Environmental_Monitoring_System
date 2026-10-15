# Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
# Use of this source code is governed by the MIT license.

cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXTRA_COMPONENT_DIRS "tests")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(envMonitoringSys)
