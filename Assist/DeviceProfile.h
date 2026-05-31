//
// DeviceProfile.h
//	Vulkan Device Assist
//
// Encapsulate evaluation of Vulkan-supportive Devices.
//
// An array of these instantiates in the construction of
//	DeviceAssessment, later released in its destructor
//	(in the spirit of, or mimicking, RAII).
// "ProfilesReference" type assists with that.
//
// 2/27/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DeviceProfile_h
#define DeviceProfile_h

#include "iPlatform.h"


typedef uint64_t  RawScore;


const VkFormat			VkFormat_UNSET			= (VkFormat) -1;
const VkColorSpaceKHR	VkColorSpaceKHR_UNSET	= (VkColorSpaceKHR) -1;
const VkPresentModeKHR	VkPresentModeKHR_UNSET	= (VkPresentModeKHR) -1;


struct DeviceProfile
{
	vector<StrPtr>	extensionNames;
	bool			lacksRequiredExtensions = false;
	VkPhysicalDeviceProperties	properties = {};
	VkSurfaceFormatKHR			selectedSurfaceFormat = { VkFormat_UNSET, VkColorSpaceKHR_UNSET };
	VkPresentModeKHR			selectedPresentMode = VkPresentModeKHR_UNSET;
	VkFormat					selectedDepthFormat = VK_FORMAT_UNDEFINED;
	string			description = "unset";
	// scoring:
	uint64_t		totalScore = 0;
	RawScore		rawPerPassScore = 0;
	RawScore		surfaceSupportScore = 0;
};

typedef DeviceProfile (&ProfilesReference)[];


#endif // DeviceProfile_h
