# コンパイル時に使用するプリプロセッサマクロを定義する
# GLMの設定
add_compile_definitions(
    GLM_FORCE_DEPTH_ZERO_TO_ONE # GLMの投影行列における深度（Z座標）の範囲を、0～1にする
    GLM_FORCE_RADIANS # GLMで角度を扱う際に、ラジアンを使用する(最近のは基本ラジアン。古いGLMとの互換性を意識した設定)
)

# Triangle サンプルを登録
add_subdirectory(triangle)