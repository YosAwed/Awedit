# Repository Guidelines

## Project Structure & Module Organization
- Source: `TextEditor/` (`Main.cpp`, `MainWindow.*`, `TextDocument.*`, `TextRenderer.*`, `EditController.*`, `SearchEngine.*`, `UndoManager.*`, `KeyboardHandler.*`, `Resource.*`).
- Solution: `TextEditor.sln` (open in Visual Studio).
- Binaries: `TextEditor/x64/{Debug,Release}/TextEditor.exe` (x86 variants if configured).
- Assets: `Resource.rc`, `Resource.h`, optional `icon.ico` (see `icon_placeholder.txt`).

## Build, Test, and Development Commands
- Visual Studio: open `TextEditor.sln` → select `Debug`/`Release`, `x64`/`Win32` → build (`Ctrl+Shift+B`).
- CLI (Developer Command Prompt):
  - Debug x64: `msbuild TextEditor.sln /p:Configuration=Debug /p:Platform=x64`
  - Release x64: `msbuild TextEditor.sln /p:Configuration=Release /p:Platform=x64`
- Run: `TextEditor/x64/Debug/TextEditor.exe` (or `Release`).

## Coding Style & Naming Conventions
- Language: C++17; Windows SDK 10+; Visual Studio 2019+.
- Indentation: 4 spaces; Allman braces; UTF‑8 source.
- Naming: classes `C`+PascalCase (e.g., `CMainWindow`); methods PascalCase; members `m_` prefix (e.g., `m_hwnd`).
- Prefer STL and safe functions over legacy/unsafe C APIs.

## Testing Guidelines
- No formal unit tests yet. Provide manual verification steps in PRs:
  - Open/save files (including 10MB+), edit, undo/redo.
  - Find/replace, cursor/selection behaviors, IME input.
  - Rendering correctness (DirectWrite) and scrolling performance.
- Optional: propose GoogleTest scaffolding in a separate PR.

## Commit & Pull Request Guidelines
- Commits: use Conventional Commits where possible: `feat:`, `fix:`, `refactor:`, `docs:`, `build:`; include a short scope (e.g., `renderer`, `editor`).
- PRs: clear description, linked issues, before/after screenshots for UI, reproduction/verification steps, target config (`Debug/Release`, `x64/Win32`).
- Keep changes focused; update docs if behavior or build steps change.

## Security & Configuration Tips
- Ensure Windows SDK and toolset installed; set C++ standard to C++17 in project settings.
- Avoid blocking UI thread with long I/O; large files use memory‑mapped I/O—do not copy entire buffers.
- Prefer secure `_s` variants or STL for file/string ops when applicable.
