message("BUILDING MICROPHONE")

add_library(microphone microphone.mm)

set(
    APPLE_FRAMEWORKS 
    "-framework Foundation"
    "-framework CoreFoundation"
    "-framework AudioToolBox" 
)
find_package(Boost REQUIRED COMPONENTS headers)

target_include_directories(microphone PUBLIC ${CMAKE_SOURCE_DIR}/libs/mslogger)

target_link_libraries(
    microphone PRIVATE 
    ${APPLE_FRAMEWORKS}
    Boost::headers
    )