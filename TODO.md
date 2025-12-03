# TODO

## Rendering & Wrapping
- Selection on wrapped lines: draw with `IDWriteTextLayout::HitTestTextRange` (per fragment) instead of single-row rectangles.
- Caret + hit-testing: fully migrate to `TextLayout` (use `GetLineMetrics`/`HitTest*`) and drop fixed `m_lineHeight` assumptions.
- Wrap mode UX: add toggle (status bar + menu) between `Wrap` and `No Wrap`; disable horizontal scroll in `Wrap`.
- Performance: cache `IDWriteTextLayout` per line; invalidate on edit, resize, font/locale change; cap cache memory; reuse brushes.
- Visual polish: wrap indent (hanging indent for continued lines), soft-wrap glyph at line ends, whitespace/tab rendering and tab stop width.

## Encoding & I/O
- Save encoding control: default to UTF-8 (BOM), option to preserve original encoding; show current encoding in status bar.
- Robust detection options: allow forcing CP932/UTF-8 per file; expose an “Reopen with Encoding…” command.
- Large-file path: stream read (non-mapped) with chunked CRLF scanning; async loading to avoid UI stalls (post results to UI thread).

## Input, IME, Text Shaping
- Grapheme-aware navigation: move cursor by cluster (use `HitTestTextRange`/cluster metrics) to handle ZWJ/emoji/合字.
- IME composition: underline/target-range styling via `IDWriteTextLayout` and proper candidate window anchoring (re-query after wrap).
- Font fallback: enable DirectWrite fallback and optional ligatures; allow font family/size in settings.

## UX & Accessibility
- High DPI: verify DPI awareness and scale fonts, paddings; test mixed-DPI monitors.
- Editor chrome: line numbers gutter, current-line highlight, selection colors from theme; optional minimap (later).

## Testing & Tooling
- Manual checklist for wrap, selection, IME, encodings, and 10MB+ files; perf smoke tests (open/scroll/search timings).
- Add `clang-format` config (4-space, Allman, `C`-prefixed classes) and optional `clang-tidy` rules.
- Contrib hygiene: PR template (repro steps, screenshots), CI to build x64 Debug/Release.
