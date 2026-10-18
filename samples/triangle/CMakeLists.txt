# プロジェクト名
set(APP_NAME 01.Triangle)

# 共通CMakeモジュールを読み込み
include(${CMAKE_SOURCE_DIR}/samples/cmake/CompileShaders.cmake)

# シェーダーのパス設定
set(SHADER_DIR ${CMAKE_SOURCE_DIR}/assets/shaders/triangle)
set(SHADER_VERT ${SHADER_DIR}/triangle.vert)
set(SHADER_FRAG ${SHADER_DIR}/triangle.frag)

# シェーダーのコンパイル設定
compile_shader(${SHADER_VERT} COMPILED_VERT)
compile_shader(${SHADER_FRAG} COMPILED_FRAG)

# 共通コアのソース・ヘッダー
file(GLOB CORE_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/lib/core/*.h
)
file(GLOB CORE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/lib/core/*.cpp
)

# サンプル側のソース・ヘッダー
file(GLOB SAMPLE_APP_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
file(GLOB SAMPLE_APP_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# Visual Studio のフィルター
source_group("App/Sources Files" FILES ${SAMPLE_APP_SOURCES})
source_group("App/Header Files" FILES ${SAMPLE_APP_HEADERS})
source_group("Shaders" FILES ${SHADER_VERT} ${SHADER_FRAG})
source_group("Core/Source Files" FILES ${CORE_SOURCES})
source_group("Core/Header Files" FILES ${CORE_HEADERS})

# 実行ファイル
add_executable(${APP_NAME} WIN32
    ${SAMPLE_APP_HEADERS}
    ${SAMPLE_APP_SOURCES}
    ${SHADER_VERT}
    ${SHADER_FRAG}
    ${CORE_HEADERS}
    ${CORE_SOURCES}
)

# インクルードパス
target_include_directories(${APP_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/lib"
    "${CMAKE_SOURCE_DIR}/lib/core"
    "${stb_SOURCE_DIR}"
)

# プリコンパイル済みヘッダー
target_precompile_headers(${APP_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/core/pch.h
)

# リンク設定
target_link_libraries(${APP_NAME} PRIVATE
    glm
    glfw
    Vulkan::Vulkan
)

# コンパイル済みシェーダーを登録
target_sources(${APP_NAME} PRIVATE
    ${COMPILED_VERT}
    ${COMPILED_FRAG}
)
