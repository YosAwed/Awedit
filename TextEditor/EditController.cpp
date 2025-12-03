// EditController.cpp - 編集コントローラー実装
#include "EditController.h"
#include <algorithm>

CEditController::CEditController()
    : m_isSelecting(false)
    , m_isRectangularSelection(false)
{
    m_cursors.push_back(TextPosition(0, 0));
}

CEditController::~CEditController()
{
}

void CEditController::SetCursor(const TextPosition& pos)
{
    m_cursors.clear();
    m_cursors.push_back(pos);
    m_selections.clear();
}

void CEditController::AddCursor(const TextPosition& pos)
{
    // 既存のカーソルと重複しないかチェック
    for (const auto& cursor : m_cursors)
    {
        if (cursor == pos)
        {
            return;
        }
    }
    m_cursors.push_back(pos);
}

void CEditController::RemoveLastCursor()
{
    if (m_cursors.size() > 1)
    {
        m_cursors.pop_back();
        if (!m_selections.empty())
        {
            m_selections.pop_back();
        }
    }
}

void CEditController::ClearCursors()
{
    m_cursors.clear();
    m_cursors.push_back(TextPosition(0, 0));
    m_selections.clear();
}

void CEditController::BeginSelection(int x, int y, bool isRectangular)
{
    m_isSelecting = true;
    m_isRectangularSelection = isRectangular;
    
    // 簡易的な座標からテキスト位置への変換
    // 実際のアプリケーションではTextRendererを使用
    m_selectionAnchor = TextPosition(y / 20, x / 8);
    
    if (!isRectangular)
    {
        SetCursor(m_selectionAnchor);
    }
}

void CEditController::BeginSelectionAtPosition(const TextPosition& pos, bool isRectangular)
{
    m_isSelecting = true;
    m_isRectangularSelection = isRectangular;
    m_selectionAnchor = pos;
    if (!isRectangular)
    {
        SetCursor(m_selectionAnchor);
    }
}

void CEditController::UpdateSelection(int x, int y)
{
    if (!m_isSelecting)
    {
        return;
    }

    TextPosition currentPos(y / 20, x / 8);
    
    if (m_isRectangularSelection)
    {
        // 矩形選択の実装
        m_cursors.clear();
        m_selections.clear();

        size_t startLine = std::min(m_selectionAnchor.line, currentPos.line);
        size_t endLine = std::max(m_selectionAnchor.line, currentPos.line);
        size_t startCol = std::min(m_selectionAnchor.column, currentPos.column);
        size_t endCol = std::max(m_selectionAnchor.column, currentPos.column);

        for (size_t line = startLine; line <= endLine; ++line)
        {
            TextPosition start(line, startCol);
            TextPosition end(line, endCol);
            m_cursors.push_back(end);
            m_selections.push_back(Selection(start, end));
        }
    }
    else
    {
        // 通常の選択
        if (!m_cursors.empty())
        {
            m_cursors[0] = currentPos;
            m_selections.clear();
            m_selections.push_back(Selection(m_selectionAnchor, currentPos));
        }
    }
}

void CEditController::UpdateSelectionToPosition(const TextPosition& currentPos)
{
    if (!m_isSelecting)
    {
        return;
    }

    if (m_isRectangularSelection)
    {
        m_cursors.clear();
        m_selections.clear();

        size_t startLine = std::min(m_selectionAnchor.line, currentPos.line);
        size_t endLine = std::max(m_selectionAnchor.line, currentPos.line);
        size_t startCol = std::min(m_selectionAnchor.column, currentPos.column);
        size_t endCol = std::max(m_selectionAnchor.column, currentPos.column);

        for (size_t line = startLine; line <= endLine; ++line)
        {
            TextPosition start(line, startCol);
            TextPosition end(line, endCol);
            m_cursors.push_back(end);
            m_selections.push_back(Selection(start, end));
        }
    }
    else
    {
        if (!m_cursors.empty())
        {
            m_cursors[0] = currentPos;
            m_selections.clear();
            m_selections.push_back(Selection(m_selectionAnchor, currentPos));
        }
    }
}

