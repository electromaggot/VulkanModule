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

	const VkFormat				ARGUABLY_MOST_GENERAL_VK_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;

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
