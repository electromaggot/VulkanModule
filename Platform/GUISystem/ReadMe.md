GUISystem/Platform
------------------

GUI may seem like a component totally separate from platform, however, it heavily couples with
the rendering system (of course) as well as platform-specific I/O.  Hence why GUISystem is
included here in, and depends on, this Platform directory.

TBD_TODO:
Specifying which GUI system you wish to use, or none at all, depends entirely on your project settings.
That is, which GUI-specific files you include and those you don't.

If you have no use for a GUI, while you *will* have to include iPlatform in your project . . .

There will be empty calls, but the compiler should optimize them out.
