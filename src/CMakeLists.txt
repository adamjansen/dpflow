file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.h")

file(GLOB_RECURSE SRC_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(dplib ${SRC_LIST} ${HEADER_LIST})
target_include_directories(dplib
PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)
target_link_libraries(dplib fmt::fmt magic_enum::magic_enum libsocketcan Pal::Sigslot bytearray)
target_compile_features(dplib PUBLIC cxx_std_17)

configure_file(${PROJECT_SOURCE_DIR}/include/dplib/version.h.in
  ${PROJECT_BINARY_DIR}/dplib/version.h
)

#source_group(
#TREE "${PROJECT_SOURCE_DIR}/include"
#PREFIX "Header Files"
#FILES ${HEADER_LIST}
#)
