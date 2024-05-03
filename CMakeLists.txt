cmake_minimum_required(VERSION 3.14)

project(ChatBot LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

find_package(nlohmann_json REQUIRED)
include_directories(${NLOHMANN_JSON_INCLUDE_DIRS})

add_executable(ChatBot 
  src/main.cpp
)

target_link_libraries(ChatBot Qt6::Core Qt6::Widgets Qt6::Gui ${CURL_LIBRARIES} ${NLOHMANN_JSON_LIBRARIES})
