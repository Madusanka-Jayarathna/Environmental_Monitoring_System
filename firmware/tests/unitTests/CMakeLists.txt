cmake_minimum_required(VERSION 3.16)
project(HostUnitTests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_COVERAGE "Enable code coverage instrumentation" ON)
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang")
        message(STATUS "Enabling Clang Source-based Code Coverage")
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping -O0 -g)
        add_link_options(-fprofile-instr-generate)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        message(STATUS "Enabling GCC gcov Code Coverage")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    endif()
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/esp_adc
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/driver
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/freertos
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/libSensors/public
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/libComponentHandler/public
)

set(LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/libSensors/private/SoilMoistureFirmware.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/libSensors/private/BME280Firmware.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/libSensors/private/firmwareUtils.cpp
)

set(TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_SoilMoistureFirmware.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_BME280Firmware.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_ComponentHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main_test_runner.cpp
)

add_executable(run_unit_tests ${LIB_SOURCES} ${TEST_SOURCES})

enable_testing()
add_test(NAME HostUnitTests COMMAND run_unit_tests)
