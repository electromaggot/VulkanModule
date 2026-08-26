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

#include "ResourceTracker.h"
#include "imgui.h"	// for WantCaptureMouse and WantCaptureKeyboard flags
#include <climits>	// (to build on Linux side)
#include <cstdlib>	// for setenv on macOS
#ifdef __APPLE__
 #include <unistd.h>	// for access()
#endif


// Sanity bounds for geometry restored from saved settings: reject values that could only come
//	from a corrupt settings file or a since-disconnected display, falling back to the config's
//	default window size.  Deliberately NOT application-configurable -- an app has no basis on
//	which to choose a number here, and nothing but this validation reads them.
//
static const int MAX_SANE_SCREEN_WIDTH	= 7680 * 2;		// 8K x 2: a range check, not a limit
static const int MAX_SANE_SCREEN_HEIGHT	= 4320 * 2;		//	we ever expect to approach.

// Baseline "100%" display density, which a measured DPI is divided by to yield a scale factor.
//	A platform convention rather than an application choice -- Apple's baseline is 72 DPI --
//	so it lives here now, having formerly come from each app's PlatformConstants.h.
//
static const float DEFAULT_DOTS_PER_INCH = 72.0f;


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
#ifdef __APPLE__
	// Ensure the Vulkan loader can find MoltenVK. Shell env vars often contain
	// stale versioned Homebrew Cellar paths that break after `brew upgrade`.
	// Only set if not already pointing to a valid file (don't override user choice).
	auto ensureICD = [](const char* envVar) {
		const char* current = getenv(envVar);
		if (current && access(current, F_OK) == 0)
			return;
		static const char* candidates[] = {
			"/opt/homebrew/share/vulkan/icd.d/MoltenVK_icd.json",
			"/opt/homebrew/etc/vulkan/icd.d/MoltenVK_icd.json",
			"/usr/local/share/vulkan/icd.d/MoltenVK_icd.json",
			"/usr/local/etc/vulkan/icd.d/MoltenVK_icd.json",
		};
		for (auto path : candidates) {
			if (access(path, F_OK) == 0) {
				setenv(envVar, path, 1);
				return;
			}
		}
	};
	ensureICD("VK_ICD_FILENAMES");
	ensureICD("VK_DRIVER_FILES");
#endif

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
		Fatal("Fail to Initialize SDL: " + string(SDL_GetError()));

	char hintHomeIndicator[2] = { DEFAULT_HOME_INDICATOR_APPEARANCE, '\0' };
	SDL_SetHint(SDL_HINT_IOS_HIDE_HOME_INDICATOR, hintHomeIndicator);

#ifdef __APPLE__
	// Vsync is NOT configured here.  MoltenVK derives CAMetalLayer.displaySyncEnabled from the
	//	Vulkan present mode we select at swapchain creation, so PRESENT_MODE_PRECEDENCE
	//	(VulkanConfigure.h) is the single place that decides paced-vs-unpaced.  Setting it here
	//	too would just be a second, conflicting opinion.
	// (Removed 4-Aug-2026: SDL_HINT_RENDER_VSYNC / SDL_HINT_RENDER_SCALE_QUALITY.  Those govern
	//	SDL's 2D SDL_Renderer, which a Vulkan app never creates -- they were inert, and their
	//	"vsync disabled" wording misrepresented what the app was actually doing.)

	setenv("MVK_CONFIG_DISPLAY_WATERMARK", "0", 0);				// No debug watermark.
	setenv("MVK_CONFIG_SYNCHRONOUS_QUEUE_SUBMITS", "0", 0);		// Async queue submits (throughput).

	Log(NOTE, "macOS: MoltenVK configured; frame pacing follows the selected Vulkan present mode.");
#endif
}

// Refresh rate (Hz) of the display this window is on; 0 if it can't be determined.
//
int PlatformSDL::GetDisplayRefreshHz() const
{
	if (!pWindow)
		return 0;
	int iDisplay = SDL_GetWindowDisplayIndex(pWindow);
	if (iDisplay < 0)
		return 0;
	SDL_DisplayMode mode;
	if (SDL_GetCurrentDisplayMode(iDisplay, &mode) != 0)
		return 0;
	return mode.refresh_rate;		// SDL reports 0 when the rate is unspecified/variable.
}

