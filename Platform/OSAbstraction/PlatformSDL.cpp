//
// PlatformSDL.cpp
//	General App Chassis
//
// See matched header file for definitive main comment.
//
// The platform may return references to resources, even e.g. pointer to a string.
// In such cases, a reference is assumed to persist (not be weak or scope-dependent)
//	and its resource freed intelligently.  For example, strings may reside in an
//	executable's fixed data.  If otherwise allocated, Apple uses ARC (automatic
//	reference counting), Windows via managed objects and garbage collection, so
//	ensure the platform layer (if applicable) is built to invoke them.
//
// Created 2/7/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "PlatformSDL.h"

#include "AppConstants.h"
#include <climits>	// (to build on Linux side)


PlatformSDL::PlatformSDL()
{
	namePlatform = "SDL";

	initializeSDL();

	createVulkanCompatibleWindow();

	querySupportedVulkanExtensions();

	displayFoundVulkanExtensions();

	SDL_StartTextInput();
}

PlatformSDL::~PlatformSDL()
{
	delete supportedVulkanExtensions;

	SDL_DestroyWindow(pWindow);

	SDL_Quit();
}

// INITIALIZATION ----------------------------------------------------------------------------------

enum HomeIndicatorAppearance : char {	// The indicator bar is:
	Hidden = '1',	// - only visible for short while when screen is tapped or device is rotated.
	Dimmed = '2'	// - dark, first swipe makes it visible, second swipe performs the "home" action (default for fullscreen applications)
};					//		(although not seeing this behavior as default! despite FULLSCREEN).
const char DEFAULT_HOME_INDICATOR_APPEARANCE = Hidden;


// Start SDL.
//
void PlatformSDL::initializeSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
		Fatal("Fail to Initialize SDL: " + string(SDL_GetError()));

	char hintHomeIndicator[2] = { DEFAULT_HOME_INDICATOR_APPEARANCE, '\0' };
	SDL_SetHint(SDL_HINT_IOS_HIDE_HOME_INDICATOR, hintHomeIndicator);
}

// Create a vulkan window; fatal throw on failure.
//
void PlatformSDL::createVulkanCompatibleWindow()
{
	AppSettings& settings = AppConstants.Settings;
	int winWide = 0, winHigh = 0, winX = INT_MIN, winY = INT_MIN;
	if (settings.isInitialized) {
		winWide = settings.startingWindowWidth;
		winHigh = settings.startingWindowHeight;
		winX = settings.startingWindowX;
		winY = settings.startingWindowY;
	}
	if (winWide <= 0 || winWide > AppConstants.MaxSaneScreenWidth)		// Do not allow…
		winWide  = AppConstants.DefaultWindowWidth;
	if (winHigh <= 0 || winHigh > AppConstants.MaxSaneScreenHeight)		//	…window to…
		winHigh = AppConstants.DefaultWindowHeight;
	if (winX < -winWide || winX > AppConstants.MaxSaneScreenWidth)		//	…be entirely…
		winX = SDL_WINDOWPOS_CENTERED;
	if (winY < -winHigh || winY > AppConstants.MaxSaneScreenHeight)		//	…off-screen.
		winY = SDL_WINDOWPOS_CENTERED;

	int windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN
					| SDL_WINDOW_RESIZABLE	// not just for desktop windows, also applies to mobile device orientation changes
					| (IsMobile ? SDL_WINDOW_FULLSCREEN : 0);

	pWindow = SDL_CreateWindow(AppConstants.WindowTitle, winX, winY, winWide, winHigh, windowFlags);
	if (!pWindow)
		Fatal("Fail to Create Vulkan-compatible Window with SDL: " + string(SDL_GetError()));

	pixelsWide = LastSavedPixelsWide = winWide;
	pixelsHigh = LastSavedPixelsHigh = winHigh;
	windowX = winX;
	windowY = winY;

	//recordWindowGeometry();		// re-saves anything that had to be "corrected" above
	//No, on 2nd thought, won't.  If re-run, will re-assign the same way.  If user tweaks, then it will save.
	//(plus, the above seems to wipe out the saved credentials, which need to be pulled from Vault if that's to happen)

//	if (! isMobilePlatform)		// we don't need this for full-screen or mobile (where called unnecessarily on device rotation)
		SDL_AddEventWatch(realtimeResizingEventWatcher, this);
//TODO: had commented the above test, but not documented why. Later, justify resizeEventWatcher on mobile...
//		   was it to actually catch and process the device rotation?
}

