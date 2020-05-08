Vulkan/Platform
---------------

This platform-specific code is bundled with the VulkanModule for convenience.

While in theory (and spirit) the Vulkan code should stand completely independent of
platform, actual practice requires the two to be coupled.  The renderer is simply
too dependent upon I/O:

Output is most important.  To display anything, on any platform, Vulkan must know
the capabilities of the device, such as its resolution or color capabilities.  On 
desktops, this could involve a window that's resized or moved to another display.
For mobile, the renderer should handle screen rotations, or at least,
issues like reconstructing the swapchain if the app is unsuspended.

Input is important too.  Most apps actually want to do something interactive, like
change camera angle per a user-based "control scheme."  There may be other elements
in play too, like a GUI system that obviously depends on user input.  At the very
least, the user needs a way to direct the app to close in an orderly fashion.
