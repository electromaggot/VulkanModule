//
// simgui.h (stub version)
//	No-op stub for Dear ImGui when GUI system is disabled.
//
// Provides minimal ImGui API stubs for compilation when ENABLE_IMGUI=OFF.
// Used by CMake conditional compilation.
//
// Created 3/17/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef imgui_h
#define imgui_h

// ImGui stub - provides no-op implementations for GUI code when ImGui is disabled.

#ifndef IMGUI_IMPL_API				// API export macro (used by ImGui backends)
#define IMGUI_IMPL_API
#endif


typedef unsigned int ImGuiID;		// Forward declarations and typedefs

struct ImVec2 {						// Basic 2D vector (forward declared here for ImGuiViewport)
	float x, y;
	ImVec2() : x(0.0f), y(0.0f) {}
	ImVec2(float _x, float _y) : x(_x), y(_y) {}
};

struct ImVec4 {						// Basic 4D vector (for colors)
	float x, y, z, w;
	ImVec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

struct ImGuiViewport {				// Viewport (minimal stub)
	ImGuiID ID;
	ImVec2 Pos;
	ImVec2 Size;
	ImVec2 WorkPos;
	ImVec2 WorkSize;
	ImGuiViewport() : ID(0), Pos(0, 0), Size(0, 0), WorkPos(0, 0), WorkSize(0, 0) {}
};

enum {
	ImGuiKey_Backspace,
	ImGuiKey_Enter,
	NUM_ENUMS
};

enum ImGuiConfigFlags_ {			// ImGui config flags
	ImGuiConfigFlags_None                   = 0,
	ImGuiConfigFlags_NavEnableKeyboard      = 1 << 0,
	ImGuiConfigFlags_NavEnableGamepad       = 1 << 1,
	ImGuiConfigFlags_NavEnableSetMousePos   = 1 << 2,
	ImGuiConfigFlags_NavNoCaptureKeyboard   = 1 << 3,
	ImGuiConfigFlags_NoMouse                = 1 << 4,
	ImGuiConfigFlags_NoMouseCursorChange    = 1 << 5,
	ImGuiConfigFlags_DockingEnable          = 1 << 6,
	ImGuiConfigFlags_ViewportsEnable        = 1 << 10,
	ImGuiConfigFlags_DpiEnableScaleViewports = 1 << 14,
	ImGuiConfigFlags_DpiEnableScaleFonts    = 1 << 15
};
typedef int ImGuiConfigFlags;

enum ImGuiCond_ {			// ImGui condition flags
	ImGuiCond_None          = 0,
	ImGuiCond_Always        = 1 << 0,
	ImGuiCond_Once          = 1 << 1,
	ImGuiCond_FirstUseEver  = 1 << 2,
	ImGuiCond_Appearing     = 1 << 3
};
typedef int ImGuiCond;

enum ImGuiWindowFlags_ {			// ImGui window flags
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
	ImGuiWindowFlags_AlwaysHorizontalScrollbar = 1 << 15,
	ImGuiWindowFlags_AlwaysUseWindowPadding = 1 << 16,
	ImGuiWindowFlags_NoNavInputs            = 1 << 17,
	ImGuiWindowFlags_NoNavFocus             = 1 << 18,
	ImGuiWindowFlags_UnsavedDocument        = 1 << 19,
	ImGuiWindowFlags_NoDocking              = 1 << 20,
	ImGuiWindowFlags_NoNav                  = ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,
	ImGuiWindowFlags_NoDecoration           = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
												| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse,
	ImGuiWindowFlags_NoInputs               = ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs
												| ImGuiWindowFlags_NoNavFocus
};
typedef int ImGuiWindowFlags;

enum ImGuiDockNodeFlags_ {			// ImGui dock node flags
	ImGuiDockNodeFlags_None                     = 0,
	ImGuiDockNodeFlags_KeepAliveOnly            = 1 << 0,
	ImGuiDockNodeFlags_NoDockingInCentralNode   = 1 << 2,
	ImGuiDockNodeFlags_PassthruCentralNode      = 1 << 3,
	ImGuiDockNodeFlags_NoSplit                  = 1 << 4,
	ImGuiDockNodeFlags_NoResize                 = 1 << 5,
	ImGuiDockNodeFlags_AutoHideTabBar           = 1 << 6,
	ImGuiDockNodeFlags_DockSpace                = 1 << 10
};
typedef int ImGuiDockNodeFlags;

enum ImGuiStyleVar_ {				// ImGui style variable for PushStyleVar/PopStyleVar
	ImGuiStyleVar_Alpha,
	ImGuiStyleVar_WindowPadding,
	ImGuiStyleVar_WindowRounding,
	ImGuiStyleVar_WindowBorderSize,
	ImGuiStyleVar_WindowMinSize,
	ImGuiStyleVar_WindowTitleAlign,
	ImGuiStyleVar_ChildRounding,
	ImGuiStyleVar_ChildBorderSize,
	ImGuiStyleVar_PopupRounding,
	ImGuiStyleVar_PopupBorderSize,
	ImGuiStyleVar_FramePadding,
	ImGuiStyleVar_FrameRounding,
	ImGuiStyleVar_FrameBorderSize,
	ImGuiStyleVar_ItemSpacing,
	ImGuiStyleVar_ItemInnerSpacing,
	ImGuiStyleVar_IndentSpacing,
	ImGuiStyleVar_ScrollbarSize,
	ImGuiStyleVar_ScrollbarRounding,
	ImGuiStyleVar_GrabMinSize,
	ImGuiStyleVar_GrabRounding,
	ImGuiStyleVar_TabRounding,
	ImGuiStyleVar_ButtonTextAlign,
	ImGuiStyleVar_SelectableTextAlign,
	ImGuiStyleVar_COUNT
};
typedef int ImGuiStyleVar;

enum ImGuiDir_ {					// Direction for DockBuilder split
	ImGuiDir_None  = -1,
	ImGuiDir_Left  =  0,
	ImGuiDir_Right =  1,
	ImGuiDir_Up    =  2,
	ImGuiDir_Down  =  3,
	ImGuiDir_COUNT
};
typedef int ImGuiDir;

struct ImDrawData {					// Draw data, used by rendering backends
	bool Valid;
	ImDrawData() : Valid(false)	 { }
};

struct ImGuiIO {
	int KeyMap[NUM_ENUMS];
	int KeysDown[NUM_ENUMS];

