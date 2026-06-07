//
// GraphicsDevice.h
//	Vulkan Setup
//
// Abstract the graphics card/GPU.
//
// Nomenclature note - meaning of
//	prefixes to variable names:
//		n = "number of" items in collection, or size/count
//		i = "index of" item in array, vector, or similar collection
//		is = boolean prefix, also: did  ...perhaps other verb
//	additional conventions:
//		rr or call = "return result" from a function call
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DeviceAbstract_h
#define DeviceAbstract_h

#include "VulkanInstance.h"
#include "ValidationLayers.h"
#include "WindowSurface.h"
#include "DeviceQueues.h"
#include "DeviceAssessment.h"


class GraphicsDevice
{
private:
	GraphicsDevice();
public:
	GraphicsDevice(WindowSurface& surface, VulkanInstance& vulkan, ValidationLayers& validation);
	~GraphicsDevice();

	DeviceQueues&		Queues;				// each device's queues/families

		// MEMBERS
private:
	VkDevice			logicalDevice;

	VkPhysicalDevice	physicalDevice;		// which GPU is SELECTED
	DeviceProfile		selected;
	string				reason;				// and why!

	DeviceQueues		queueFamilies;

		// METHODS
public:
	// Two-phase device-loss recovery (sleep/wake): DestroyLogicalDevice() at WillSleep while the
	//	device is still valid (after all children are destroyed), createLogicalDevice() at DidWake.
	void createLogicalDevice(ValidationLayers& validation);
	void DestroyLogicalDevice();

private:
	void determineDeviceExtensionSupport(VkPhysicalDevice* devices, int nDevices, DeviceAssessment& assays);

	VkPhysicalDevice selectGPU(VkInstance& instance, VkSurfaceKHR& surface,
							   DeviceSelectionMethod choice = DEVICE_SELECTION_MODE);
	VkPhysicalDevice manuallySelectGPU(VkInstance& instance, VkSurfaceKHR& surface);
	int consolePromptUserToChooseDevice(int nDevices);

	int pickBestDevice(VkPhysicalDevice devices[], int nDevices, DeviceAssessment& deviceAssess);
	int pickSpecifiedDevice(DeviceAssessment& deviceAssess, int nDevices);

public:
	// Device-loss recovery: destroy the lost logical device and recreate it in place (the same
	//	GraphicsDevice object, a fresh VkDevice handle in `logicalDevice`).  physicalDevice, queue
	//	family indices, and selected extensions all survive device loss, so this just re-runs
	//	createLogicalDevice().  References returned by getLogical() track the reassigned member.
	void RecreateLogicalDevice(ValidationLayers& validation);

	bool IsImageFormatSupported(VkFormat format, VkImageTiling tiling = (VkImageTiling) -1);

		// getters
	VkPhysicalDevice&	getGPU()		{ return physicalDevice;	  }
	VkDevice&			getLogical()	{ return logicalDevice;		  }
	DeviceProfile&		getProfile()	{ return selected;			  }
};

#endif // DeviceAbstract_h
