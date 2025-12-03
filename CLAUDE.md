# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A high-performance text editor built with Win32 API and C++17, featuring DirectWrite rendering, multi-cursor editing, and memory-mapped file support for large files (10MB+).

## Build Commands

### Build (Visual Studio GUI)
1. Open `TextEditor.sln` in Visual Studio
2. Select configuration (Debug/Release) and platform (x86/x64)
3. Build → Build Solution (or Ctrl+Shift+B)

### Build (Command Line)
From Developer Command Prompt for VS:
```cmd
# Release build (x64, recommended)
msbuild TextEditor.sln /p:Configuration=Release /p:Platform=x64

# Debug build
msbuild TextEditor.sln /p:Configuration=Debug /p:Platform=x64

# Clean and rebuild
msbuild TextEditor.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64

# Parallel build for faster compilation
msbuild TextEditor.sln /p:Configuration=Release /p:Platform=x64 /m
```

### Run
```cmd
# After building, executable is at:
TextEditor\x64\Release\TextEditor.exe  # Release x64
TextEditor\x64\Debug\TextEditor.exe    # Debug x64
```

## Architecture

### Class Responsibilities

The codebase follows separation of concerns with 7 main classes:

- **CMainWindow** (`MainWindow.h/cpp`): Window management and Win32 message loop. Entry point for all user interactions (menu commands, keyboard input, mouse events). Coordinates between all other components.

- **CTextDocument** (`TextDocument.h/cpp`): Text data storage using `std::vector<std::wstring>` (line-based). Handles file I/O with automatic memory-mapped file loading for files ≥10MB. Supports UTF-8 and UTF-16 with BOM.

- **CTextRenderer** (`TextRenderer.h/cpp`): DirectWrite/Direct2D rendering engine. Handles text layout, cursor drawing, selection highlighting, and color emoji support. Uses hardware-accelerated rendering via `ID2D1HwndRenderTarget`.

- **CEditController** (`EditController.h/cpp`): Multi-cursor and rectangular selection manager. Maintains `std::vector<TextPosition>` for cursor positions. Coordinates all editing operations across multiple cursors simultaneously.

- **CSearchEngine** (`SearchEngine.h/cpp`): Search/replace using `std::regex`. Supports case-sensitive, whole-word, and regex pattern matching. Note: API is implemented but dialog UI is not yet implemented.

- **CUndoManager** (`UndoManager.h/cpp`): Command pattern-based undo/redo. Each edit operation creates an `ICommand` object. Maintains history stack with memory monitoring.

- **CKeyboardHandler** (`KeyboardHandler.h/cpp`): Keyboard shortcut registration using `std::map<KeyCombination, std::function<void()>>`. Handles all Ctrl+Key combinations and special keys.

### Data Flow

1. User input → `CMainWindow::HandleMessage()` → `CKeyboardHandler` or direct handler
2. Edit operation → `CEditController` → `CTextDocument` → `CUndoManager` (command created)
3. Document changed → `CMainWindow` invalidates window → `OnPaint()` → `CTextRenderer::Render()`
4. File operations → `CTextDocument::LoadFromFile()` (automatic memory-mapped file detection for large files)

### Memory-Mapped Files

Files ≥10MB are automatically loaded using `CreateFileMapping()` and `MapViewOfFile()`. This allows efficient handling of large files without loading entire content into memory. Located in `CTextDocument::LoadFromMemoryMappedFile()`.

### Multi-Cursor Implementation

All editing operations iterate over `CEditController::m_cursors` vector. Each cursor position is updated after operations to maintain correct positions. Rectangular selection creates multiple cursors with corresponding selection ranges.

## Key Technical Details

### DirectWrite Initialization
COM must be initialized before DirectWrite/Direct2D usage (done in `Main.cpp`). Renderer creates:
- `ID2D1Factory` for Direct2D
- `IDWriteFactory` for DirectWrite
- `ID2D1HwndRenderTarget` for window rendering
- `IDWriteTextFormat` for font/size configuration