void CEditController::SelectToPosition(const TextPosition& pos, CTextDocument* pDocument)
{
    if (!pDocument)
    {
        return;
    }

    TextPosition clamped = pDocument->ClampPosition(pos);

    if (m_cursors.empty())
    {
        m_cursors.push_back(TextPosition(0, 0));
    }

    // 既に選択がある場合は既存のアンカーを尊重。なければ現在のカーソルがアンカー。
    TextPosition anchor;
    if (HasSelection())
    {
        // 既存選択の始点をアンカーとする
        TextPosition start, end;
        GetSelection(start, end);
        anchor = start;
    }
    else
    {
        anchor = m_cursors[0];
    }

    m_selectionAnchor = anchor;
    m_selections.clear();
    m_selections.push_back(Selection(anchor, clamped));
    m_cursors[0] = clamped;
}

void CEditController::EndSelection(int x, int y)
{
    m_isSelecting = false;
}

void CEditController::SelectAll(CTextDocument* pDocument)
{
    if (!pDocument)
    {
        return;
    }

    m_cursors.clear();
    m_selections.clear();

    TextPosition start(0, 0);
    TextPosition end(pDocument->GetLineCount() - 1, pDocument->GetLine(pDocument->GetLineCount() - 1).length());

    m_cursors.push_back(end);
    m_selections.push_back(Selection(start, end));
}

bool CEditController::HasSelection() const
{
    for (const auto& sel : m_selections)
    {
        if (!sel.IsEmpty())
        {
            return true;
        }
    }
    return false;
}

void CEditController::GetSelection(TextPosition& start, TextPosition& end) const
{
    if (!m_selections.empty())
    {
        start = m_selections[0].start;
        end = m_selections[0].end;
    }
    else if (!m_cursors.empty())
    {
        start = end = m_cursors[0];
    }
}

std::wstring CEditController::GetSelectedText(CTextDocument* pDocument) const
{
    if (!pDocument || m_selections.empty())
    {
        return L"";
    }

    std::wstring result;
    for (const auto& sel : m_selections)
    {
        if (!sel.IsEmpty())
        {
            result += pDocument->GetTextRange(sel.start, sel.end);
            if (m_selections.size() > 1)
            {
                result += L"\r\n";
            }
        }
    }

    return result;
}

void CEditController::InsertChar(CTextDocument* pDocument, wchar_t ch)
{
    if (!pDocument)
    {
        return;
    }

    // 選択範囲があれば削除
    if (HasSelection())
    {
        DeleteSelection(pDocument);
    }

    // 各カーソル位置に文字を挿入
    for (auto& cursor : m_cursors)
    {
        pDocument->InsertChar(cursor, ch);
        
        // カーソル位置を更新
        if (ch == L'\r' || ch == L'\n')
        {
            cursor.line++;
            cursor.column = 0;
        }
        else
        {
            cursor.column++;
        }
    }
}

void CEditController::InsertText(CTextDocument* pDocument, const std::wstring& text)
{
    if (!pDocument || text.empty())
    {
        return;
    }

    // 選択範囲があれば削除
    if (HasSelection())
    {
        DeleteSelection(pDocument);
    }

    // 各カーソル位置にテキストを挿入
    for (auto& cursor : m_cursors)
    {
        pDocument->InsertText(cursor, text);
        
        // カーソル位置を更新（簡易実装）
        size_t newlineCount = 0;
        size_t lastNewlinePos = 0;
        for (size_t i = 0; i < text.length(); ++i)
        {
            if (text[i] == L'\n')
            {
                newlineCount++;
                lastNewlinePos = i;
            }
        }

        if (newlineCount > 0)
        {
            cursor.line += newlineCount;
            cursor.column = text.length() - lastNewlinePos - 1;
        }
        else
        {
            cursor.column += text.length();
        }
    }
}

void CEditController::DeleteSelection(CTextDocument* pDocument)
{
    if (!pDocument || !HasSelection())
    {
        return;
    }

    // 選択範囲を削除（後ろから削除して位置ずれを防ぐ）
    for (auto it = m_selections.rbegin(); it != m_selections.rend(); ++it)
    {
        if (!it->IsEmpty())
        {
            pDocument->DeleteRange(it->start, it->end);
        }
    }

    // カーソルを選択開始位置に移動
    if (!m_selections.empty())
    {
        for (size_t i = 0; i < m_cursors.size() && i < m_selections.size(); ++i)
        {
            m_cursors[i] = m_selections[i].start < m_selections[i].end ? 
                          m_selections[i].start : m_selections[i].end;
        }
    }

    m_selections.clear();
}

