//
// ShaderCache.h
//	VulkanModule Adjunct
//
// Manages shared ShaderModules to avoid redundant shader loading.
// Shaders are cached by their shader file names, and reference counted
// for proper cleanup.
//
// Created by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef ShaderCache_h
#define ShaderCache_h

#include "ShaderModules.h"
#include "GraphicsDevice.h"
#include <map>
#include <string>

class ShaderCache
{
public:
	ShaderCache(GraphicsDevice& device);
	~ShaderCache();

	// Get or create ShaderModules for the given shader set
	// Returns pointer to shared ShaderModules (do not delete directly!)
	ShaderModules* getOrCreate(const Shaders& shaders);

	// Increment reference count (called when a renderable uses these shaders)
	void addRef(ShaderModules* pShaderModules);

	// Decrement reference count (called when a renderable is destroyed)
	// Deletes the ShaderModules if refcount reaches zero
	void release(ShaderModules* pShaderModules);

private:
	GraphicsDevice& graphicsDevice;

	// Map shader names to ShaderModules instance
	std::map<std::string, ShaderModules*> shaderCache;

	// Reference counts for each ShaderModules instance
	std::map<ShaderModules*, int> refCounts;

	// Create a unique key from shader file names
	std::string createKey(const Shaders& shaders) const;
};

#endif // ShaderCache_h