	ImVec2 DisplaySize;				// Display size (main viewport size, in pixels)

	float Framerate;				// Framerate estimation

	const char* IniFilename;		// INI settings filename (default = "imgui.ini")

	ImGuiConfigFlags ConfigFlags;	// Config flags

	bool WantCaptureMouse;			// Input capture flags (stub always returns false)
	bool WantCaptureKeyboard;

	ImGuiIO() : DisplaySize(0, 0), Framerate(60.0f), IniFilename("imgui.ini"),
				ConfigFlags(0), WantCaptureMouse(false), WantCaptureKeyboard(false)  { }
};

struct ImGui {
	static ImGuiIO& GetIO()  {
		static ImGuiIO io;			// Singleton instance
		return io;
	}

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
	static void SetNextWindowSize(const ImVec2&, ImGuiCond = 0)  { }
	static void SetNextWindowViewport(ImGuiID viewport_id)  { }
	static void SetNextWindowBgAlpha(float)	 { }
	static ImGuiViewport* GetMainViewport()  { static ImGuiViewport vp; return &vp; }

	// Text
	static void Text(const char*, ...)		 { }
	static void TextColored(const ImVec4& col, const char*, ...)  { }
	static void TextDisabled(const char*, ...)  { }
	static void BulletText(const char*, ...) { }

	// Layout
	static void Separator()	 { }
	static void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f)  { }

	// Tooltips
	static bool IsItemHovered(int flags = 0)  { return false; }
	static void SetTooltip(const char*, ...)  { }

	// Widgets: Main
	static bool Button(const char* label, const ImVec2& size = ImVec2(0, 0))  { return false; }
	static bool Checkbox(const char* label, bool* v)  { return false; }
	static bool RadioButton(const char* label, bool active)  { return false; }
	static bool RadioButton(const char* label, int* v, int v_button)  { return false; }

	// Widgets: Menus
	static bool BeginMainMenuBar()  { return false; }
	static void EndMainMenuBar()  { }
	static bool BeginMenu(const char* label, bool enabled = true)  { return false; }
	static void EndMenu()  { }
	static bool MenuItem(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true)  { return false; }
	static bool MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled = true)  { return false; }

	// Style
	static void StyleColorsDark(void* = nullptr)	{ }
	static void StyleColorsClassic(void* = nullptr)	{ }
	static void StyleColorsLight(void* = nullptr)	{ }
	static void PushStyleVar(ImGuiStyleVar idx, float val)  { }
	static void PushStyleVar(ImGuiStyleVar idx, const ImVec2& val)  { }
	static void PopStyleVar(int count = 1)  { }

	// ID stack/scopes
	static ImGuiID GetID(const char* str_id) { return 0; }
	static ImGuiID GetID(const void* ptr_id) { return 0; }

	// Docking
	static ImGuiID DockSpace(ImGuiID id, const ImVec2& size = ImVec2(0, 0), int flags = 0) { return 0; }
	static ImGuiID DockSpaceOverViewport(const void* viewport = nullptr, int flags = 0) { return 0; }

	// Docking Builder (for layout setup)
	static void DockBuilderRemoveNode(ImGuiID node_id)  { }
	static ImGuiID DockBuilderAddNode(ImGuiID node_id = 0, int flags = 0)  { return 0; }
	static void DockBuilderSetNodeSize(ImGuiID node_id, const ImVec2& size)  { }
	static ImGuiID DockBuilderSplitNode(ImGuiID node_id, int split_dir, float size_ratio_for_node_at_dir, ImGuiID* out_id_at_dir, ImGuiID* out_id_at_opposite_dir)  { return 0; }
	static void DockBuilderDockWindow(const char* window_name, ImGuiID node_id)  { }
	static void DockBuilderFinish(ImGuiID node_id)  { }
};

#define IMGUI_CHECKVERSION()

// ImGui helper macros
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR) / sizeof(*(_ARR))))


#endif	// !imgui_h  (same name as non-stub)
