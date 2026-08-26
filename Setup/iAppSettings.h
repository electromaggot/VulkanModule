//
// iAppSettings.h
//	Vulkan Setup
//
// The application's PERSISTENT settings, as this module needs to see them.
//
// Unlike VulkanConfig -- which is fixed configuration the application hands over once -- this
//	is a live object the module calls back into: window geometry is read at window creation and
//	written back on every move, resize, or fullscreen toggle, then committed by Save().  A plain
//	struct cannot express that, hence an interface the application implements.
//
//	THE APPLICATION DEFINES AppStoredSettings(), declared below.  Returning nullptr is
//	legitimate and means "this application persists nothing": the module then starts windows at
//	the VulkanConfig default size and quietly skips saving.  Everything here is optional in that
//	sense, but the linker still requires the definition, so no consumer forgets it by accident.
//
//	Typical implementation -- an existing settings class simply gains the interface:
//
//		class AppSettings : public iAppSettings { ... };
//
//		iAppSettings* AppStoredSettings()	{ return &AppConstants.Settings; }
//
// Created 8/24/26 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef iAppSettings_h
#define iAppSettings_h

#include "VulkanPlatform.h"


// Where a window sits and how big it is, as restored from (or committed to) storage.
//	Grouped because the platform layer always wants all of it at once.
//
struct WindowGeometry
{
	int		x = 0,  y = 0;
	int		width = 0,  height = 0;

	bool	isFullScreen = false;

	bool	isStored = false;	// false: nothing was ever saved, so the values above are
								//	meaningless -- start from VulkanConfig's defaults instead.
								//	(While fullscreen, the width/height/x/y retained are the
								//	 last WINDOWED ones, so leaving it restores that size.)
};


class iAppSettings
{
public:
	virtual ~iAppSettings() = default;

		// WINDOW GEOMETRY - the module reads once at startup, writes on every move/resize.
	virtual WindowGeometry	GetWindowGeometry() const = 0;
	virtual void			SetWindowGeometry(const WindowGeometry& geometry) = 0;

	virtual void			Save() = 0;		// commit to wherever this app persists things

		// DIAGNOSTICS - both optional; defaults suit an app that doesn't care.
	virtual bool	IsDebugLogToFile() const	{ return false; }
	virtual StrPtr	SettingsFilePath() const	{ return ""; }
					// Reported in the startup log.  Must stay valid for the process, and
					//	never null: it reaches printf.
};


// DEFINED BY THE APPLICATION (see the header comment above).  May return nullptr, so callers
//	must check -- and the module is expected to degrade gracefully when it does.
//
iAppSettings*	AppStoredSettings();


#endif	// iAppSettings_h
