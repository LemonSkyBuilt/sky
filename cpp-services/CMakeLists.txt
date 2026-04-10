cmake_minimum_required(VERSION 3.20)

project(cpp-services VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(CTest)

function(trading_apply_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

function(add_trading_service target_name)
    add_executable(${target_name} ${ARGN})
    target_compile_features(${target_name} PRIVATE cxx_std_20)
    target_link_libraries(${target_name} PRIVATE trading_common)
    trading_apply_warnings(${target_name})
    add_test(NAME ${target_name}-self-check COMMAND ${target_name} --self-check)
endfunction()

add_subdirectory(common)
add_subdirectory(matching-engine)
add_subdirectory(risk-core)
add_subdirectory(market-replay-engine)
