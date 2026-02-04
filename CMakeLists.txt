cmake_minimum_required(VERSION 3.10)
project(BondPricingEngine)

# Specify the C++ Standard (C++17 or C++20)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include the header files
include_directories(include)

# Define the source files
set(SOURCES
    src/main.cpp
    src/YieldCurve.cpp
    src/Bond.cpp
    src/Instruments.cpp
    src/RiskEngine.cpp
)

# Create the executable
add_executable(PricingEngine ${SOURCES})