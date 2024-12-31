message("BUILDING CLIENT")

add_library(client client.cpp)

find_package(Boost REQUIRED COMPONENTS headers)
target_include_directories(client PUBLIC ${CMAKE_SOURCE_DIR}/libs/mslogger)
target_link_libraries(client Boost::headers)