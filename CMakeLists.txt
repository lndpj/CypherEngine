cmake_minimum_required(VERSION 3.20)

project(CypherEngine
    VERSION 0.1.0
    DESCRIPTION "CypherEngine from-scratch C++20 3D engine inspired by idTech, GoldSrc, and early CryEngine"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    add_compile_options(/Zc:preprocessor)
endif()

# Keep binaries and intermediate files in the build directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(CYPHERENGINE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CYPHERENGINE_SRC_DIR "${CYPHERENGINE_ROOT_DIR}/src")
set(CYPHERENGINE_THIRDPARTY_DIR "${CYPHERENGINE_ROOT_DIR}/thirdparty")

set(CYPHERENGINE_INCLUDE_DIRS
    "${CYPHERENGINE_SRC_DIR}"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherCommon"
    "${CYPHERENGINE_SRC_DIR}/CypherCommon"
    "${CYPHERENGINE_SRC_DIR}/CypherCommon/Tier0"
    "${CYPHERENGINE_SRC_DIR}/CypherCommon/Tier1"
    "${CYPHERENGINE_SRC_DIR}/CypherCommon/Tier2"
    "${CYPHERENGINE_SRC_DIR}/CypherCommon/Tier3"
    "${CYPHERENGINE_SRC_DIR}/CypherCommon/Tier4"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherAI"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherAnimation"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherAudio"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherCommand"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherConfig"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherConsole"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherCVar"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherEntity"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherFileSystem"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherHost"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherInput"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherLog"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherMath"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherMemory"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherNetwork"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherPak"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherPhysics"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherPlatform"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherProfile"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherRender"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherResource"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherScript"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherSystem"
    "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherWorld"
    "${CYPHERENGINE_SRC_DIR}/CypherClient"
    "${CYPHERENGINE_SRC_DIR}/CypherEditor"
    "${CYPHERENGINE_SRC_DIR}/CypherGame"
    "${CYPHERENGINE_SRC_DIR}/CypherServer"
    "${CYPHERENGINE_SRC_DIR}/CypherTools"
    "${CYPHERENGINE_SRC_DIR}/rengine"
    "${CYPHERENGINE_SRC_DIR}/rengine/math"
)

# Optional dependencies: only enabled when the corresponding files exist.
set(CYPHERENGINE_LINK_LIBS "")

if (EXISTS "${CYPHERENGINE_THIRDPARTY_DIR}/glad/src/gl.c")
    message(STATUS "Found vendored glad")
    add_library(glad STATIC "${CYPHERENGINE_THIRDPARTY_DIR}/glad/src/gl.c")
    target_include_directories(glad PUBLIC "${CYPHERENGINE_THIRDPARTY_DIR}/glad/include")
    list(APPEND CYPHERENGINE_LINK_LIBS glad)
else()
    message(STATUS "No vendored glad yet. Add thirdparty/glad before enabling modern OpenGL loading.")
endif()

if (EXISTS "${CYPHERENGINE_THIRDPARTY_DIR}/raylib/CMakeLists.txt")
    message(STATUS "Found vendored raylib")
    add_subdirectory(${CYPHERENGINE_THIRDPARTY_DIR}/raylib raylib_build)
    list(APPEND CYPHERENGINE_LINK_LIBS raylib)
elseif (EXISTS "${CYPHERENGINE_THIRDPARTY_DIR}/raylib/src")
    message(STATUS "Found raylib tree but no CMakeLists.txt; skipping raylib add_subdirectory")
else()
    message(STATUS "No vendored raylib. CypherEngine currently uses SDL3/OpenGL directly.")
endif()

if (EXISTS "${CYPHERENGINE_THIRDPARTY_DIR}/flecs/include/flecs.h")
    add_library(flecs INTERFACE)
    target_include_directories(flecs INTERFACE "${CYPHERENGINE_THIRDPARTY_DIR}/flecs/include")
    list(APPEND CYPHERENGINE_LINK_LIBS flecs)
else()
    message(STATUS "No vendored Flecs headers yet. Add thirdparty/flecs/include/flecs.h.")
endif()

if (EXISTS "${CYPHERENGINE_THIRDPARTY_DIR}/glm/glm/glm.hpp")
    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE "${CYPHERENGINE_THIRDPARTY_DIR}/glm")
    list(APPEND CYPHERENGINE_LINK_LIBS glm)
