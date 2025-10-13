//
// ShadowSystem.cpp
//
// Implementation of unified shadow mapping system.
//
// Created 1 Oct 2025 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "ShadowSystem.h"
#include "ShadowMap.h"
#include "ShadowPass.h"
#include "VulkanSetup.h"
#include "CommandObjects.h"
#include "Logging.h"


ShadowSystem::ShadowSystem(VulkanSetup& vulkan, uint32_t numFrames,
						   ShadowTechnique tech,
						   uint32_t resolution,
						   ShadowProjectionMode projMode,
						   ShadowCameraMode camMode)
	: technique(tech)
	, projectionMode(projMode)
	, cameraMode(camMode)
	, shadowPass(nullptr)
{
	if (technique == SHADOW_TECHNIQUE_NONE) {
		// No resources allocated - zero VRAM cost.
		Log(NOTE, "ShadowSystem: Shadows DISABLED (zero VRAM allocation)");
		return;
	}

	// Allocate shadow resources...
	Log(NOTE, "ShadowSystem: Creating shadow resources with technique=%d, resolution=%ux%u",
		technique, resolution, resolution);

	// Create one shadow map per frame to prevent cross-frame race conditions:
	for (uint32_t i = 0; i < numFrames; ++i) {
		ShadowMap* shadowMap = new ShadowMap(vulkan.device, vulkan.command.getCommandPool(),
											  resolution, resolution);
		shadowMaps.push_back(shadowMap);
	}
	Log(NOTE, "ShadowSystem: Created %u shadow maps (one per frame)", numFrames);

	// Create shadow pass (shares render pass from first shadow map) -
	shadowPass = new ShadowPass(vulkan, *shadowMaps[0]);
	Log(NOTE, "ShadowSystem: Shadow pass initialized");
}

ShadowSystem::~ShadowSystem()
{
	// Clean up shadow resources...
	if (shadowPass) {
		delete shadowPass;
		shadowPass = nullptr;
	}

	for (ShadowMap* shadowMap : shadowMaps) {
		delete shadowMap;
	}
	shadowMaps.clear();

	if (technique != SHADOW_TECHNIQUE_NONE) {
		Log(NOTE, "ShadowSystem: Resources destroyed");
	}
}

bool ShadowSystem::recordFrame(vector<iRenderable*>& renderables, uint32_t frameIndex)
{
	if (technique == SHADOW_TECHNIQUE_NONE) {
		return false;  // No-op, zero cost
	}

	// Record shadow pass for this frame:
	shadowPass->recordShadowPass(renderables, frameIndex, *shadowMaps[frameIndex]);
	return true;
}

VkImageView ShadowSystem::getShadowMapView(uint32_t frameIndex) const
{
	if (technique == SHADOW_TECHNIQUE_NONE || frameIndex >= shadowMaps.size()) {
		return VK_NULL_HANDLE;
	}
	return shadowMaps[frameIndex]->getImageView();
}

VkSampler ShadowSystem::getShadowMapSampler() const
{
	if (technique == SHADOW_TECHNIQUE_NONE || shadowMaps.empty()) {
		return VK_NULL_HANDLE;
	}
	// All shadow maps share the same sampler:
	return shadowMaps[0]->getSampler();
}

VkCommandBuffer ShadowSystem::getCommandBuffer(uint32_t frameIndex) const
{
	if (technique == SHADOW_TECHNIQUE_NONE || !shadowPass) {
		return VK_NULL_HANDLE;
	}
	return shadowPass->getCommandBuffer(frameIndex);
}

vector<VkDescriptorImageInfo> ShadowSystem::getPerFrameDescriptorInfo() const
{
	vector<VkDescriptorImageInfo> descriptors;

	if (technique == SHADOW_TECHNIQUE_NONE) {
		return descriptors;  // Empty vector
	}

	// Create descriptor info for each shadow map:
	for (uint32_t i = 0; i < shadowMaps.size(); ++i) {
		VkDescriptorImageInfo info = {
			.sampler = shadowMaps[i]->getSampler(),
			.imageView = shadowMaps[i]->getImageView(),
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
		};
		descriptors.push_back(info);
	}

	return descriptors;
}

VkRenderPass ShadowSystem::getRenderPass() const
{
	if (technique == SHADOW_TECHNIQUE_NONE || shadowMaps.empty()) {
		return VK_NULL_HANDLE;
	}
	// All shadow maps share the same render pass:
	return shadowMaps[0]->getRenderPass();
}

VkExtent2D ShadowSystem::getExtent() const
{
	if (technique == SHADOW_TECHNIQUE_NONE || shadowMaps.empty()) {
		return VkExtent2D{ 0, 0 };
	}
	return VkExtent2D{
		shadowMaps[0]->getWidth(),
		shadowMaps[0]->getHeight()
	};
}
