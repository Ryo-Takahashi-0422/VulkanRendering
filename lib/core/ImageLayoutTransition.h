#pragma once

struct ImageLayoutTransition {
	VkImageLayout oldLayout;
	VkImageLayout newLayout;
	VkAccessFlags srcAccessMask;
	VkAccessFlags dstAccessMask;
	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	// Undefine状態から描画先としてのレイアウトへ
	static ImageLayoutTransition FromUnderfinedToColorAttachment();

	// PresentSrc状態から描画先としてのレイアウトへ
	static ImageLayoutTransition FromPresentSrcToColorAttachment();

	// 描画先からPresentSrc状態としてのレイアウトへ
	static ImageLayoutTransition FromColorToPresent();
};