cmake_minimum_required(VERSION 3.14...3.26)

# ---- Project ----

# Note: update this to your new project's name and version
project(
  DPFlow
  VERSION 1.0
  DESCRIPTION "DataPanel Flow"
  LANGUAGES C CXX
)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

set(GLOBAL PROPERTY USE_FOLDERS ON)

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

include(cmake/CPM.cmake)
include(cmake/Doxygen.cmake)
include(cmake/tools.cmake)

CPMAddPackage("gh:fmtlib/fmt#9.1.0")

CPMAddPackage(
  GITHUB_REPOSITORY gabime/spdlog
  VERSION 1.11.0
  OPTIONS "SPDLOG_FMT_EXTERNAL 1"
)

CPMAddPackage(
  GITHUB_REPOSITORY Neargye/magic_enum
  VERSION 0.8.2
  OPTIONS "MAGIC_ENUM_OPT_INSTALL ON"
)

CPMAddPackage("gh:Megaxela/ByteArray#1775033")

CPMAddPackage(
  NAME libsocketcan
  GIT_REPOSITORY https://git.pengutronix.de/git/tools/libsocketcan
  VERSION 0.0.12
  DOWNLOAD_ONLY YES
)

if (libsocketcan_ADDED)
  add_library(libsocketcan STATIC ${libsocketcan_SOURCE_DIR}/src/libsocketcan.c)

  target_include_directories(libsocketcan
    PUBLIC
      $<BUILD_INTERFACE:${libsocketcan_SOURCE_DIR}>/include
  )
endif()

CPMAddPackage(
  GITHUB_REPOSITORY palacaze/sigslot
  VERSION 1.2.2
)
# ---- Add source files ----

add_subdirectory(src)

add_subdirectory(apps)

add_subdirectory(test)

add_clang_format_target()
