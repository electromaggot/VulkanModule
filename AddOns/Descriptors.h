//
// Descriptors.h
//	Vulkan Add-ons
//
// "DESCRIBE" to Vulkan: add-ons like a Uniform Buffer Object or
//	a Texture Image object.  Descriptors include Pool, Sets, Layout.
//
// Created 6/14/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef Descriptors_h
#define Descriptors_h

#include "BufferBase.h"
#include "Swapchain.h"


enum GeneralVkDescriptorType {
	BUFFER	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	TEXTURE = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
};


struct DescribEd {	// Pronounced "describe-ed" meaning: "the thing being described" - by a Descriptor.
									// Specifically, these are objects bound (via binding) to a shader.
	GeneralVkDescriptorType	type;
	union {
		VkDescriptorBufferInfo bufferInfo;
		VkDescriptorImageInfo  imageInfo;
	};
	VkShaderStageFlags		stage;

	DescribEd(VkDescriptorBufferInfo bufinf, VkShaderStageFlags flags)
		:	type(BUFFER), bufferInfo(bufinf), stage(flags)			{ }
	DescribEd(VkDescriptorImageInfo imginf, VkShaderStageFlags flags)
		:	type(TEXTURE), imageInfo(imginf), stage(flags)			{ }
	VkDescriptorType	getDescriptorType()		{ return (VkDescriptorType) type;	}
	VkShaderStageFlags	getShaderStageFlags()	{ return stage;						}
};



class Descriptors : BufferBase
{
public:
	Descriptors(vector<DescribEd>& describeds, Swapchain& swapchain, GraphicsDevice& device);
	~Descriptors();

		// MEMBERS
private:
	vector<DescribEd>		describers;

	uint32_t				numBuffers;	 // will == numSwapchainImages !

	VkDescriptorPool		descriptorPool;
	vector<VkDescriptorSet>	descriptorSets;

	VkDescriptorSetLayout	descriptorSetLayout;

		// METHODS
private:
	void create();
	void destroy();
	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();
public:
	void Recreate(vector<DescribEd> descriptions, Swapchain& swapchain);

		// getters
	VkDescriptorSetLayout*		getpLayout()  { return &descriptorSetLayout; }
	vector<VkDescriptorSet>&	getSets()	  { return descriptorSets;		 }
	VkDescriptorPool&			getPool()	  { return descriptorPool;		 }

	bool						exist()		  { return ! descriptorSets.empty(); }
};

#endif // Descriptors_h
