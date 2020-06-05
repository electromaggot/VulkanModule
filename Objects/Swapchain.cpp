//
// Swapchain.cpp
//	Vulkan Setup
//
// See matched header file for definitive main comment.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "Swapchain.h"
#include "VulkanSingleton.h"
#include "Helpers.h"


Swapchain::Swapchain(GraphicsDevice& graphics, WindowSurface& surface)
	:	device(graphics), windowSurface(surface)
{
	create(windowSurface.getVkSurface());

	createImageViews();
}

Swapchain::~Swapchain()  { destroy(); }

void Swapchain::destroy()
{
	VkDevice& logicalDevice = device.getLogical();

	for (auto imageView : imageViews) {
		vkDestroyImageView(logicalDevice, imageView, nullALLOC);
	}
	vkDestroySwapchainKHR(logicalDevice, swapchain, nullALLOC);
}


void Swapchain::create(VkSurfaceKHR& surface)
{
	VkSurfaceCapabilitiesKHR capabilities;
	call = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getGPU(), surface, &capabilities);

	if (call != VK_SUCCESS)
		Fatal("Get Physical Device Surface Capabilities FAILURE" + ErrStr(call));

	DeviceProfile selectedDevice = device.getProfile();
	VkSurfaceFormatKHR surfaceFormat = selectedDevice.selectedSurfaceFormat;
	VkPresentModeKHR presentMode = selectedDevice.selectedPresentMode;
	uint32_t nImages = determineImageCount(capabilities);
	extent = determineSwapExtent(capabilities);

	DeviceQueues::IndexArrayRef queueFamilyIndices = device.Queues.getIndices();

	VkSwapchainCreateInfoKHR createInfo = {
		.sType	= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext	= nullptr,
		.flags	= 0,
		.surface		 = surface,

		.minImageCount	 = nImages,
		.imageFormat	 = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent	 = extent,
		.imageArrayLayers = AppConstants.SupportStereo3D ? (uint32_t) 2 : 1,
		.imageUsage		 = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,			// (best performance, same family)
		.queueFamilyIndexCount	= N_ELEMENTS_IN_ARRAY(queueFamilyIndices),
		.pQueueFamilyIndices	= queueFamilyIndices,

		.preTransform	 = capabilities.currentTransform,		// (i.e. no transform)
		.compositeAlpha	 = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,	// (ignore alpha, no blending)
		.presentMode	 = presentMode,
		.clipped		 = VK_TRUE,

		.oldSwapchain	 = VK_NULL_HANDLE
	};

	call = vkCreateSwapchainKHR(device.getLogical(), &createInfo, nullALLOC, &swapchain);

	if (call != VK_SUCCESS)
		Fatal("Create Swapchain FAILURE" + ErrStr(call));
}

// 'Get Swapchain Images' to 'Create Image View's.
//
void Swapchain::createImageViews()
{
	VkDevice& logicalDevice = device.getLogical();
	VkFormat  imageFormat = device.getProfile().selectedSurfaceFormat.format;

	uint32_t nImages;
	call = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &nImages, nullptr);
	if (call != VK_SUCCESS || nImages == 0)
		Fatal("Get Swapchain nImages " + to_string(nImages) + ErrStr(call));

	VkImage images[nImages];
	call = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &nImages, images);
	if (call != VK_SUCCESS)
		Fatal("Get Swapchain Images FAILURE" + ErrStr(call));

	imageViews.resize(nImages);

	VkImageViewCreateInfo createInfo = {
		.sType	= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.viewType	= VK_IMAGE_VIEW_TYPE_2D,
		.format		= imageFormat,
		.components = {
			.r	= VK_COMPONENT_SWIZZLE_IDENTITY,
			.g	= VK_COMPONENT_SWIZZLE_IDENTITY,
			.b	= VK_COMPONENT_SWIZZLE_IDENTITY,
			.a	= VK_COMPONENT_SWIZZLE_IDENTITY
		},
		.subresourceRange = {
			.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel	= 0,
			.levelCount		= 1,
			.baseArrayLayer	= 0,
			.layerCount	 = AppConstants.SupportStereo3D ? (uint32_t) 2 : 1
		}
	};

	while (nImages-- > 0)
	{
		createInfo.image = images[nImages];

		call = vkCreateImageView(logicalDevice, &createInfo, nullALLOC, &imageViews[nImages]);

		if (call != VK_SUCCESS)
			Log(ERROR, "Create Image View #" + to_string(nImages) + " format " + to_string(imageFormat) + ErrStr(call));
	}
}


// Set dimensions of render destination surface.
// Vulkan returning uint32_t::max as a capabilities.currentExtent indicates
//	that we are free to choose our own, within the min/minImageExtent range.
//
VkExtent2D Swapchain::determineSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	const uint32_t INVALID = numeric_limits<uint32_t>::max();

	// Favor getting Extent ourselves first...
	VkExtent2D windowExtent = windowSurface.GetWindowExtent();	// Assign from platform, a perhaps-resized size.

	if (windowExtent.width == INVALID || windowExtent.height == INVALID) {	// If still not a valid extent,
		if (capabilities.currentExtent.width != INVALID && capabilities.currentExtent.height != INVALID)
			return capabilities.currentExtent;								//	secondarily try capabilities or
		windowExtent = { static_cast<uint32_t>(AppConstants.DefaultWindowWidth),	//	last-ditch assign one
						 static_cast<uint32_t>(AppConstants.DefaultWindowHeight) };	//	from default values.
	}
	clamp(windowExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	clamp(windowExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return windowExtent;
}

// +1 to .minImageCount per: https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Creating-the-swap-chain
// And note that maxImageCount 0 means that there is no maximum.
//
uint32_t Swapchain::determineImageCount(const VkSurfaceCapabilitiesKHR& capabilities)
{
	uint32_t nImages = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && nImages > capabilities.maxImageCount)
		nImages = capabilities.maxImageCount;
	return nImages;
}

void Swapchain::Recreate()
{
	destroy();
	create(windowSurface.getVkSurface());
	createImageViews();
}

