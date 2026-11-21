# Vault Heist - Data Structures project (C++17 + SFML 2.6.2)
#
# SFML is downloaded and built automatically the first time you configure,
# so the project needs nothing installed except a C++ compiler and CMake.
cmake_minimum_required(VERSION 3.24)
project(VaultHeist CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Newer CMake refuses very old minimum-version lines inside SFML's own
# build scripts; this line tells it to accept them.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Build SFML as static libraries so the final exe has no DLL dependencies.
set(BUILD_SHARED_LIBS OFF)
set(SFML_BUILD_AUDIO ON)
set(SFML_BUILD_NETWORK OFF)

include(FetchContent)
FetchContent_Declare(
    SFML
    URL https://github.com/SFML/SFML/archive/refs/tags/2.6.2.zip
)
FetchContent_MakeAvailable(SFML)

# ---- Model library: pure data structures + level data, no SFML ----
file(GLOB MODEL_SOURCES CONFIGURE_DEPENDS
    src/core/*.cpp
    src/model/*.cpp
    src/game/LevelLoader.cpp
    src/game/LevelTreeBuilder.cpp
    src/game/LevelBuilderB.cpp
    src/game/SaveManager.cpp
    src/game/Campaign.cpp
    src/game/ScoreSystem.cpp
)
add_library(vh_model STATIC ${MODEL_SOURCES})
target_include_directories(vh_model PUBLIC include)

# ---- GUI library: widgets, renderer, screens (uses SFML) ----
file(GLOB GUI_SOURCES CONFIGURE_DEPENDS
    src/gui/*.cpp
    src/gui/widgets/*.cpp
    src/screens/*.cpp
    src/game/LevelFactory.cpp
    src/game/LevelFactoryA.cpp
    src/game/LevelFactoryB.cpp
)
add_library(vh_gui STATIC ${GUI_SOURCES})
target_link_libraries(vh_gui PUBLIC vh_model sfml-graphics sfml-audio)

# ---- Executables ----
add_executable(vault_heist apps/main.cpp)
target_link_libraries(vault_heist PRIVATE vh_gui)

add_executable(dev_a apps/dev_a.cpp)
target_link_libraries(dev_a PRIVATE vh_gui)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
add_executable(run_tests ${TEST_SOURCES})
target_link_libraries(run_tests PRIVATE vh_model)

add_executable(run_bench bench/main.cpp)
target_link_libraries(run_bench PRIVATE vh_model sfml-system)

# Copy the assets folder next to the executables so they run from anywhere.
add_custom_command(TARGET vault_heist POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:vault_heist>/assets
)

# SFML's audio module always needs OpenAL at run time, even in a static build,
# so put openal32.dll beside the executables.
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(VH_OPENAL "${CMAKE_BINARY_DIR}/_deps/sfml-src/extlibs/bin/x64/openal32.dll")
    else()
        set(VH_OPENAL "${CMAKE_BINARY_DIR}/_deps/sfml-src/extlibs/bin/x86/openal32.dll")
    endif()
    add_custom_command(TARGET vault_heist POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${VH_OPENAL}" $<TARGET_FILE_DIR:vault_heist>
    )
endif()
