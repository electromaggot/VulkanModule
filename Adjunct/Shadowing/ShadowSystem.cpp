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
						   ShadowTechnique tech, uint32_t resolution,
						   ShadowProjectionMode projMode, ShadowCameraMode camMode)
	:	technique(tech)
		, projectionMode(projMode)
		, cameraMode(camMode)
		, resolution(resolution)
		, shadowPass(nullptr)
{
	if (technique == SHADOW_TECHNIQUE_NONE) {	// No resources allocated - zero VRAM cost.
		Log(NOTE, "ShadowSystem: Shadows DISABLED (zero VRAM allocation)");
		return;
	}
	createResources(vulkan, numFrames);
}

ShadowSystem::~ShadowSystem()
{
	destroyGpuResources();
	if (technique != SHADOW_TECHNIQUE_NONE)
		Log(DEAD, "ShadowSystem: Resources destroyed");
}

// Create the per-frame shadow maps + shadow pass.  Shared by the ctor and recreateGpuResources().
void ShadowSystem::createResources(VulkanSetup& vulkan, uint32_t numFrames)
{
	Log(NOTE, "ShadowSystem: Creating shadow resources with technique=%d, resolution=%ux%u",
		technique, resolution, resolution);

	for (uint32_t i = 0; i < numFrames; ++i) {	// One shadow map per frame (no cross-frame races).
		ShadowMap* shadowMap = new ShadowMap(vulkan.device, vulkan.command.getCommandPool(),
											  resolution, resolution);
		shadowMaps.push_back(shadowMap);
	}
	shadowPass = new ShadowPass(vulkan, *shadowMaps[0]);	// Shares render pass from first shadow map.
	Log(NOTE, "ShadowSystem: Created %u shadow maps + shadow pass", numFrames);
}

void ShadowSystem::destroyGpuResources()
{
	if (shadowPass) {
		delete shadowPass;
		shadowPass = nullptr;
	}
	for (ShadowMap* shadowMap : shadowMaps)
		delete shadowMap;
	shadowMaps.clear();
}

void ShadowSystem::recreateGpuResources(VulkanSetup& vulkan, uint32_t numFrames)
{
	if (technique == SHADOW_TECHNIQUE_NONE)		// Disabled — nothing to recreate.
		return;
	destroyGpuResources();						// Idempotent (handles already-destroyed case).
	createResources(vulkan, numFrames);
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

iRenderPass* ShadowSystem::getpRenderPass() const
{
	if (technique == SHADOW_TECHNIQUE_NONE || shadowMaps.empty()) {
		return nullptr;
	}
	// All shadow maps share the same render pass:
	return shadowMaps[0]->getpRenderPass();
}

VkRenderPass ShadowSystem::getVkRenderPass() const
{
	if (technique == SHADOW_TECHNIQUE_NONE || shadowMaps.empty()) {
		return VK_NULL_HANDLE;
	}
	// All shadow maps share the same render pass:
	return shadowMaps[0]->getVkRenderPass();
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
