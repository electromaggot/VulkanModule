//
// imgui.h
//	Dummy placeholder for Dear ImGui or other GUI system.
//
// Created 3/17/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef imgui_h
#define imgui_h

// ImGui stub - provides no-op implementations for GUI code when ImGui is disabled.

// API export macro (used by ImGui backends)
#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif

enum {
	ImGuiKey_Backspace,
	ImGuiKey_Enter,
	NUM_ENUMS
};

// ImGui condition flags
enum ImGuiCond_ {
	ImGuiCond_None          = 0,
	ImGuiCond_Always        = 1 << 0,
	ImGuiCond_Once          = 1 << 1,
	ImGuiCond_FirstUseEver  = 1 << 2,
	ImGuiCond_Appearing     = 1 << 3
};
typedef int ImGuiCond;

// ImGui window flags
enum ImGuiWindowFlags_ {
	ImGuiWindowFlags_None                   = 0,
	ImGuiWindowFlags_NoTitleBar             = 1 << 0,
	ImGuiWindowFlags_NoResize               = 1 << 1,
	ImGuiWindowFlags_NoMove                 = 1 << 2,
	ImGuiWindowFlags_NoScrollbar            = 1 << 3,
	ImGuiWindowFlags_NoScrollWithMouse      = 1 << 4,
	ImGuiWindowFlags_NoCollapse             = 1 << 5,
	ImGuiWindowFlags_AlwaysAutoResize       = 1 << 6,
	ImGuiWindowFlags_NoBackground           = 1 << 7,
	ImGuiWindowFlags_NoSavedSettings        = 1 << 8,
	ImGuiWindowFlags_NoMouseInputs          = 1 << 9,
	ImGuiWindowFlags_MenuBar                = 1 << 10,
	ImGuiWindowFlags_HorizontalScrollbar    = 1 << 11,
	ImGuiWindowFlags_NoFocusOnAppearing     = 1 << 12,
	ImGuiWindowFlags_NoBringToFrontOnFocus  = 1 << 13,
	ImGuiWindowFlags_AlwaysVerticalScrollbar = 1 << 14,
	ImGuiWindowFlags_AlwaysHorizontalScrollbar = 1<< 15,
	ImGuiWindowFlags_AlwaysUseWindowPadding = 1 << 16,
	ImGuiWindowFlags_NoNavInputs            = 1 << 17,
	ImGuiWindowFlags_NoNavFocus             = 1 << 18,
	ImGuiWindowFlags_UnsavedDocument        = 1 << 19,
	ImGuiWindowFlags_NoNav                  = ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,
	ImGuiWindowFlags_NoDecoration           = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
												| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse,
	ImGuiWindowFlags_NoInputs               = ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs
												| ImGuiWindowFlags_NoNavFocus
};
typedef int ImGuiWindowFlags;

// Basic 2D vector
struct ImVec2 {
	float x, y;
	ImVec2() : x(0.0f), y(0.0f) {}
	ImVec2(float _x, float _y) : x(_x), y(_y) {}
};

// Draw data, used by rendering backends
struct ImDrawData {
	bool Valid;
	ImDrawData() : Valid(false)	 { }
};

struct ImGuiIO {
	int KeyMap[NUM_ENUMS];
	int KeysDown[NUM_ENUMS];

	// Display size (main viewport size, in pixels)
	ImVec2 DisplaySize;

	// Framerate estimation
	float Framerate;

	// INI settings filename (default = "imgui.ini")
	const char* IniFilename;

	ImGuiIO() : DisplaySize(0, 0), Framerate(60.0f), IniFilename("imgui.ini")  { }
};

struct ImGui {
	static ImGuiIO io;
	static ImGuiIO& GetIO() { return io; }

	// Context management
	static void* CreateContext(void* = nullptr)	 { return nullptr; }
	static void DestroyContext(void* = nullptr)	 { }

	// Main
	static void NewFrame() {}
	static void Render() {}
	static ImDrawData* GetDrawData()  { return nullptr; }

	// Window manipulation
	static bool Begin(const char*, bool* = nullptr, ImGuiWindowFlags = 0)  { return false; }
	static void End()	{ }
	static void SetNextWindowPos(const ImVec2&, ImGuiCond = 0, const ImVec2& = ImVec2(0, 0))  { }
	static void SetNextWindowBgAlpha(float)	 { }

	// Text
	static void Text(const char*, ...)		 { }
	static void BulletText(const char*, ...) { }

	// Layout
	static void Separator()	 { }

	// Style
	static void StyleColorsDark(void* = nullptr)	{ }
	static void StyleColorsClassic(void* = nullptr)	{ }
	static void StyleColorsLight(void* = nullptr)	{ }
};

#define IMGUI_CHECKVERSION()

// ImGui helper macros
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR) / sizeof(*(_ARR))))

#ifdef INSTANTIATE
ImGuiIO ImGui::io;
#endif


#endif	// imgui_h
