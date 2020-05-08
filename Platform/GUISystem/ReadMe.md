GUISystem/Platform
------------------

GUI may seem like a component totally separate from platform, however, its implementation heavily
couples with the rendering system (of course) as well as platform-specific I/O.  Hence why GUISystem
is included here in, and depends on, this Platform directory.

Specifying which GUI system you wish to use, or none at all, depends entirely on your project settings.
That is, which GUI-specific files you include... or those you don't, if you have no need for a GUI.</br>
We have settled on Dear ImGui, which is well-regarded and low-BS, but it's possible through this
directory to support other GUI systems.  (Unfortunately, to limit piling-on extraneous code, I
stopped short of defining an extra interface layer to make the GUI system "agnostic" - you'd have to
add that.  So for simplicity, some dependencies remain, like calling `ImGui_ImplSDL2_InitForVulkan()`.)

### For Dear ImGui 

Use the `DearImGui.cpp` file here for linkage into Dear ImGui and add to your project its
implementation files, including the platform-specific ones for SDL and Vulkan.  These will
probably be from an "external" shared directory, since Dear ImGui is its own Git project.

### For no GUI

You may have no use for a GUI, but you'll still have to include iPlatform and, for instance,
PlatformSDL.  While that currently contains some Dear ImGui specifics, a directory of "stubs" is
provided to resolve those method calls with empty (unused) implementations.  Just make sure to
reference those in your project.  Your code won't be calling these anyway, and the compiler
optimizes them out, but your project will build.

On Apple platforms, Xcode searches a `.cpp` file's same directory for `.h` files it `#include`s -
which is why the "stubs" need to be in their own subdirectory.  This keeps a file like `DearImGui.cpp`
- if you are using that GUI - from referencing the stubbed `imgui_impl_sdl.h` instead of the correct
one in the "imgui" project's directory.  Be sure to add a specific reference to that directory
to Xcode's `Header Search Paths` for your own project.

