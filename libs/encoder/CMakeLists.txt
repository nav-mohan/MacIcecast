message("BUILDING ENCODER")

add_library(encoder encoder.cpp)

# Apple Silicon HomeBrew
set(LAME_ROOT_DIR /opt/homebrew/Cellar/lame/3.100)
set(AAC_ROOT_DIR /opt/homebrew/Cellar/fdk-aac/2.0.2)

# Apple Intel HomeBrew
# set(LAME_ROOT_DIR /usr/local/Cellar/lame/3.100)
# set(AAC_ROOT_DIR /usr/local/Cellar/fdk-aac/2.0.2)

set(LAME_LINK_LIBRARY ${LAME_ROOT_DIR}/lib/libmp3lame.a)
set(LAME_INCLUDE_DIR ${LAME_ROOT_DIR}/include)
set(AAC_LINK_LIBRARY ${AAC_ROOT_DIR}/lib/libfdk-aac.a)
set(AAC_INCLUDE_DIR ${AAC_ROOT_DIR}/include)

target_include_directories(encoder PUBLIC ${LAME_INCLUDE_DIR} ${AAC_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libs/mslogger)
target_link_libraries(encoder PRIVATE ${LAME_LINK_LIBRARY} ${AAC_LINK_LIBRARY})