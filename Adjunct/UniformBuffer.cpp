//
// UniformBuffer.cpp
//	Vulkan Add-ons
//
// See header description.
//
// Note that herein:
//	numBuffers = numSwapchainImages;
//
// Created 6/14/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "UniformBuffer.h"


UniformBuffer::UniformBuffer(int bytesizeUniformBufferObject, Swapchain& swapchain,
							 								  GraphicsDevice& device)
	:	BufferBase(device),
		numBuffers(static_cast<uint32_t>(swapchain.getImageViews().size())),
		nbytesBufferObject(bytesizeUniformBufferObject)
{
	create();
}

UniformBuffer::~UniformBuffer()
{
	destroy();
}

void UniformBuffer::destroy()
{
	for (int iBuffer = 0; iBuffer < numBuffers; ++iBuffer) {
		vkDestroyBuffer(device, uniformBuffers[iBuffer], nullALLOC);
		vkFreeMemory(device, uniformBuffersMemory[iBuffer], nullALLOC);
	}
}


void UniformBuffer::create()
{
	uniformBuffers.resize(numBuffers);
	uniformBuffersMemory.resize(numBuffers);

	for (int iBuffer = 0; iBuffer < numBuffers; ++iBuffer)
		createGeneralBuffer(nbytesBufferObject, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							uniformBuffers[iBuffer], uniformBuffersMemory[iBuffer]);
}


void UniformBuffer::Update(int indexCurrentImage, void* pUBO, size_t nbytesUBO)
{
	void* data;
	vkMapMemory(device, uniformBuffersMemory[indexCurrentImage], 0, nbytesUBO, 0, &data);
	if (call != VK_SUCCESS)
		Fatal("Uniform Buffer Map Memory FAILURE" + ErrStr(call));	// as for this being fatal, see (**) Dev Note in BufferBase.h

	memcpy(data, pUBO, nbytesUBO);
	vkUnmapMemory(device, uniformBuffersMemory[indexCurrentImage]);
}


void UniformBuffer::Recreate(int bytesizeUniformBufferObject, Swapchain& swapchain)
{
	if (bytesizeUniformBufferObject >= 0)	// otherwise keep same sized buffer as before
		nbytesBufferObject = bytesizeUniformBufferObject;
	numBuffers = static_cast<uint32_t>(swapchain.getImageViews().size());

	destroy();
	create();
}


/* DEV NOTE - Two bugs that hid each other, and why per-frame buffers now actually work...
  This class allocates one buffer per swapchain image so that writing the CPU side for the next
	frame cannot disturb a buffer the GPU is still reading for a frame in flight.  Until refactor
	that intent was defeated twice over, and each defect concealed the other:
	  1. getDescriptorBufferInfo() took no argument and always returned uniformBuffers[0].  It's
		 called once, while descriptors are being built, so EVERY frame's descriptor set pointed
		 at buffer 0.  Buffers 1..n-1 were dutifully written by Update() and never read.
	  2. CommandBufferSet::recordCommands() passed `iBuffer` - its loop index over that set's own
		 VkCommandBuffers - as the renderables' frame index.  Each set was allocated exactly one
		 command buffer (CommandControl::PostInitPrepBuffers), so that index was always 0, and
		 every frame therefore bound descriptor set 0.
  Fixing either alone changed nothing observable, which was the trap: correct the descriptor and
	you still only ever bind set 0; correct the index and every set still names buffer 0.  Only
	together did they yield per-frame uniforms.  A half-fix looked like a wrong theory.
  The visible cost beforehand: the GPU always read buffer 0, which Update() refreshed only on
	frames where the acquired image index happened to be 0 - one frame in three on a triple-
	buffered swapchain.  UBO-driven motion therefore advanced at a third of the frame rate, and
	buffer 0 could be rewritten while an in-flight frame was still reading it: precisely the
	hazard the per-frame allocation exists to avoid.  The same stale index also made
	SecondaryRenderable replay frame 0's secondary command buffer on every frame.
*/
