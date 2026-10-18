cmake_minimum_required(VERSION 3.25)
project(VulkanPracticalGuideSamples)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Vulkan REQUIRED)


include(FetchContent)

# GLFW を自動で取得・ビルド
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# STB 
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG     master
)
FetchContent_MakeAvailable(stb)

# GLM (Vulkan SDKと同じ場所のものは使用せず、本サンプル用に固定)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# VOLK (Vulkan SDKと同じ場所のものは使用せず、本サンプル用に固定)
FetchContent_Declare(
    volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
    GIT_TAG        1.4.304  ## 現時点でVulkanSDKと完全一致のものが無いため、妥協
    EXCLUDE_FROM_ALL        ## 本ライブラリのビルドは不要
)
FetchContent_MakeAvailable(volk)

# Assimp
if(NOT UNIX)
  include(FetchContent)
  FetchContent_Declare(
      assimp
      GIT_REPOSITORY https://github.com/assimp/assimp.git
      GIT_TAG        v5.4.0
  )
  set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1)    ## Assimpの構成時に警告が出るため抑制.
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE )
  set(ASSIMP_EXPORTER_ENABLED OFF CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
  set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_OBJ_IMPORTER ON  CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(assimp)
endif()

add_subdirectory(samples)

# スタートアッププロジェクト
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT 01.Triangle)