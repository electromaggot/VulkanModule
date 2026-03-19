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

#endif // __APPLE__
