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

add_executable(Template3D
               Code/main.cpp
			   Code/Core/Utilities/pch.cpp)

add_subdirectory(Code)


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


target_precompile_headers(Template3D PRIVATE Code/Core/Utilities/pch.hpp)

if (MSVC)
    target_compile_options(Template3D PRIVATE /FI"Utilities/pch.hpp")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(Template3D PRIVATE -include Utilities/pch.hpp)
endif()

target_include_directories(Template3D PRIVATE
                           ${CMAKE_SOURCE_DIR}/Code/Core
                           ${CMAKE_SOURCE_DIR}/Code/Managers)
