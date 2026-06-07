//
// Swapchain.h
//	Vulkan Setup
//
// Encapsulate Swapchain and curate its Extent.
//	Also parents ImageViews!
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef Swapchain_h
#define Swapchain_h

#include "GraphicsDevice.h"


using std::numeric_limits; using std::min; using std::max;


class Swapchain
{
public:
	Swapchain(GraphicsDevice& graphics, WindowSurface& windowSurface);
	~Swapchain();

		// MEMBERS
private:
	VkSwapchainKHR		swapchain;

	VkExtent2D			extent;

	vector<VkImageView>	imageViews;

	GraphicsDevice&		device;
	WindowSurface&		windowSurface;

		// METHODS
private:
	void	create(VkSurfaceKHR& surface);
	void	createImageViews();

	VkExtent2D	determineSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	uint32_t	determineImageCount(const VkSurfaceCapabilitiesKHR& capabilities);
public:
	void	Recreate();
	void	destroy();		// Public for device-loss teardown; idempotent (nulls handles).

		// getters
	vector<VkImageView>& getImageViews()	{ return imageViews; }
	int					 getNumImages()		{ return (int) imageViews.size(); }
	VkExtent2D&			 getExtent()		{ return extent; }

	VkSwapchainKHR&		 getVkSwapchain()	{ return swapchain; }
};

#endif // Swapchain_h
