message("BUILDING MICROPHONE")

add_library(microphone microphone.mm)

set(
    APPLE_FRAMEWORKS 
    "-framework Foundation"
    "-framework CoreFoundation"
    "-framework AudioToolBox" 
)
find_package(Boost REQUIRED COMPONENTS headers)

target_link_libraries(
    microphone PRIVATE 
    ${APPLE_FRAMEWORKS}
    Boost::headers
    )