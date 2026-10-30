//
// TextureCache.cpp
//	VulkanModule Adjunct
//
// See header file comment for overview.
//
// 10/29/26 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "TextureCache.h"
#include "Logging.h"

#include <cstdio>


TextureCache::~TextureCache()
{
	if (!entries.empty())
		Log(WARN, "TextureCache: %zu shared texture(s) still held at its destruction; destroying them.", entries.size());
	for (auto& [key, entry] : entries)
		delete entry.pTexture;
}

TextureImage* TextureCache::acquire(TextureSpec& spec, VkCommandPool& pool, GraphicsDevice& device, iPlatform& platform)
{
	std::string key = keyFor(spec);
	if (key.empty())
		return new TextureImage(spec, pool, device, platform);

	Entry& entry = entries[key];
	if (!entry.pTexture) {
		entry.pTexture = new TextureImage(spec, pool, device, platform);
		keysByTexture[entry.pTexture] = key;
	}
	++entry.nUsers;
	return entry.pTexture;
}

void TextureCache::release(TextureImage* pTexture)
{
	auto found = keysByTexture.find(pTexture);
	if (found == keysByTexture.end()) {		// Made for its renderable alone.
		delete pTexture;
		return;
	}
	auto entry = entries.find(found->second);
	if (--entry->second.nUsers > 0)
		return;
	delete pTexture;
	entries.erase(entry);
	keysByTexture.erase(found);
}

// Whatever makes two textures differ: the source (a file, or in-memory pixels by address and shape) and
//	how it's sampled.  In-memory pixels are keyed by the pixels themselves, not the ImageInfo describing
//	them: each renderable may describe the same pixels with its own ImageInfo.
//
std::string TextureCache::keyFor(const TextureSpec& spec)
{
	if (spec.wantMutable)
		return "";
	std::string source;
	if (spec.pImageInfo) {
		if (!spec.shareable || !spec.pImageInfo->pPixels)
			return "";
		const ImageInfo& image = *spec.pImageInfo;
		char pixels[128];
		snprintf(pixels, sizeof pixels, "pixels %p %llu bytes %dx%d format %d", image.pPixels,
				 (unsigned long long) image.numBytes, image.wide, image.high, (int) image.format);
		source = pixels;
	} else if (spec.fileName)
		source = std::string("file ") + spec.fileName;
	else
		return "";							// (A blank texture, to be written: never shared.)

	char sampling[96];
	snprintf(sampling, sizeof sampling, "|filter %d|wrap %d|flip %d|lod %g", (int) spec.filterMode, (int) spec.wrapMode,
			 (int) spec.flipVertical, spec.lodBias);
	return source + sampling;
}
