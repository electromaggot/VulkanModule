//
// EventObjects.h
//	Vulkan Components
//
// 4/20/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef EventObjects_h
#define EventObjects_h

#include "VulkanSetup.h"


/*typedef enum SyncEvent {
	AWAIT_PRIOR		= 0x00000001,
	TRIGGER_NEXT	= 0x00000002
} SyncEvent;*/


static VkPipelineStageFlagBits DEFAULT = (VkPipelineStageFlagBits) -1;


class Event {
public:
	Event(GraphicsDevice& graphics);
	~Event();

	void CmdSetRecordTo(VkCommandBuffer& commandBuffer, VkPipelineStageFlagBits stage = DEFAULT);
	void CmdWaitRecordTo(VkCommandBuffer& commandBuffer, VkPipelineStageFlagBits stage = DEFAULT);

	bool IsEventSet();

	void Set();
	void Reset();

private:
		// MEMBERS
	VkEvent		vkEvent;

	VkPipelineStageFlagBits	stageThatSetSignal;
	VkPipelineStageFlags	stageAtWhichToWait;

	VkDevice&	device;		// Save to destruct as constructed

		// METHODS
	void create();
};

#endif	// EventObjects_h
