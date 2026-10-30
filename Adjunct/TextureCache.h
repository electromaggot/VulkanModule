//
// TextureCache.h
//	VulkanModule Adjunct
//
// Shares TextureImages between renderables that name the same texture, so it's decoded and uploaded once
//	however many renderables show it (e.g. ten copies of a model, sixty lines of text in one font).  VulkanSetup
//	holds one; AddOns acquires its renderable's textures from it and releases them when the renderable goes.
//
// Reference counted: the last renderable to release a texture destroys it.  So a teardown that destroys every
//	renderable (a scene change, system sleep) leaves no texture behind on the device, and a texture still held
//	then is a renderable that outlived it (VulkanSetup says so at sleep).
//
// What may be shared: a file's texture; an in-memory one (pImageInfo) only if its spec says `shareable`, i.e.
//	its pixels won't change in place (a font atlas, say); never a `wantMutable` one, written to later, which
//	every renderable sharing it would show.  Anything else is made for its renderable alone, as it always was.
//
// 10/29/26 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef TextureCache_h
#define TextureCache_h

#include "TextureImage.h"
#include <map>
#include <string>


class TextureCache
{
public:
	TextureCache() = default;
	~TextureCache();

	// The texture this spec describes: shared if it may be, else made for caller alone.  Release it, don't delete it.
	TextureImage* acquire(TextureSpec& spec, VkCommandPool& pool, GraphicsDevice& device, iPlatform& platform);
	void release(TextureImage* pTexture);

	size_t nShared() const		{ return entries.size(); }		// Textures held for sharing now.

private:
	struct Entry {
		TextureImage* pTexture = nullptr;
		int			  nUsers   = 0;
	};
	std::map<std::string, Entry>		 entries;
	std::map<TextureImage*, std::string> keysByTexture;

	static std::string keyFor(const TextureSpec& spec);			// Empty: not to be shared.
};

#endif	// TextureCache_h
