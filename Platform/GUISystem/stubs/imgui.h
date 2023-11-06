//
// imgui.h
//	Dummy placeholder for Dear ImGui or other GUI system.
//
// Created 3/17/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef imgui_h
#define imgui_h

enum {
	ImGuiKey_Backspace,
	ImGuiKey_Enter,
	NUM_ENUMS
};

struct ImGuiIO {
	int KeyMap[NUM_ENUMS];
	int KeysDown[NUM_ENUMS];
};

struct ImGui {
	static ImGuiIO io;
	static ImGuiIO& GetIO() { return io; }
};

#ifdef INSTANTIATE
ImGuiIO ImGui::io;
#endif

#endif	// imgui_h
