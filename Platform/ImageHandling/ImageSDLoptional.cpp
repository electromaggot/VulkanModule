//
// ImageSDLoptional.cpp
//	General App Chassis
//
// Empty implementation if you want SDL but not SDL_image.
//
// Created 2/7/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "ImageSDL.h"
#include "sdlEquivalents.h"


ImageSDL::ImageSDL()
	:	pImage(nullptr)
{ }

ImageSDL::~ImageSDL()
{ }

ImageInfo ImageSDL::Load(StrPtr filePath) {
	return { nullptr, 0, VK_FORMAT_UNDEFINED, 0, 0 };
}

ImageInfo ImageSDL::validatedImageInfo(const char* strOp) {
	return { nullptr, 0, VK_FORMAT_UNDEFINED, 0, 0 };
}

VkFormat ImageSDL::findVkFormatEquivalentTo(SDL_PixelFormatEnum sdlFormatRequested) {
	return VK_FORMAT_UNDEFINED;
}

SDL_PixelFormatEnum ImageSDL::findSDLFormatEquivalentTo(VkFormat vkFormatRequested) {
	return SDL_PIXELFORMAT_UNKNOWN;
}

ImageInfo ImageSDL::ConvertTo(VkFormat toFormat, bool fallbackOnFailure) {
	return { nullptr, 0, VK_FORMAT_UNDEFINED, 0, 0 };
}