else()
    message(STATUS "No vendored GLM headers yet. Add thirdparty/glm/glm/glm.hpp.")
endif()

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
list(APPEND CYPHERENGINE_LINK_LIBS SDL3::SDL3)

file(GLOB_RECURSE CYPHERENGINE_SOURCES CONFIGURE_DEPENDS
    "${CYPHERENGINE_SRC_DIR}/*.cpp"
    "${CYPHERENGINE_SRC_DIR}/*.cxx"
    "${CYPHERENGINE_SRC_DIR}/*.cc"
)

if (CYPHERENGINE_SOURCES)
    add_executable(CypherEngine ${CYPHERENGINE_SOURCES})
    target_include_directories(CypherEngine PRIVATE ${CYPHERENGINE_INCLUDE_DIRS})
    if (CYPHERENGINE_LINK_LIBS)
        target_link_libraries(CypherEngine PRIVATE ${CYPHERENGINE_LINK_LIBS})
    endif()
else()
    message(STATUS "src/ has no C++ sources yet. Add your first entry file at src/main.cpp to enable executable target.")
endif()

option(CYPHERENGINE_BUILD_TESTS "Build CypherEngine test targets" OFF)

if (CYPHERENGINE_BUILD_TESTS)
    enable_testing()

    add_executable(cypher_common_tier0_smoke "${CYPHERENGINE_ROOT_DIR}/tests/CypherCommon_Tier0_Smoke.cpp")
    target_include_directories(cypher_common_tier0_smoke PRIVATE ${CYPHERENGINE_INCLUDE_DIRS})
    add_test(NAME cypher_common_tier0_smoke COMMAND cypher_common_tier0_smoke)

    set(CYPHERENGINE_FILESYSTEM_SMOKE_SOURCES
        "${CYPHERENGINE_ROOT_DIR}/tests/CypherFileSystem_Smoke.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherFileSystem/CypherFileSystem.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherFileSystem/CypherFileSystem_Async.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherFileSystem/CypherFileSystem_Diagnostics.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherFileSystem/CypherFileSystem_Discovery.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherFileSystem/CypherFileSystem_Directory.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherFileSystem/CypherFileSystem_Mount.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherFileSystem/CypherFileSystem_Package.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherFileSystem/CypherFileSystem_Path.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherFileSystem/CypherFileSystem_Runtime.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherFileSystem/CypherFileSystem_Watch.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherPak/CypherPak_Compression.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherPak/CypherPak_Reader.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherPak/CypherPak_Writer.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherLog/CypherLog.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherLog/CypherLog_Format.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherSystem/CypherSystem_Platform.cpp"
    )

    add_executable(cypher_filesystem_smoke ${CYPHERENGINE_FILESYSTEM_SMOKE_SOURCES})
    target_include_directories(cypher_filesystem_smoke PRIVATE ${CYPHERENGINE_INCLUDE_DIRS})
    add_test(NAME cypher_filesystem_smoke COMMAND cypher_filesystem_smoke)

    set(CYPHERENGINE_PAK_SMOKE_SOURCES
        "${CYPHERENGINE_ROOT_DIR}/tests/CypherPak_Smoke.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherPak/CypherPak_Compression.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherPak/CypherPak_Reader.cpp"
        "${CYPHERENGINE_SRC_DIR}/CypherEngine/CypherPak/CypherPak_Writer.cpp"
    )

    add_executable(cypher_pak_smoke ${CYPHERENGINE_PAK_SMOKE_SOURCES})
    target_include_directories(cypher_pak_smoke PRIVATE ${CYPHERENGINE_INCLUDE_DIRS})
    add_test(NAME cypher_pak_smoke COMMAND cypher_pak_smoke)
endif()

if (EXISTS "${CYPHERENGINE_ROOT_DIR}/tools/CMakeLists.txt")
    option(CYPHERENGINE_BUILD_TOOLS "Build tool targets" OFF)
    if (CYPHERENGINE_BUILD_TOOLS)
        add_subdirectory("${CYPHERENGINE_ROOT_DIR}/tools")
    endif()
else()
    message(STATUS "No tools/CMakeLists.txt yet. Tool targets will be added in a later phase.")
endif()
