add_library(trading_common STATIC
    src/service_runtime.cpp
)

target_compile_features(trading_common PUBLIC cxx_std_20)
target_include_directories(trading_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

trading_apply_warnings(trading_common)
