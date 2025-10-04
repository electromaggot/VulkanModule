//
// PlatformSDL.h
//	General App Chassis
//
// Platform-specific Concretion for Simple DirectMedia Layer.
//
// Feel free to delete the base class/interface if you'll only ever use SDL.
//	An abstraction with one concretion is unnecessary cruft; simpler is better!
//
// TODO: Add multi-monitor (multi-window) support, eventually.
//
// Created 2/7/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef PlatformSDL_h
#define PlatformSDL_h

// Avoid SDL's many warnings: Empty paragraph passed to '\param' command
#pragma clang diagnostic ignored "-Wdocumentation"
#include <string.h>		// <--(otherwise 'memcpy' may warn "use of undeclared identifier" in SDL_stdinc.h)
#include <SDL.h>
#include <SDL_vulkan.h>

#include "iPlatform.h"
#include "ImageSDL.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"


class PlatformSDL : public iPlatform
{
public:
	PlatformSDL();
	~PlatformSDL();

		// MEMBERS
private:
	SDL_Window*	pWindow;	// Note: only supports a single window, hence single monitor.

	SDL_Event	event;

	ImageSDL	image;

	typedef void (*PFNResizeForceRender)(void* pObject);
	PFNResizeForceRender pfnResizeForceRender = nullptr;
	void* pRenderingObject = nullptr;

	int		conveyPress	= 0;
	Uint32	timePress	= 0;
	const Uint32 MILLISECONDS_LONG_PRESS = 300;		// about 1/3rd second, arbitrary

public:
	int LastSavedPixelsWide = 0;
	int	LastSavedPixelsHigh = 0;

	#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR		// TODO: Add ANDROID support
		const bool IsMobile = true;
	#else
		const bool IsMobile = false;
	#endif

		// METHODS
	void CreateVulkanSurface(VkInstance instance, VkSurfaceKHR& surface);
	bool GetWindowSize(int& pixelWidth, int& pixelHeight);
	void SetWindowTitle(const char* title);
	void DialogBox(const char* message, const char* title = "ERROR", AlertLevel level = FAILURE);
	bool PollEvent(iControlScheme* pControl = nullptr);
	bool IsEventQUIT();
	void AwaitEvent();
	void ClearEvents();
	void ShowSoftKeyboard(bool show = true);
	int  WasSimplePress();

	iImageSource& ImageSource()	 { return static_cast<iImageSource&>(image); }
	void InitGUISystem()		 { ImGui_ImplSDL2_InitForVulkan(pWindow); }
	void GUISystemNewFrame()	 { ImGui_ImplSDL2_NewFrame(pWindow); }
	void GUISystemProcessEvent(SDL_Event* pEvent)
								 { ImGui_ImplSDL2_ProcessEvent(pEvent); }
	void RegisterForceRenderCallback(PFNResizeForceRender pfnForceRender, void* pObject) {
									pfnResizeForceRender = pfnForceRender;
									pRenderingObject = pObject;
								 }
private:
	void initializeSDL();
	void createVulkanCompatibleWindow();
	void querySupportedVulkanExtensions();
	void recordWindowGeometry();
	void rememberWindowSize(int wide, int high);
	void recordWindowPosition(int x, int y);
	float getDisplayScaling();
	float getDisplayDPI(int iDisplay = 0);

	void process(SDL_WindowEvent& windowEvent);

	// WIP, TODO: "more officially" integrate later
	void createMultiMonitorWindows();
	void createVulkanSurface(int iScreen, VkInstance instance, VkSurfaceKHR& surface);
	void destroyMultiMonitorWindows();

	static int realtimeResizingEventWatcher(void* data, SDL_Event* event);
};

#endif	// PlatformSDL_h
