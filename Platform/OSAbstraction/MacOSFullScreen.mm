//
// MacOSFullScreen.mm
//	Platform-specific helpers for macOS native fullscreen.
//
// SDL2 does not track macOS native fullscreen (triggered by the green
//	title-bar button) with its own flags, and on notched MacBooks the
//	window size is identical in windowed-maximized and native fullscreen.
// These helpers query/control the NSWindow directly via SDL_GetWindowWMInfo.
//
// Created February 2026 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifdef __APPLE__

#include <SDL.h>
#include <SDL_syswm.h>
#import <Cocoa/Cocoa.h>


static NSWindow* getNSWindow(SDL_Window* pWindow)
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);

	if (SDL_GetWindowWMInfo(pWindow, &info) && info.subsystem == SDL_SYSWM_COCOA)
		return info.info.cocoa.window;
	return nil;
}

extern "C" bool macOS_IsNativeFullScreen(SDL_Window* pWindow)
{
	NSWindow* nswindow = getNSWindow(pWindow);
	if (nswindow)
		return ([nswindow styleMask] & NSWindowStyleMaskFullScreen) != 0;
	return false;
}

extern "C" void macOS_ExitNativeFullScreen(SDL_Window* pWindow)
{
	NSWindow* nswindow = getNSWindow(pWindow);
	if (nswindow && ([nswindow styleMask] & NSWindowStyleMaskFullScreen))
		[nswindow toggleFullScreen: nil];
}

// Device-loss recovery: register for system sleep/wake notifications and forward them to the app.
//	WillSleep fires while the GPU is still valid (the safe moment to tear down GPU resources); DidWake
//	fires after wake (rebuild).  Handlers are invoked synchronously on the main thread (during SDL's
//	event pump), so the app can do its Vulkan teardown before the machine actually sleeps.
typedef void (*SleepWakeFn)(void* context);
static SleepWakeFn s_onWillSleep = nullptr;
static SleepWakeFn s_onDidWake	 = nullptr;
static void*	   s_sleepWakeContext = nullptr;

extern "C" void macOS_InstallSleepWakeHandlers(SleepWakeFn onWillSleep, SleepWakeFn onDidWake, void* context)
{
	s_onWillSleep		= onWillSleep;
	s_onDidWake			= onDidWake;
	s_sleepWakeContext	= context;

	NSNotificationCenter* center = [[NSWorkspace sharedWorkspace] notificationCenter];

	[center addObserverForName: NSWorkspaceWillSleepNotification
					   object: nil
						queue: nil
				   usingBlock: ^(NSNotification* note) {
		(void) note;	// WillSleep fires while the GPU/VkDevice is still valid — the safe teardown moment.
		if (s_onWillSleep) s_onWillSleep(s_sleepWakeContext);
	}];

	[center addObserverForName: NSWorkspaceDidWakeNotification
					   object: nil
						queue: nil
				   usingBlock: ^(NSNotification* note) {
		(void) note;	// DidWake fires after the GPU reset — rebuild the device and its resources here.
		if (s_onDidWake) s_onDidWake(s_sleepWakeContext);
	}];
}

// Sustained-performance assertion.
//
// macOS idles the CPU/GPU down when the user isn't interacting — and it judges "interacting" by
//	INPUT (pointer movement, keys), not by what's animating on screen.  For a music visualizer
//	that is precisely backwards: the viewer sits still and watches, so the machine throttles at
//	exactly the moment smoothness matters.  Measured here: ~120fps untouched vs a full 144fps
//	while the mouse was being moved anywhere on the system, focused or not.
//
// NSActivityLatencyCritical is Apple's documented flag for "timing-sensitive media playback";
//	NSActivityUserInitiated additionally prevents App Nap.  Held only while it's needed — the
//	assertion has a real power cost, so it is NOT something to leave on permanently.
//
// Idempotent: repeated calls with the same state do nothing.
//
// ⚠️ THIS FILE IS BUILT BOTH WITH AND WITHOUT ARC — the Xcode target enables it
//	(CLANG_ENABLE_OBJC_ARC), the CMake build does not — so memory management here must compile
//	under either.  beginActivityWithOptions:reason: returns an AUTORELEASED token that has to
//	outlive the frame, which ARC handles for us (the static is implicitly __strong) but manual
//	retain/release does not.  Hence the guards: retain/release are required without ARC and
//	forbidden with it.  Keep any future Obj-C here similarly neutral.
//
static id<NSObject> s_activityToken = nil;		// __strong under ARC; hand-retained otherwise.

extern "C" void macOS_SetSustainedPerformance(bool sustain, const char* reason)
{
	if (sustain == (s_activityToken != nil))
		return;							// Already in the requested state.

	if (sustain) {
		NSActivityOptions options = NSActivityUserInitiated | NSActivityLatencyCritical;
		NSString* why = reason ? [NSString stringWithUTF8String: reason] : @"realtime rendering";
		s_activityToken = [[NSProcessInfo processInfo] beginActivityWithOptions: options
																		reason: why];
	#if !__has_feature(objc_arc)
		[s_activityToken retain];		// Survive the autorelease pool this frame is drawn in.
	#endif
	} else {
		[[NSProcessInfo processInfo] endActivity: s_activityToken];
	#if !__has_feature(objc_arc)
		[s_activityToken release];
	#endif
		s_activityToken = nil;			// Under ARC this assignment is what releases it.
	}
}

#endif // __APPLE__
