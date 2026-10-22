# Architecture and platform backends

Pixelate is split into a platform-neutral UI layer and platform-specific
backends.

## Platform-neutral layer

Parser implementation is organized under `language/parser/`: `program.cpp`
handles top-level blocks and declarations, `elements.cpp` handles UI elements,
`updates.cpp` handles property updates, and `statements.cpp` handles
expressions, conditions, control flow, and triggers.

These parts should remain reusable across Windows, macOS, and Linux:

- `runtime/` — executes PXL programs.
- `language/` — lexer, parser, AST, and interpreter.
- `graphics/ui/widget.h` — widget geometry, hit testing, and widget behavior.
- `graphics/ui/label.*`, `graphics/ui/paragraph.*`, `graphics/ui/button.*`, `graphics/ui/textbox.*`, and `graphics/ui/textarea.*` — public widget implementations.
- `graphics/ui/draw_context.h` — abstract drawing operations used by widgets.
- `graphics/platform/platform_types.h` — platform-neutral colors, cursor names, and window options.
- `graphics/platform/native_textbox.h` — the platform-neutral editable textbox and textarea contracts.

The generic widgets do not need to know whether they are running on Win32,
Cocoa, or X11/Wayland.

## Current platform backends

Windows-specific code is under `graphics/platform/windows/`:

- `win32_draw_context.*` implements drawing with GDI.
- `win32_native_textbox.cpp` and `win32_native_textarea.cpp` implement native editable Windows controls.
- `platform_types.h` contains shared widget text style and font-family values.
- `graphics/platform/windows/win32_app.cpp` contains the Win32 window/event-loop adapter.

### Linux (experimental)

Linux support is under `graphics/platform/linux/`:

- `gtk3_app.cpp` implements the GTK 3 window and event-loop adapter.
- `gtk3_draw_context.*` implements drawing with Cairo and Pango.
- `gtk3_native_textbox.cpp` and `gtk3_native_textarea.cpp` implement GTK text controls.

The Linux backend is functional but experimental. Native font availability,
desktop theme differences, and window-system details can affect appearance
and behavior.

The `App` public API remains unchanged, so the runtime and PXL programs do not
need to change when a new backend is introduced.

## Adding another backend

A future backend should provide implementations of the drawing context,
native textbox contract, and the App window/event-loop adapter while keeping
the public interfaces and platform-neutral types the same. The Windows files
should remain isolated from the new backend.

The CMake build selects the Windows or Linux backend based on the host
platform. The backend boundary is still evolving while Pixelate remains under
early development.
