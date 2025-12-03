// EditController.h - 編集コントローラー（マルチカーソル対応）
#pragma once
#include <windows.h>
#include <vector>
#include "TextDocument.h"

// 選択範囲を表す構造体
struct Selection
{
    TextPosition start;
    TextPosition end;

    Selection() {}
    Selection(const TextPosition& s, const TextPosition& e) : start(s), end(e) {}

    bool IsEmpty() const { return start == end; }
};

class CEditController
{
public:
    CEditController();
    ~CEditController();

    // カーソル操作
    void SetCursor(const TextPosition& pos);
    void AddCursor(const TextPosition& pos);
    void RemoveLastCursor();
    void ClearCursors();
    const std::vector<TextPosition>& GetCursors() const { return m_cursors; }

    // 選択操作
    void BeginSelection(int x, int y, bool isRectangular);
    void UpdateSelection(int x, int y);
    void EndSelection(int x, int y);
    // 高精度（レンダラー由来）座標でのI/F
    void BeginSelectionAtPosition(const TextPosition& pos, bool isRectangular);
    void UpdateSelectionToPosition(const TextPosition& pos);
    // Shift+クリックなど、現在のカーソルからposまでを選択
    void SelectToPosition(const TextPosition& pos, CTextDocument* pDocument);
    void SelectAll(CTextDocument* pDocument);
    bool HasSelection() const;
    void GetSelection(TextPosition& start, TextPosition& end) const;
    std::wstring GetSelectedText(CTextDocument* pDocument) const;

    // 編集操作
    void InsertChar(CTextDocument* pDocument, wchar_t ch);
    void InsertText(CTextDocument* pDocument, const std::wstring& text);
    void DeleteSelection(CTextDocument* pDocument);
    void DeleteChar(CTextDocument* pDocument, bool forward);

    // 移動操作
    void MoveCursor(int dx, int dy, CTextDocument* pDocument);
    void MoveToLineStart();
    void MoveToLineEnd(CTextDocument* pDocument);
    void MoveToDocumentStart();
    void MoveToDocumentEnd(CTextDocument* pDocument);

    // マルチカーソル操作
    void AddCursorAbove(CTextDocument* pDocument);
    void AddCursorBelow(CTextDocument* pDocument);

    // 矩形選択
    bool IsRectangularSelection() const { return m_isRectangularSelection; }
    void SetRectangularSelection(bool value) { m_isRectangularSelection = value; }

private:
    std::vector<TextPosition> m_cursors;
    std::vector<Selection> m_selections;
    
    bool m_isSelecting;
    bool m_isRectangularSelection;
    TextPosition m_selectionAnchor;
};