### Resource Management
- COM objects: Use `Release()` and set to `nullptr`
- HANDLE resources: Use `CloseHandle()`
- Memory-mapped files: Unmap with `UnmapViewOfFile()`, close mapping with `CloseHandle(m_hMapping)`, close file with `CloseHandle(m_hFile)`

### Encoding Support
- UTF-8: Default encoding
- UTF-16 with BOM: Detected by BOM (0xFEFF)
- CRLF line endings: Full support (Windows standard)

## Common Development Scenarios

### Adding a new menu command
1. Add ID to `Resource.h` (e.g., `#define IDM_NEW_FEATURE 1234`)
2. Add menu item to `Resource.rc`
3. Add handler in `CMainWindow::OnCommand()` switch statement
4. Implement functionality using appropriate controller/manager

### Adding a new keyboard shortcut
Register in `CKeyboardHandler::InitializeDefaultShortcuts()`:
```cpp
RegisterShortcut(KeyCombination(VK_X, true, false, false),
    [this]() { /* handler */ });
```

### Adding a new editing operation
1. Create `ICommand` subclass in `UndoManager.h/cpp`
2. Implement `Execute()`, `Undo()`, `Redo()` methods
3. Add method to `CEditController` that creates and executes the command
4. Ensure operation works across all cursors in `m_cursors` vector

### Extending file format support
Add format detection and parsing in `CTextDocument::LoadFromFile()`. Consider using memory-mapped approach for formats that can be large.

## Known Limitations

1. **Icon file**: Default `icon.ico` is not included in repository. Reference is commented out in `Resource.rc` and default Windows icon is used instead.

2. **Search/Replace UI**: Backend API fully implemented in `CSearchEngine`, but dialog UI is missing. Search functionality is ready to be connected to a dialog.

3. **Memory-mapped file editing**: Read-only mode. Editing and saving memory-mapped files is not yet implemented.

4. **Line length**: Very long lines (10,000+ characters) may cause rendering slowdown.

5. **Encoding**: Only UTF-8 and UTF-16 with BOM supported. No automatic encoding detection for other formats.

## Troubleshooting Build Errors

### UTF-8 Encoding Errors
If you see errors like "リテラル サフィックス '繧ｨ繝ｩ繝ｼ' が無効です" or character encoding issues:
- The project file has been configured with `/utf-8` compiler option for all configurations
- Ensure all source files (.cpp, .h) are saved as UTF-8 (with or without BOM)
- In Visual Studio: File → Advanced Save Options → Select "Unicode (UTF-8 without signature)"
- Resource.rc has `#pragma code_page(65001)` to handle UTF-8 text in menus/dialogs

### Missing windowsx.h Macros
If you see "GET_X_LPARAM identifier not found" errors:
- `#include <windowsx.h>` is already added to MainWindow.cpp
- This provides GET_X_LPARAM, GET_Y_LPARAM, and GET_WHEEL_DELTA_WPARAM macros

### Icon Resource Errors
If you see "icon.ico not found" errors:
- The icon reference in Resource.rc is commented out
- Default Windows application icon is used via LoadIcon(NULL, IDI_APPLICATION)

### std::min/std::max Macro Conflicts
If you see errors like "スコープ解決演算子 (::) の右側にあるトークンは使えません" with std::min or std::max:
- The project has been configured with `NOMINMAX` preprocessor definition
- This prevents windows.h from defining min/max macros that conflict with std::min/std::max

## Coding Conventions

- **Class names**: `CClassName` (C prefix + PascalCase)
- **Member variables**: `m_variableName` (m_ prefix + camelCase)
- **Pointers**: `pPointer` (p prefix)
- **Constants**: `CONSTANT_NAME` (uppercase with underscores)
- **C++ standard**: C++17 features are available
