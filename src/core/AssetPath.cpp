#include "pch.h"
#include "core/AssetPath.h"
#include <string_view>

namespace {
	std::filesystem::path g_assetRoot = "assets/";

	std::string_view ToSubDirectoryName(AssetType type)
	{
		constexpr std::array<std::string_view, int(AssetType::AssetTypeMAx)> kAssetDirs = {
			"shaders", "textures", "models",
		};
		return kAssetDirs[int(type)];
	}
}

void SetAssetRootPath(const std::filesystem::path& path) 
{
	auto fullPath = std::filesystem::absolute(path);
	g_assetRoot = std::filesystem::canonical(fullPath); // ê≥ãKâª
}

std::filesystem::path GetAssetRootPath()
{
	return g_assetRoot;
}

std::filesystem::path GetAssetPath(AssetType type, const std::filesystem::path& fileName)
{
	return GetAssetRootPath() / ToSubDirectoryName(type) / fileName;
}
