//
// ImageSDL.h
//	General App Chassis
//
//	SDL concretion of iImageSource.
//
// Created 2/7/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef ImageSDL_h
#define ImageSDL_h

#include "iImageSource.h"
#include <SDL_image.h>


class ImageSDL : public iImageSource
{
	const SDL_PixelFormatEnum	ARGUABLY_MOST_COMMON_SDL_FORMAT = SDL_PIXELFORMAT_ARGB8888;

	// Use A8B8G8R8_UNORM_PACK32 (not R8G8B8A8_UNORM) as the fallback:
	// On little-endian (macOS ARM), PACK32 reverse-maps to SDL_PIXELFORMAT_ABGR8888
	//	which stores bytes [R,G,B,A] in memory — matching the VK format's byte layout.
	// R8G8B8A8_UNORM maps to SDL_PIXELFORMAT_RGBA8888 which stores [A,B,G,R] — wrong byte order.
	const VkFormat				ARGUABLY_MOST_GENERAL_VK_FORMAT = VK_FORMAT_A8B8G8R8_UNORM_PACK32;

private:
	SDL_Surface*	pImage;

public:
	ImageSDL();

	~ImageSDL();

protected:
	ImageInfo Load(StrPtr filePath);

	ImageInfo validatedImageInfo(const char* strOp = "");

	VkFormat findVkFormatEquivalentTo(SDL_PixelFormatEnum sdlFormatRequested);

	SDL_PixelFormatEnum findSDLFormatEquivalentTo(VkFormat vkFormatRequested);

	ImageInfo ConvertTo(VkFormat toFormat, bool fallbackOnFailure = true);
};

#endif	// ImageSDL_h
