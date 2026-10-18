#pragma once

// アセットのルートパスを設定する
void SetAssetRootPath(const std::filesystem::path& path);

// 現在のアセットのルートパスを設定する
std::filesystem::path GetAssetRootPath();

// アセット種別
enum class AssetType
{
	Shader = 0,
	Texture,
	Model,

	AssetTypeMAx,
};

// アセット種別指定ありのファイルパスを取得する
std::filesystem::path GetAssetPath(AssetType type, const std::filesystem::path& fileName);
