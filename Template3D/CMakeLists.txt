cmake_minimum_required(VERSION 3.26)

set(CMAKE_TOOLCHAIN_FILE "D:/PackagesCPP/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(Template3D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h" REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)

add_executable(Template3D "Code/main.cpp")

file(GLOB_RECURSE sourceFiles
     "${CMAKE_SOURCE_DIR}/Code/*.cpp"
     "${CMAKE_SOURCE_DIR}/Code/*.h"
     "${CMAKE_SOURCE_DIR}/Code/*.hpp"
     "${CMAKE_SOURCE_DIR}/Code/*.inl")

list(REMOVE_ITEM sourceFiles "${CMAKE_SOURCE_DIR}/Code/main.cpp")

target_sources(Template3D PRIVATE ${sourceFiles})

source_group("Project" FILES "Code/main.cpp")
foreach(sourceFile IN ITEMS ${sourceFiles})
    get_filename_component(sourcePath "${sourceFile}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}/Code" "" groupPath "${sourcePath}")
    string(REPLACE "/" "\\" groupPath "${groupPath}")
    source_group("Project\\${groupPath}" FILES "${sourceFile}")
endforeach()

target_compile_options(Template3D PRIVATE /utf-8)
target_compile_definitions(Template3D PRIVATE GLM_ENABLE_EXPERIMENTAL)

target_link_libraries(Template3D PRIVATE glm::glm)
target_link_libraries(Template3D PRIVATE spdlog::spdlog)
target_include_directories(Template3D PRIVATE ${TINYGLTF_INCLUDE_DIRS})
target_link_libraries(Template3D PRIVATE Vulkan::Vulkan)
target_link_libraries(Template3D PRIVATE glfw)
target_link_libraries(Template3D PRIVATE magic_enum::magic_enum)
target_link_libraries(Template3D PRIVATE glad::glad)
target_link_libraries(Template3D PRIVATE glslang::glslang glslang::glslang-default-resource-limits glslang::SPIRV glslang::SPVRemapper)

# Precompiled headers and force include for pch
target_precompile_headers(Template3D PRIVATE Code/Core/Utilities/pch.hpp)
if (MSVC)
    target_compile_options(Template3D PRIVATE /FI"Utilities/pch.hpp")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(Template3D PRIVATE -include Utilities/pch.hpp)
endif()

target_include_directories(Template3D PRIVATE
    ${CMAKE_SOURCE_DIR}/Code/Core
    ${CMAKE_SOURCE_DIR}/Code/Managers)