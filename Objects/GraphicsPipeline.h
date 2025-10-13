//
// GraphicsPipeline.h
//	Vulkan Setup
//
// Encapsulate the Pipeline.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at whim)
//
#ifndef GraphicsPipeline_h
#define GraphicsPipeline_h

#include "GraphicsDevice.h"
#include "ShaderModules.h"
#include "Swapchain.h"
#include "RenderPass.h"
#include "VertexAbstract.h"
#include "Descriptors.h"
#include "Customizer.h"


class GraphicsPipeline
{
public:
	GraphicsPipeline(ShaderModules& shaders, RenderPass& renderPass,
					 Swapchain& swapchain, GraphicsDevice& graphics,
					 VertexAbstract* pVertex = nullptr, Descriptors* pDescriptors = nullptr,
					 Customizer customize = NONE);

	// Overload for custom VkRenderPass (e.g., shadow mapping)
	GraphicsPipeline(ShaderModules& shaders, VkRenderPass vkRenderPass,
					 Swapchain& swapchain, GraphicsDevice& graphics,
					 VertexAbstract* pVertex = nullptr, Descriptors* pDescriptors = nullptr,
					 Customizer customize = NONE, VkExtent2D customExtent = {0, 0});

	~GraphicsPipeline();

		// MEMBERS
private:
	VkPipelineLayout pipelineLayout;
	VkPipeline		 graphicsPipeline;

	VkDevice		 device;

		// METHODS
private:
	void create(ShaderModules& shaderModules, VertexAbstract* pVertex, VkExtent2D swapchainExtent,
				RenderPass& renderPass, Descriptors* pDescriptors, Customizer customize);
	void create(ShaderModules& shaderModules, VertexAbstract* pVertex, VkExtent2D swapchainExtent,
				VkRenderPass vkRenderPass, bool useDepthBuffer, bool hasColorAttachment,
				Descriptors* pDescriptors, Customizer customize);
	void destroy();
public:
	void Recreate(ShaderModules& shaderModules, RenderPass& renderPass, Swapchain& swapchain,
				  VertexAbstract* pVertex = nullptr, Descriptors* pDescriptors = nullptr,
				  Customizer customize = NONE);
		// getters
	VkPipeline&  getVkPipeline()			{ return graphicsPipeline;	}
	VkPipelineLayout&  getPipelineLayout()	{ return pipelineLayout;	}
};

#endif // GraphicsPipeline_h
