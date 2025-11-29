//
// simplot.h (stub version)
//	No-op stub for ImPlot when GUI system is disabled.
//
// Provides minimal ImPlot API stubs for compilation when IMGUI_DISABLEd.
//
// Created 11/23/25 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef implot_h
#define implot_h

// ImPlot stub - provides no-op implementations when ImPlot is disabled.

namespace ImPlot {
	// Context management
	inline void* CreateContext()						{ return nullptr; }
	inline void DestroyContext(void* ctx = nullptr)		{ }

	// Stub functions - do nothing when ImPlot disabled
	inline void ShowDemoWindow(bool* p_open = nullptr)	{ }
}

#endif	// implot_h  (same name as non-stub)