void CEditController::DeleteChar(CTextDocument* pDocument, bool forward)
{
    if (!pDocument)
    {
        return;
    }

    if (HasSelection())
    {
        DeleteSelection(pDocument);
        return;
    }

    // 各カーソル位置で文字を削除
    for (auto& cursor : m_cursors)
    {
        if (forward)
        {
            pDocument->DeleteChar(cursor);
        }
        else
        {
            // Backspace
            if (cursor.column > 0)
            {
                cursor.column--;
                pDocument->DeleteChar(cursor);
            }
            else if (cursor.line > 0)
            {
                size_t prevLineLength = pDocument->GetLine(cursor.line - 1).length();
                cursor.line--;
                cursor.column = prevLineLength;
                pDocument->DeleteChar(cursor);
            }
        }
    }
}

void CEditController::MoveCursor(int dx, int dy, CTextDocument* pDocument)
{
    if (!pDocument)
    {
        return;
    }

    for (auto& cursor : m_cursors)
    {
        // Y方向の移動
        if (dy != 0)
        {
            int newLine = static_cast<int>(cursor.line) + dy;
            if (newLine < 0)
            {
                newLine = 0;
            }
            if (newLine >= static_cast<int>(pDocument->GetLineCount()))
            {
                newLine = static_cast<int>(pDocument->GetLineCount()) - 1;
            }
            cursor.line = static_cast<size_t>(newLine);
        }

        // X方向の移動
        if (dx != 0)
        {
            int newCol = static_cast<int>(cursor.column) + dx;
            if (newCol < 0)
            {
                newCol = 0;
            }
            size_t lineLength = pDocument->GetLine(cursor.line).length();
            if (newCol > static_cast<int>(lineLength))
            {
                newCol = static_cast<int>(lineLength);
            }
            cursor.column = static_cast<size_t>(newCol);
        }

        // 位置を正規化
        cursor = pDocument->ClampPosition(cursor);
    }

    m_selections.clear();
}

void CEditController::MoveToLineStart()
{
    for (auto& cursor : m_cursors)
    {
        cursor.column = 0;
    }
    m_selections.clear();
}

void CEditController::MoveToLineEnd(CTextDocument* pDocument)
{
    if (!pDocument)
    {
        return;
    }

    for (auto& cursor : m_cursors)
    {
        cursor.column = pDocument->GetLine(cursor.line).length();
    }
    m_selections.clear();
}

void CEditController::MoveToDocumentStart()
{
    for (auto& cursor : m_cursors)
    {
        cursor.line = 0;
        cursor.column = 0;
    }
    m_selections.clear();
}

void CEditController::MoveToDocumentEnd(CTextDocument* pDocument)
{
    if (!pDocument)
    {
        return;
    }

    for (auto& cursor : m_cursors)
    {
        cursor.line = pDocument->GetLineCount() - 1;
        cursor.column = pDocument->GetLine(cursor.line).length();
    }
    m_selections.clear();
}

void CEditController::AddCursorAbove(CTextDocument* pDocument)
{
    if (!pDocument || m_cursors.empty())
    {
        return;
    }

    TextPosition lastCursor = m_cursors.back();
    if (lastCursor.line > 0)
    {
        TextPosition newCursor(lastCursor.line - 1, lastCursor.column);
        newCursor = pDocument->ClampPosition(newCursor);
        AddCursor(newCursor);
    }
}

void CEditController::AddCursorBelow(CTextDocument* pDocument)
{
    if (!pDocument || m_cursors.empty())
    {
        return;
    }

    TextPosition lastCursor = m_cursors.back();
    if (lastCursor.line < pDocument->GetLineCount() - 1)
    {
        TextPosition newCursor(lastCursor.line + 1, lastCursor.column);
        newCursor = pDocument->ClampPosition(newCursor);
        AddCursor(newCursor);
    }
}
