//
// ShadowSystem.h
//	Vulkan Module, optional Shadowing
//
// Unified shadow mapping system that encapsulates shadow map resources and rendering.
//	Supports multiple shadow techniques and can be completely disabled for zero VRAM cost.
//
// Created 1 Oct 2025 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef ShadowSystem_h
#define ShadowSystem_h

#include <vector>
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include "ShadowMappingTypes.h"  // Shadow mapping type definitions

class VulkanSetup;
struct iRenderable;  // Changed from 'class' to 'struct' to match definition.
class ShadowMap;
class ShadowPass;

using glm::vec3;
using std::vector;


class ShadowSystem
{
public:
	// Constructor: Creates shadow resources based on technique.
	// If technique is NONE, allocates ZERO resources (no VRAM cost).
	ShadowSystem(VulkanSetup& vulkan, uint32_t numFrames,
				 ShadowTechnique technique,
				 uint32_t resolution = 2048,
				 ShadowProjectionMode projMode = SHADOW_PERSPECTIVE,
				 ShadowCameraMode camMode = SHADOW_CAMERA_STRAIGHT_DOWN);
	~ShadowSystem();

	// Query methods:
	bool isEnabled() const { return technique != SHADOW_TECHNIQUE_NONE; }
	ShadowTechnique getTechnique() const { return technique; }
	uint32_t getNumShadowMaps() const { return (uint32_t)shadowMaps.size(); }

	// Recording: Returns true if shadows were recorded, false if disabled.
	bool recordFrame(vector<iRenderable*>& renderables, uint32_t frameIndex);

	// Accessor methods for descriptor creation (safe to call even if disabled).
	VkImageView getShadowMapView(uint32_t frameIndex) const;
	VkSampler getShadowMapSampler() const;
	VkCommandBuffer getCommandBuffer(uint32_t frameIndex) const;
	VkRenderPass getRenderPass() const;	// Returns shadow map render pass (VK_NULL_HANDLE if disabled).
	VkExtent2D getExtent() const;		// Returns shadow map extent (0x0 if disabled).

	// Get per-frame descriptor info for all shadow maps (for descriptor set creation).
	// Returns empty vector if shadows disabled.
	vector<VkDescriptorImageInfo> getPerFrameDescriptorInfo() const;

	// Configuration getters (for calculating light space matrix in application).
	ShadowProjectionMode getProjectionMode() const { return projectionMode; }
	ShadowCameraMode getCameraMode() const { return cameraMode; }

private:
	ShadowTechnique technique;
	ShadowProjectionMode projectionMode;
	ShadowCameraMode cameraMode;

	// Resources (only allocated if technique != NONE)
	vector<ShadowMap*> shadowMaps;		// One per frame to prevent cross-frame races
	ShadowPass* shadowPass;				// Command buffer recorder

	// Prevent copying
	ShadowSystem(const ShadowSystem&) = delete;
	ShadowSystem& operator=(const ShadowSystem&) = delete;
};

#endif // ShadowSystem_h
