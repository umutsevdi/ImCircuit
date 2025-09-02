# ┌──────────────┐
# │ Build replxx │
# └──────────────┘
FetchContent_Declare(
    replxx
    GIT_REPOSITORY https://github.com/AmokHuginnsson/replxx.git
    GIT_TAG master
)
FetchContent_MakeAvailable(replxx)

file(GLOB CLI_RES src/*.cpp)

add_library(cli ${CLI_RES})
target_include_directories(cli PRIVATE ${replxx_SOURCE_DIR}/include)
target_link_libraries(cli common core replxx)