// Finer-granularity callbacks as Window border is grabbed & dragged, making rendering
//	calls to update window content as it is resized.
//
int PlatformSDL::realtimeResizingEventWatcher(void* data, SDL_Event* event)
{
	if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED)
	{
		PlatformSDL* pSelf = (PlatformSDL*) data;
		SDL_Window* pEventWindow = SDL_GetWindowFromID(event->window.windowID);
		if (pEventWindow == pSelf->pWindow)
		{
			SDL_GetWindowSize(pEventWindow, &pSelf->pixelsWide, &pSelf->pixelsHigh);

			pSelf->isWindowResized = true;
			// Beware if not properly synchronized with rendering, may crash in vkQueueSubmit() with, e.g.:
			// -[MTLDebugRenderCommandEncoder setScissorRect:]:2702: failed assertion `(rect.x(0) + rect.width(627))(627) must be <= render pass width(623)'

			if (pSelf->pfnResizeForceRender)
				pSelf->pfnResizeForceRender(pSelf->pRenderingObject);

			Log(LOW, "Resize %d x %d... force render.", pSelf->pixelsWide, pSelf->pixelsHigh);
		}
	}
	return 0;
}

// Work In Progress ... MULTI-MONITOR SUPPORT
//	These methods seem to work, but:
//	TODO: needs method to specify using: windowed vs. full-screen vs. how many full-screen-displays
//		obviously this needs combined with the isMobilePlatform stuff above
//		note that SDL_WINDOW_BORDERLESS may need to supersede SDL_WINDOW_FULLSCREEN for multimon to work
//
struct ScreenInfo {
	SDL_Window*		pWindow;
	SDL_Rect		bounds;
	SDL_Renderer*	pRenderer;
};
int nScreens = 0;
ScreenInfo* screenInfos;
void PlatformSDL::createMultiMonitorWindows()
{
	nScreens = SDL_GetNumVideoDisplays();
	screenInfos = new ScreenInfo[nScreens];
	for (int iScreen = 0; iScreen < nScreens; ++iScreen) {
		ScreenInfo& screen = screenInfos[iScreen];
		SDL_GetDisplayBounds(iScreen, &screen.bounds);
		char title[] = "Display 0";
		title[sizeof(title) - 2] = '1' + iScreen;
		screen.pWindow = SDL_CreateWindow(title, screen.bounds.x, screen.bounds.y,
										  screen.bounds.w, screen.bounds.h, SDL_WINDOW_BORDERLESS);
		screen.pRenderer = SDL_CreateRenderer(screen.pWindow, 0, SDL_RENDERER_ACCELERATED);
		SDL_ShowWindow(screen.pWindow);
	}
}
void PlatformSDL::createVulkanSurface(int iScreen, VkInstance instance, VkSurfaceKHR& surface)
{
	if (SDL_Vulkan_CreateSurface(screenInfos[iScreen].pWindow, instance, &surface))
		return;
	Fatal("Unable to Create Vulkan-compatible Surface using SDL: " + string(SDL_GetError()));
}
void PlatformSDL::destroyMultiMonitorWindows()
{
	for (int iScreen = 0; iScreen < nScreens; ++iScreen) {
		ScreenInfo& screen = screenInfos[iScreen];
		SDL_DestroyRenderer(screen.pRenderer);
		SDL_DestroyWindow(screen.pWindow);
	}
	delete screenInfos;
}
//
// End WIP

// Ask SDL/windowing-system/OS what Vulkan extensions it supports; non-fatally debug-prints success or failure.
//	Expect the default VK_KHR_surface to be returned, accompanied by any platform-specific extensions.
//
void PlatformSDL::querySupportedVulkanExtensions()
{
	ArrayCount nExtensionsReturnedBySDL = 0;
	if (!SDL_Vulkan_GetInstanceExtensions(pWindow, &nExtensionsReturnedBySDL, nullptr))
		Log(ERROR, "Fail to query NUMBER of Vulkan Instance Extensions: " + string(SDL_GetError()));

	nAdditionalExtensions = extendedPlatform.nRequestedExtensionsAvailable();
	nVulkanExtensions = nExtensionsReturnedBySDL + nAdditionalExtensions;

	supportedVulkanExtensions = new StrPtr[nVulkanExtensions];	// Actually get the extensions themselves:
	if (!SDL_Vulkan_GetInstanceExtensions(pWindow, &nExtensionsReturnedBySDL, supportedVulkanExtensions))
		Log(ERROR, "Fail querying " + to_string(nVulkanExtensions) + " Vulkan Instance Extension names: " + string(SDL_GetError()));

	for (Index iAdd = 0, iConcat = nExtensionsReturnedBySDL; iAdd < nAdditionalExtensions && iConcat < nVulkanExtensions; ++iAdd, ++iConcat)
		supportedVulkanExtensions[iConcat] = extendedPlatform.requestedExtensionNames()[iAdd];
}

