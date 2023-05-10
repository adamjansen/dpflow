CPMAddPackage(GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION 3.0.0
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)

CPMAddPackage("gh:ThePhD/sol2@3.3.0")

add_executable(dpflow src/main.cpp)
target_compile_features(dpflow PRIVATE cxx_std_17)

target_include_directories(dpflow PRIVATE ${PROJECT_BINARY_DIR})

target_link_libraries(dpflow PRIVATE dplib fmt::fmt cxxopts sol2 magic_enum)
