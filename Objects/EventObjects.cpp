//
// EventObjects.cpp
//	Vulkan Components
//
// See header file comment for overview.
//
// 4/20/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "EventObjects.h"
#include "VulkanSetup.h"

VkPipelineStageFlagBits DEFAULT_INTRA_STAGE = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;


Event::Event(GraphicsDevice& graphics)
	:	device(graphics.getLogical())
{
	create();

	stageThatSetSignal = DEFAULT_INTRA_STAGE;
}

Event::~Event()
{
	vkDestroyEvent(device, vkEvent, nullALLOC);
}


void Event::CmdSetRecordTo(VkCommandBuffer& commandBuffer, VkPipelineStageFlagBits stage)
{
	if (stage != DEFAULT)
		stageThatSetSignal = stage;

	vkCmdSetEvent(commandBuffer, vkEvent, stageThatSetSignal);
}

void Event::CmdWaitRecordTo(VkCommandBuffer& commandBuffer, VkPipelineStageFlagBits stage)
{
	if (stage != DEFAULT)
		stageAtWhichToWait = stage;
	else
		stageAtWhichToWait = stageThatSetSignal;

stageAtWhichToWait = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

	vkCmdWaitEvents(commandBuffer,	1, &vkEvent,	// 1 VkEvent
						stageThatSetSignal,
						stageAtWhichToWait,
							0, nullptr,		// 0 VkMemoryBarriers
							0, nullptr,		// 0 VkBufferMemoryBarriers
							0, nullptr		// 0 VkImageMemoryBarriers
				   );
}


bool Event::IsEventSet() {
	return vkGetEventStatus(device, vkEvent) == VK_EVENT_SET;
}

void Event::Set() {
	vkSetEvent(device, vkEvent);
}
void Event::Reset() {
	vkResetEvent(device, vkEvent);
}


void Event::create()
{
	VkEventCreateInfo createInfo = {
		.sType	= VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0
	};

	call = vkCreateEvent(device, &createInfo, nullALLOC, &vkEvent);

	if (call != VK_SUCCESS)
		Fatal("Create Event FAILURE" + ErrStr(call));
}
