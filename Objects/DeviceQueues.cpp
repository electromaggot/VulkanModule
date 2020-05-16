//
// DeviceQueues.cpp
//	Vulkan Setup Module
//
// See header file for main comment.
//
// 2/2/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "DeviceQueues.h"


DeviceQueues::DeviceQueues()
	:	familyIndex(INDEX_UNDEFINED),
		suitability(UNKNOWN)
{
	//for (int iIndex = 0; iIndex < nIndices; ++iIndex)  familyIndices[iIndex] = INDEX_UNDEFINED;
}

// For the given physical device (and display surface pair) query its Queue Families
//	for the specific one that best supports Graphics commands and Present operation.
// Return assessment of how "suitable" the device, with selected "best fit" queue/family,
//	for our most typical purpose: rendering an image & presenting it to a display device.
//
QueueFitness DeviceQueues::DetermineFamilyIndex(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface)
{
	suitability = NOT_SUPPORTED;

	bool deviceSupportsGraphics	= false;
	bool deviceSupportsPresent	= false;

	uint32_t nFamilies = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &nFamilies, nullptr);

	if (nFamilies == 0)
		Log(ERROR, "Physical Device supports NO Queue Families.");
	else {
		VkQueueFamilyProperties familyProperties[nFamilies];
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &nFamilies, familyProperties);

		for (int iFamily = 0; iFamily < nFamilies; ++iFamily)
		{
			VkQueueFamilyProperties& family = familyProperties[iFamily];

			if (family.queueCount > 0)
			{
				bool familySupportsGraphics = (family.queueFlags & VK_QUEUE_GRAPHICS_BIT);
				if (familySupportsGraphics)
					deviceSupportsGraphics = true;

				VkBool32 familySupportsPresent = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, iFamily, surface, &familySupportsPresent);
				if (familySupportsPresent)
					deviceSupportsPresent = true;

				if (familySupportsGraphics && familySupportsPresent) {	// Ideally one queue family supporting both
					familyIndex = iFamily;								//	Graphics and Present is preferred,
					suitability = SUPPORTS_BOTH;
					return suitability;									//	so if found, quit early successfully.
				}
			}
		}
		// Note that purposely, if only one "familySupports..." is found, this will *not* return a
		//	suitability of PARTIAL_SUPPORT, because partial support isn't actually desirable at all.
	}
	logSupportAnomaly(deviceSupportsGraphics, deviceSupportsPresent);
	return suitability;
}

// It's helpful to know if a device provides one but not the other, i.e. either Graphics-but-not-Present
//	or Present-but-not-Graphics, especially if somehow the device otherwise does not SUPPORTS_BOTH.
//
void DeviceQueues::logSupportAnomaly(bool deviceSupportsGraphics, bool deviceSupportsPresent)
{
	if (suitability == UNKNOWN)
		Log(ERROR, "Call to DetermineFamilyIndex() never made.");
	else if (suitability < SUPPORTS_BOTH)
	{
		char allNames[] = "Graphics, Present";
		char* setNames = allNames;
		if (deviceSupportsGraphics)	setNames = &allNames[10];
		if (deviceSupportsPresent)	allNames[8] = '\0';
		Log(ERROR, "Command Queue/Family not supported: %s", setNames);
	}
	if (QueueCreateInfos[0].sType != VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO)
		Log(ERROR, "QueueCreateInfos[] look uninitialized, call InitializeQueueCreateInfos() first.");
}

void DeviceQueues::InitializeQueueCreateInfos()
{
	float queuePriority = 1.0f;

	for (int iQueue = 0; iQueue < nIndices; ++iQueue)
	{
		QueueCreateInfos[iQueue] = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = familyIndices[iQueue],
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};
	}
}

void DeviceQueues::GatherQueueHandlesFor(VkDevice& logicalDevice)
{
	vkGetDeviceQueue(logicalDevice, familyIndex, 0, &currentQueue);

	//for (int iIndexQueue = 0; iIndexQueue < nIndices; ++iIndexQueue)
	//	vkGetDeviceQueue(logicalDevice, familyIndices[iIndexQueue], 0, &queues[iIndexQueue]);
}
