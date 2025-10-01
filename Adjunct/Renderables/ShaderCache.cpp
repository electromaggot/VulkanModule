//
// ShaderCache.cpp
//	VulkanModule Adjunct
//
// See header file comment for overview.
//
// Created by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "ShaderCache.h"
#include "Logging.h"

ShaderCache::ShaderCache(GraphicsDevice& device)
	: graphicsDevice(device)
{
}

ShaderCache::~ShaderCache()
{
	// Clean up any remaining cached shaders
	for (auto& pair : shaderCache) {
		Log(WARN, "ShaderCache: Deleting unreleased shader set: %s", pair.first.c_str());
		delete pair.second;
	}
	shaderCache.clear();
	refCounts.clear();
}

std::string ShaderCache::createKey(const Shaders& shaders) const
{
	std::string key;
	for (const auto& shader : shaders) {
		if (!key.empty())
			key += "|";
		key += std::to_string(shader.fileType) + ":" + shader.nameShaderFile;
	}
	return key;
}

ShaderModules* ShaderCache::getOrCreate(const Shaders& shaders)
{
	std::string key = createKey(shaders);

	// Check if already cached
	auto it = shaderCache.find(key);
	if (it != shaderCache.end()) {
		Log(NOTE, "ShaderCache: Reusing cached shaders: %s", key.c_str());
		return it->second;
	}

	// Create new ShaderModules
	Log(NOTE, "ShaderCache: Loading new shader set: %s", key.c_str());
	ShaderModules* pShaderModules = new ShaderModules(const_cast<Shaders&>(shaders), graphicsDevice);

	shaderCache[key] = pShaderModules;
	refCounts[pShaderModules] = 0;  // Will be incremented by addRef()

	return pShaderModules;
}

void ShaderCache::addRef(ShaderModules* pShaderModules)
{
	if (pShaderModules == nullptr)
		return;

	auto it = refCounts.find(pShaderModules);
	if (it != refCounts.end()) {
		it->second++;
		Log(LOW, "ShaderCache: AddRef -> refcount = %d", it->second);
	} else {
		Log(WARN, "ShaderCache: AddRef called on untracked ShaderModules!");
	}
}

void ShaderCache::release(ShaderModules* pShaderModules)
{
	if (pShaderModules == nullptr)
		return;

	auto it = refCounts.find(pShaderModules);
	if (it == refCounts.end()) {
		Log(WARN, "ShaderCache: Release called on untracked ShaderModules!");
		return;
	}

	it->second--;
	Log(LOW, "ShaderCache: Release -> refcount = %d", it->second);

	if (it->second <= 0) {
		// Find and remove from cache
		for (auto cacheIt = shaderCache.begin(); cacheIt != shaderCache.end(); ++cacheIt) {
			if (cacheIt->second == pShaderModules) {
				Log(NOTE, "ShaderCache: Deleting shader set: %s", cacheIt->first.c_str());
				delete pShaderModules;
				shaderCache.erase(cacheIt);
				break;
			}
		}
		refCounts.erase(it);
	}
}