// Create a Vulkan-compatible surface using the platform layer, throwing fatally on failure.
// Multi-monitor is not supported, as there's but a single SDL_Window pointer.
//
void PlatformSDL::CreateVulkanSurface(VkInstance instance, VkSurfaceKHR& surface)
{
	if (SDL_Vulkan_CreateSurface(pWindow, instance, &surface))
		return;
	Fatal("Unable to Create Vulkan-compatible Surface using SDL: " + string(SDL_GetError()));
}

// Return parameter-pointers set to pixel dimensions of the window or display, or zeros on
//	error (pointed-to ints' values will be destroyed).
//	SDL_GetWindowSize is not a favored method; Apple platforms with SDL_WINDOW_ALLOW_HIGHDPI
//	return rescaled "screen coordinates" not pixels, although it's used as a last resort.
//	For details: https://wiki.libsdl.org/SDL_GetWindowSize
//
bool PlatformSDL::GetWindowSize(int& pixelWidth, int& pixelHeight)
{
	string error;

	pixelWidth = pixelHeight = 0;

	auto pRenderer = SDL_GetRenderer(pWindow);
	if (pRenderer) {
		int result = SDL_GetRendererOutputSize(pRenderer, &pixelWidth, &pixelHeight);
		if (result == 0)  // success
			return true;
		error = "Renderer Output Size returned error";
	} else
		error = "Renderer undefined";
	string sdlError = SDL_GetError();
	if (sdlError != "")
		error += ": " + sdlError;

	// see if possible to return valid values from screen coordinates
	SDL_GetWindowSize(pWindow, &pixelWidth, &pixelHeight);	// (doesn't return a result)
	if (pixelWidth > 0 && pixelHeight > 0) {
		float scaling = getDisplayScaling();
		if (scaling > 0.0f) {
			pixelWidth *= (int) scaling;
			pixelHeight *= (int) scaling;
		}
		return true;
	}
	error += ", Get Window Size returned " + to_string(pixelWidth) + " x " + to_string(pixelHeight);
	sdlError = SDL_GetError();
	if (sdlError != "")
		error += ", " + sdlError;
	Log(WARN, error);
	return false;
}

void PlatformSDL::recordWindowGeometry() // (with logging too)
{
	Log(SAME, "Note: Save Window Geometry: ");

	AppSettings& settings = AppConstants.Settings;
	settings.startingWindowWidth  = pixelsWide;
	settings.startingWindowHeight = pixelsHigh;
	settings.startingWindowX = windowX;
	settings.startingWindowY = windowY;
	settings.Save();
}
void PlatformSDL::rememberWindowSize(int wide, int high)
{
	if (LastSavedPixelsWide != wide || LastSavedPixelsHigh != high) {
		LastSavedPixelsWide = wide;
		LastSavedPixelsHigh = high;
		if (! IsMobile)					// On desktop, (doesn't make sense on mobile),
			recordWindowGeometry();		//	want this to save resized window size to Settings file.
	}
}
void PlatformSDL::recordWindowPosition(int x, int y)
{
	if (windowX != x || windowY != y) {
		windowX = x;
		windowY = y;
		recordWindowGeometry();
	}
}

float PlatformSDL::getDisplayScaling()
{
	auto noHiDPI = SDL_GetHintBoolean(SDL_HINT_VIDEO_HIGHDPI_DISABLED, SDL_TRUE);
	if (noHiDPI == SDL_TRUE)
		return 0.0f;
	return getDisplayDPI() / PlatformConstants.DefaultDotsPerInch;
}
//TJ_ADVISORY_NOTE_(DELETE_THIS_AFTER_FURTHER_INVESTIGATION)
// This "Scaling" hasn't been observed to actually work.  Typically pRenderer is NULL, then
//	GetWindowSize returns what look like valid pixel dimensions, yet the scaling value returned
//	is e.g. ~ 1.8, so the dimensions grow big, seeming inaccurate.  The only saving grace is
//	Swapchain.determinSwapExtent 'clamp'ing the values back down to within its 'max' range,
//	which appears to make them "usable" again, whence processing continues normally.
//	It's probably just that I still don't fully understand Apple's SDL_WINDOW_ALLOW_HIGHDPI
//	(in SDL_WindowFlags) but I will... I will have to, eventually.