// Create a vulkan window; fatal throw on failure.
//
void PlatformSDL::createVulkanCompatibleWindow()
{
	WindowGeometry stored;			// Defaults (isStored false) if the app persists nothing,
	if (iAppSettings* pSettings = Settings())	//	in which case the fallbacks below apply.
		stored = pSettings->GetWindowGeometry();

	int winWide = 0, winHigh = 0, winX = INT_MIN, winY = INT_MIN;
	if (stored.isStored) {
		winWide = stored.width;
		winHigh = stored.height;
		winX = stored.x;
		winY = stored.y;
	}

	// Validate window position across ALL displays (multi-monitor support)
	int numDisplays = SDL_GetNumVideoDisplays();
	bool windowOnValidDisplay = false;
	const int MIN_VISIBLE_PIXELS = 50;  // Minimum pixels of title bar that must be visible

	// Check each display to see if window is visible on any of them
	for (int i = 0; i < numDisplays; ++i) {
		SDL_Rect displayBounds;
		if (SDL_GetDisplayBounds(i, &displayBounds) == 0) {
			// Check if window's title bar is accessible on this display
			bool titleBarVisible = (winX + MIN_VISIBLE_PIXELS >= displayBounds.x &&  // Not too far left
									winX <= displayBounds.x + displayBounds.w - MIN_VISIBLE_PIXELS &&  // Not too far right
									winY >= displayBounds.y &&  // Title bar not above screen
									winY <= displayBounds.y + displayBounds.h - MIN_VISIBLE_PIXELS);  // Not too far down

			if (titleBarVisible) {
				windowOnValidDisplay = true;

				// Validate window size against this display
				if (winWide <= 0 || winWide > displayBounds.w)
					winWide  = Config().defaultWindowWidth;
				if (winHigh <= 0 || winHigh > displayBounds.h)
					winHigh = Config().defaultWindowHeight;

				// Ensure window size doesn't exceed this display's bounds
				if (winWide > displayBounds.w)
					winWide = displayBounds.w;
				if (winHigh > displayBounds.h)
					winHigh = displayBounds.h;

				Log(LOW, "Window validated on display %d: pos(%d, %d) size(%dx%d)", i, winX, winY, winWide, winHigh);
				break;  // Found valid display, stop checking
			}
		}
	}

	// If window is not visible on ANY display, center it on primary display
	if (!windowOnValidDisplay) {
		Log(LOW, "Window position (%d, %d) not visible on any display, centering on primary", winX, winY);
		winX = SDL_WINDOWPOS_CENTERED;
		winY = SDL_WINDOWPOS_CENTERED;

		// Validate size against primary display
		SDL_Rect primaryBounds;
		if (SDL_GetDisplayBounds(0, &primaryBounds) == 0) {
			if (winWide <= 0 || winWide > primaryBounds.w)
				winWide  = Config().defaultWindowWidth;
			if (winHigh <= 0 || winHigh > primaryBounds.h)
				winHigh = Config().defaultWindowHeight;
		} else {
			// Fallback if SDL functions fail - use conservative defaults
			if (winWide <= 0 || winWide > MAX_SANE_SCREEN_WIDTH)
				winWide  = Config().defaultWindowWidth;
			if (winHigh <= 0 || winHigh > MAX_SANE_SCREEN_HEIGHT)
				winHigh = Config().defaultWindowHeight;
		}
	}

	// Final fallback for invalid positions
	if (winX == INT_MIN || winY == INT_MIN) {
		// Fallback to original validation
		if (winWide <= 0 || winWide > MAX_SANE_SCREEN_WIDTH)
			winWide  = Config().defaultWindowWidth;
		if (winHigh <= 0 || winHigh > MAX_SANE_SCREEN_HEIGHT)
			winHigh = Config().defaultWindowHeight;
		if (winX < -winWide || winX > MAX_SANE_SCREEN_WIDTH)
			winX = SDL_WINDOWPOS_CENTERED;
		if (winY < -winHigh || winY > MAX_SANE_SCREEN_HEIGHT)
			winY = SDL_WINDOWPOS_CENTERED;
	}

	int windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN
					| SDL_WINDOW_RESIZABLE	// not just for desktop windows, also applies to mobile device orientation changes
					| (IsMobile ? SDL_WINDOW_FULLSCREEN : 0);

	pWindow = SDL_CreateWindow(Config().windowTitle, winX, winY, winWide, winHigh, windowFlags);
	if (!pWindow)
		Fatal("Fail to Create Vulkan-compatible Window with SDL: " + string(SDL_GetError()));

	// Log display information for performance diagnostics.
	int displayIndex = SDL_GetWindowDisplayIndex(pWindow);
	if (displayIndex >= 0) {
		const char* displayName = SDL_GetDisplayName(displayIndex);
		SDL_DisplayMode displayMode;
		if (SDL_GetCurrentDisplayMode(displayIndex, &displayMode) == 0) {
			Log(NOTE, "Window on display %d: \"%s\" ( %d × %d @ %dHz )",
				displayIndex,
				displayName ? displayName : "Unknown",
				displayMode.w, displayMode.h, displayMode.refresh_rate);

			// Helpful performance hint based on display type:
			if (displayIndex > 0) {
				Log(WARN, "External display detected - FPS may be limited by display connection/vsync.");
			} else {
				Log(GOOD, "Built-in display - expect maximum performance.");
			}
		}
	}

	pixelsWide = LastSavedPixelsWide = winWide;
	pixelsHigh = LastSavedPixelsHigh = winHigh;
	windowX = winX;
	windowY = winY;

	if (stored.isFullScreen && !IsMobile)
		pendingFullScreen = true;		// defer until run loop is active (macOS requires it)

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
	if (SDL_Vulkan_CreateSurface(screenInfos[iScreen].pWindow, instance, &surface)) {
		VK_TRACK_CREATE(VK_RESOURCE_SURFACE);	// SDL creates surface internally, manually track it.
		return;
	}
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
	if (SDL_Vulkan_CreateSurface(pWindow, instance, &surface)) {
		VK_TRACK_CREATE(VK_RESOURCE_SURFACE);	// SDL creates surface internally, manually track it.
		return;
	}
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

void PlatformSDL::SetWindowTitle(const char* title)
{
	if (pWindow)
		SDL_SetWindowTitle(pWindow, title);
}

// Detect fullscreen: SDL flags (SDL-initiated) or macOS native (green button).
//	SDL2 doesn't track macOS native fullscreen with its own flags, and on notched
//	MacBooks the window size is identical in windowed-maximized and native fullscreen,
//	so query the NSWindow styleMask directly via a platform-specific helper.
//
#if __APPLE__ && ! TARGET_OS_IPHONE
	extern "C" bool macOS_IsNativeFullScreen(SDL_Window* pWindow);
	extern "C" void macOS_ExitNativeFullScreen(SDL_Window* pWindow);
	// NOTE: Must include "MacOSFullScreen.mm" in Xcode project for these to resolve.
#endif

bool PlatformSDL::detectFullScreen()
{
	if (SDL_GetWindowFlags(pWindow) & SDL_WINDOW_FULLSCREEN)
		return true;

#if __APPLE__ && ! TARGET_OS_IPHONE
	return macOS_IsNativeFullScreen(pWindow);
#else
	return false;
#endif
}

void PlatformSDL::ExitFullScreen()
{
	if (SDL_GetWindowFlags(pWindow) & SDL_WINDOW_FULLSCREEN)
		SDL_SetWindowFullscreen(pWindow, 0);
#if __APPLE__ && ! TARGET_OS_IPHONE
	else if (macOS_IsNativeFullScreen(pWindow))
		macOS_ExitNativeFullScreen(pWindow);
#endif
}

void PlatformSDL::recordWindowGeometry() // (with logging too)
{
	Log(SAME, "Note: Save Window Geometry: ");

	isFullScreen = detectFullScreen();

	iAppSettings* pSettings = Settings();
	if (!pSettings) {					// App persists nothing, so there is nowhere to record it.
		Log(RAW, "SKIPPED (this app stores no settings).");
		return;
	}
	WindowGeometry geometry = pSettings->GetWindowGeometry();
	geometry.isFullScreen = isFullScreen;
	if (!isFullScreen) {				// While fullscreen, retain the last WINDOWED geometry,
		geometry.width  = pixelsWide;	//	so leaving fullscreen restores that size/position.
		geometry.height = pixelsHigh;
		geometry.x = windowX;
		geometry.y = windowY;
	}
	geometry.isStored = true;
	pSettings->SetWindowGeometry(geometry);
	pSettings->Save();
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
	return getDisplayDPI() / DEFAULT_DOTS_PER_INCH;
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
		// Pass events to ImGui first; it needs to see ALL events.	// Note: When IMGUI_DISABLE - in GUISystem/imgui.h -
		GUISystemProcessEvent(&event);								//	stub implementations provide no-op.
		//printf("event %d : %d\n", event.type, event.window.event);

		ImGuiIO& io = ImGui::GetIO();				 // Get ImGui's input capture flags AFTER it processes the event,
		bool imguiWantsMouse = io.WantCaptureMouse;	 // although if IMGUI_DISABLE, stub always returns false (no mouse capture).

		switch (event.type) {
			case SDL_WINDOWEVENT:
				process(event.window);
				break;
			case SDL_MOUSEMOTION:
				mouseX = event.motion.x;	// Always update mouse position (passive tracking).
				mouseY = event.motion.y;
				if (pController && !imguiWantsMouse) {		// Only pass to controller if...
					pController->handlePrimaryPressAndDrag(mouseX, mouseY);
					pController->handleSecondaryPressAndDrag(mouseX, mouseY);
					pController->handleTertiaryPressAndDrag(mouseX, mouseY);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (pController) {
					if (!imguiWantsMouse)					//	...if ImGui doesn't want mouse...
						switch (event.button.button)
						{
						case SDL_BUTTON_LEFT:
							pController->handlePrimaryPressDown(event.button.x, event.button.y);
							break;
						case SDL_BUTTON_MIDDLE:
							pController->handleTertiaryPressDown(event.button.x, event.button.y);
							break;
						case SDL_BUTTON_RIGHT:
							pController->handleSecondaryPressDown(event.button.x, event.button.y);
							break;
						}
				} else
					timePress = event.button.timestamp;
				break;
			case SDL_MOUSEBUTTONUP:
				if (pController) {
					if (!imguiWantsMouse)					//	...and same here...
						switch (event.button.button)
						{
						case SDL_BUTTON_LEFT:
							pController->handlePrimaryPressUp(event.button.x, event.button.y);
							break;
						case SDL_BUTTON_MIDDLE:
							pController->handleTertiaryPressUp(event.button.x, event.button.y);
							break;
						case SDL_BUTTON_RIGHT:
							pController->handleSecondaryPressUp(event.button.x, event.button.y);
							break;
						}
				} else if (event.button.timestamp - timePress < MILLISECONDS_LONG_PRESS)
					conveyPress = event.button.button;
				break;
			case SDL_MOUSEWHEEL:
				if (pController && !imguiWantsMouse)		//	...and here too.
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
				// ESC (unshifted) exits fullscreen mode back to windowed.
				if (event.key.keysym.sym == SDLK_ESCAPE
						&& !(event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT))
						&& detectFullScreen())
					SDL_SetWindowFullscreen(pWindow, 0);

				#if TARGET_OS_IOS
					// iOS soft keyboard seems to immediately follow KEYDOWN with KEYUP, which
					//	confuses ImGui's attempt to track KeysDownDuration between those events.
					//	Result: BACKSPACE doesn't seem to work on iOS.  This tries to hack it:
					ImGuiIO& io = ImGui::GetIO();							// (note that for IMGUI_DISABLE, stub
					int keyMapBackspace = io.KeyMap[ImGuiKey_Backspace];	//	provides no-op/harmless assignments)
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

	// Event queue empty -- run loop is active, safe for deferred operations.

	if (pendingFullScreen) {					// Restore fullscreen saved from prior session,
		pendingFullScreen = false;				//	or switch from native to borderless fullscreen.
		SDL_SetWindowFullscreen(pWindow,			//	Deferred: macOS requires the run loop to be active.
								SDL_WINDOW_FULLSCREEN_DESKTOP);
	}

	// Detect fullscreen state changes (covers both SDL-initiated and macOS native).
	bool currentlyFullScreen = detectFullScreen();
	if (currentlyFullScreen != isFullScreen) {
		isFullScreen = currentlyFullScreen;
		Log(LOW, "Fullscreen state change: %s", isFullScreen ? "ENTER" : "EXIT");

#if __APPLE__ && ! TARGET_OS_IPHONE
		// If the user entered native fullscreen (green button), switch to borderless
		//	fullscreen instead -- it's consistent (ESC exits) and doesn't show a menu
		//	bar on mouse hover.  Exit native first, then apply borderless on next idle.
		if (isFullScreen && !(SDL_GetWindowFlags(pWindow) & SDL_WINDOW_FULLSCREEN)) {
			macOS_ExitNativeFullScreen(pWindow);
			pendingFullScreen = true;
		} else
#endif
		if (!IsMobile)
			recordWindowGeometry();
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
		case SDL_WINDOWEVENT_RESTORED:
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
	// Quit on: SDL_QUIT, Shift+ESC, or window close
	// (ESC alone is reserved for canceling operations, e.g. edit operation in editor mode.)
	bool isShiftEscape = ( event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE
						&& (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) );

	return (event.type == SDL_QUIT || isShiftEscape
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
