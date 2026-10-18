# GLM の設定
add_compile_definitions(
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
)

# Triangle サンプルを登録
add_subdirectory(triangle)