// Thanks to: https://nlguillemot.wordpress.com/2016/12/11/high-dpi-rendering/
//	Avoid diagonal DPI and don't bother overcomplicating if vertical doesn't match horizontal.
//	(also for Apple .plist note: https://wiki.libsdl.org/SDL_HINT_VIDEO_HIGHDPI_DISABLED )
//
float PlatformSDL::getDisplayDPI(int iDisplay)
{
	float horizontalDPI = 0.0f;		//diagonal				 //vertical
	if (SDL_GetDisplayDPI(iDisplay, nullptr, &horizontalDPI, nullptr) == 0)
		return horizontalDPI;
	return 0.0f;	// indicates error
}


// Inform user via pop-up window.  Defaults to "ERROR" condition (for title and level, see header).
//
void PlatformSDL::DialogBox(const char* message, const char* title, AlertLevel level)
{
	SDL_MessageBoxFlags levels[] = { SDL_MESSAGEBOX_ERROR, SDL_MESSAGEBOX_WARNING, SDL_MESSAGEBOX_INFORMATION };

	SDL_ShowSimpleMessageBox(levels[level], title, message, pWindow);
}

// RUNTIME -----------------------------------------------------------------------------------------

// Polling is core to SDL; using SDL_PollEvent gives best responsiveness/performance with
//	one unified IO/Render thread, but even with a multi-threaded/synchronized arrangement.
//	(Note that SDL_Wait blocks indefinitely without returning (seizes rendering) while
//	 SDL_WaitTimeout blocks at a minimum interval of one millisecond, which: is still too
//	 coarse (possibly causing a task-switch), leads to stuttering in rendering or immediate
//	 user-input to visual-output response.  Thus neither Wait method is suitable.)
//
bool PlatformSDL::PollEvent(iControlScheme* pController)
{
	if (SDL_PollEvent(&event))
	{
		GUISystemProcessEvent(&event);
		//printf("event %d : %d\n", event.type, event.window.event);

		switch (event.type) {
			case SDL_WINDOWEVENT:
				process(event.window);
				break;
			case SDL_MOUSEMOTION:
				mouseX = event.motion.x;	// handle mouse input passively
				mouseY = event.motion.y;
				if (pController) {			// handle actively
					pController->handlePrimaryPressAndDrag(mouseX, mouseY);
					pController->handleSecondaryPressAndDrag(mouseX, mouseY);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (pController)
					switch (event.button.button)
					{
					case SDL_BUTTON_LEFT:
						pController->handlePrimaryPressDown(event.button.x, event.button.y);
						break;
					case SDL_BUTTON_RIGHT:
						pController->handleSecondaryPressDown(event.button.x, event.button.y);
						break;
					}
				else
					timePress = event.button.timestamp;
				break;
			case SDL_MOUSEBUTTONUP:
				if (pController)
					switch (event.button.button)
					{
					case SDL_BUTTON_LEFT:
						pController->handlePrimaryPressUp(event.button.x, event.button.y);
						break;
					case SDL_BUTTON_RIGHT:
						pController->handleSecondaryPressUp(event.button.x, event.button.y);
						break;
					}
				else if (event.button.timestamp - timePress < MILLISECONDS_LONG_PRESS)
					conveyPress = event.button.button;
				break;
			case SDL_MOUSEWHEEL:
				if (pController)
					pController->handleMouseWheel(event.wheel.x, event.wheel.y);
				break;
			case SDL_MULTIGESTURE:
				// FYI https://skia.googlesource.com/third_party/sdl/+/master/docs/README-gesture.md
				//		file://../../../3rdParty/SDL/SDL/include/SDL_events.h
				if (pController && event.mgesture.numFingers == 2)
				{
					pController->handlePinchSpread(event.mgesture.dDist);
					pController->handleTwoFingerTwist(event.mgesture.dTheta);
				}
				break;
			case SDL_KEYUP: {
				#if TARGET_OS_IOS
					// iOS soft keyboard seems to immediately follow KEYDOWN with KEYUP, which
					//	confuses ImGui's attempt to track KeysDownDuration between those events.
					//	Result: BACKSPACE doesn't seem to work on iOS.  This tries to hack it:
					ImGuiIO& io = ImGui::GetIO();
					int keyMapBackspace = io.KeyMap[ImGuiKey_Backspace];
					io.KeysDown[keyMapBackspace] = (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE);
					//	...same seems to apply to RETURN key as well.
					io.KeysDown[io.KeyMap[ImGuiKey_Enter]] = (event.key.keysym.scancode == SDL_SCANCODE_RETURN);
					// Refer to sister code in main project GUI/Reusables: MayNeedSoftKeyboard()
				#endif
				} break;
			default:
				break;
		}
		return true;
	}
	return false;
}

void PlatformSDL::process(SDL_WindowEvent& windowEvent)
{
	static int windowMovedToY, windowMovedToX = INT_MIN;

	switch (windowEvent.event) {
		case SDL_WINDOWEVENT_SIZE_CHANGED:	// (and ignoring SDL_WINDOWEVENT_RESIZED, see DEV NOTE at bottom)
			rememberWindowSize(event.window.data1, event.window.data2);
			isWindowResized = true;			// (note this remains set until retrieved, whence one-shot resets it)
			Log(LOW, "      Window Resized %d x %d", pixelsWide, pixelsHigh);	// show resize is finished
			break;
		case SDL_WINDOWEVENT_MOVED:
			windowMovedToX = event.window.data1;
			windowMovedToY = event.window.data2;
			Log(LOW, "      Window Move %d, %d...", windowMovedToX, windowMovedToY);
			return;
		case SDL_WINDOWEVENT_MINIMIZED:
		case SDL_WINDOWEVENT_HIDDEN:
			isWindowHidden = true;
			break;
		case SDL_WINDOWEVENT_SHOWN:
		case SDL_WINDOWEVENT_EXPOSED:
			isWindowHidden = false;
			break;
		case SDL_WINDOWEVENT_MAXIMIZED:
		case SDL_WINDOWEVENT_RESTORED: // (from being maximized)
			isWindowResized = true;
			isWindowHidden = false;
			break;
	}
	if (windowMovedToX != INT_MIN) { // Current event was not WINDOW _MOVED, but this indicates that previous event was,
		recordWindowPosition(windowMovedToX, windowMovedToY);	//	therefore "end of dragging around window," so save its position.
		windowMovedToX = INT_MIN;
	}
}

bool PlatformSDL::IsEventQUIT()
{
	return (event.type == SDL_QUIT
		|| (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
		|| (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE));
}

void PlatformSDL::AwaitEvent()
{
	SDL_WaitEvent(nullptr);
}

void PlatformSDL::ClearEvents()		// This seems good to do prior to main loop for SDL, especially
{									//	since it may unnecessarily start with SDL_WINDOWEVENT_RESIZED.
	while (SDL_PollEvent(&event));
}

// Most-basic tap/click, not requiring ControlScheme, convenient for imprecise, light interaction, e.g. a demo.
//
int PlatformSDL::WasSimplePress()								// See SDL_mouse.h for return values, e.g.:
{									// Upon read,								// SDL_BUTTON_LEFT		1
	bool press = conveyPress;		//	actas as "one shot,"					// SDL_BUTTON_MIDDLE	2
	conveyPress = 0;				//	resetting value.						// SDL_BUTTON_RIGHT		3
	return press;
}


void PlatformSDL::ShowSoftKeyboard(bool show)
{
	if (show) {
		if (! SDL_IsTextInputActive())
			SDL_StartTextInput();
	} else {
		if (SDL_IsTextInputActive())
			SDL_StopTextInput();
	}
}


/* DEVELOPER NOTE:

SDL_WINDOWEVENT_RESIZED vs. SDL_WINDOWEVENT_SIZE_CHANGED
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 https://wiki.libsdl.org/SDL_WindowEventID says:
 SDL_WINDOWEVENT_RESIZED : window has been resized to data1xdata2; this event is always preceded by SDL_WINDOWEVENT_SIZE_CHANGED.
 SDL_WINDOWEVENT_SIZE_CHANGED : window size has changed, either as a result of an API call or through the system or user changing
								the window size; this event is followed by SDL_WINDOWEVENT_RESIZED if the size was changed by an
								external event, i.e. the user or the window manager.
These speak for themselves.																	  Although, does _RESIZED exclusively
But to reiterate:  Since _RESIZED is *always preceded* by _SIZE_CHANGED, it's not necessary.			 deliver Width x Height ?
That is, only SDL_WINDOWEVENT_SIZE_CHANGED is really necessary to process, especially for our needs.
*/
