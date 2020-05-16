//
// DeviceQueues.h
//	Vulkan Setup Module
//
// Encapsulate the Queue Families and Queues supported
//	by a Device for "display-to-screen" capability.
// Intended to be parented/owned/instanced by GraphicsDevice.
//
// The "suitability" analysis reflects how well a given device supports both Graphics and
//	"Present" operations in its queues (i.e. sets of identical queues, or each queue family),
//	particularly for one purpose here: rendering and presenting graphics on a display device.
//
//  Put simply, we're only interested in queues supporting both Graphics and Present; those
//	return a score of 2.  Others return 0.  Obviously it's up to caller to appropriately
//	scale these values to fit into a scoring scheme amongst other criteria.
//
// Note also for simplicity, only one queue is represented here.  If, for the given family,
//	multiple queues are needed (at arbitrary priorities), that can be added in the future
//	and is left somewhat flexible here.
//
// 2/2/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DeviceQueues_h
#define DeviceQueues_h

#include "VulkanPlatform.h"


#define INDEX_UNDEFINED		-1

enum QueueFitness {
	UNKNOWN			= -1,
	NOT_SUPPORTED	=  0,
	PARTIAL_SUPPORT	=  1,
	SUPPORTS_BOTH	=  2
};
static const char* stringFitness[] = {	"neither",
										"partial",
										"both"		};


class DeviceQueues {
public:
	DeviceQueues();

		// MEMBERS
private:
	static const int nIndices = 1;				// again, only one queue family is selected (in this implementation)
	union {
		uint32_t	familyIndex;				// selected (preferred) queue family             of the device
		uint32_t	familyIndices[nIndices];	//									 or families
	};
	union {
		VkQueue		currentQueue;				// same goes for the	queue itself
		VkQueue		queues[nIndices];			//					 / queues themselves
	};

	QueueFitness suitability;

public:
	VkDeviceQueueCreateInfo QueueCreateInfos[nIndices];	// (public so 'get' of sized-array is possible)

		// METHODS
public:
	QueueFitness DetermineFamilyIndex(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
	void GatherQueueHandlesFor(VkDevice& logicalDevice);
	void InitializeQueueCreateInfos();

private:
	void logSupportAnomaly(bool deviceSupportsGraphics, bool deviceSupportsPresent);

		// getters
public:
	VkQueue&	getCurrent()	{ return currentQueue;	}

	typedef uint32_t (&IndexArrayRef)[nIndices];

	IndexArrayRef getIndices()	{ return familyIndices; }

	uint32_t getFamilyIndex() {
		if (nIndices == 1)
			return familyIndex;
		for (int iIndex = 0; iIndex < nIndices; ++iIndex)
			if (familyIndices[iIndex] > INDEX_UNDEFINED)
				return familyIndices[iIndex];	// return first valid found
		Log(WARN, "Graphics Family Index accessed but UNDEFINED.");
		return INDEX_UNDEFINED;
	}

	// It happens that QueueFitness's enumeration values provide an appropriate
	//	"ranking" of device/queue suitability, where higher number is better.
	//
	int RankDeviceSuitability() {
		if (suitability == UNKNOWN) {
			Log(WARN, "Analyze device/queue suitability BEFORE requesting it.");
			return 0;
		}
		return (int) suitability;
	}

	const char* suitabilityString()	{ return stringFitness[suitability]; }
};

#endif // DeviceQueues_h
