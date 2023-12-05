message("BUILDING MACICECAST")
cmake_minimum_required(VERSION 3.14)

project(macicecast LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.mm)
# add_compile_options(-g)
# add_compile_options(-O0)

set(MICROPHONE_USAGE_DESCRIPTION "We need access to your microphone to record audio for your app's purpose.")
configure_file(Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist @ONLY)
set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
)


add_subdirectory(libs/microphone)
add_subdirectory(libs/encoder)
add_subdirectory(libs/client)

target_include_directories(
    ${PROJECT_NAME} PRIVATE 
    libs/microphone
    libs/encoder
    libs/client
    )
target_link_libraries(
    ${PROJECT_NAME} PRIVATE 
    microphone
    encoder
    client
    )


# FOR BUILDING A MacOS Application

# Set the source file for your executable
set(EXECUTABLE_SOURCE "${CMAKE_BINARY_DIR}/macicecast")

# Create the macOS application bundle
set(BUNDLE_NAME "macicecast")
set(BUNDLE_DESTINATION "${CMAKE_BINARY_DIR}/${BUNDLE_NAME}.app")
set(CONTENTS_DIR "${BUNDLE_DESTINATION}/Contents")
set(MACOS_DIR "${CONTENTS_DIR}/MacOS")
set(FRAMEWORKS_DIR "${CONTENTS_DIR}/Frameworks")
set(RESOURCES_DIR "${CONTENTS_DIR}/Resources")

# Create the necessary directories within the bundle
file(MAKE_DIRECTORY ${MACOS_DIR} ${FRAMEWORKS_DIR} ${RESOURCES_DIR})

# Copy your executable to the bundle
install(PROGRAMS "${EXECUTABLE_SOURCE}" DESTINATION ${MACOS_DIR})

# open "Package Example.command"
