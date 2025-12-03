// UndoManager.cpp - Undo/Redo管理実装
#include "UndoManager.h"

// CInsertTextCommand実装
void CInsertTextCommand::Execute(CTextDocument* pDocument)
{
    if (pDocument)
    {
        pDocument->InsertText(m_position, m_text);
        
        // 挿入後の終了位置を計算
        m_endPosition = m_position;
        size_t lineCount = 0;
        size_t lastLineLength = 0;
        
        for (wchar_t ch : m_text)
        {
            if (ch == L'\n')
            {
                lineCount++;
                lastLineLength = 0;
            }
            else if (ch != L'\r')
            {
                lastLineLength++;
            }
        }
        
        if (lineCount > 0)
        {
            m_endPosition.line += lineCount;
            m_endPosition.column = lastLineLength;
        }
        else
        {
            m_endPosition.column += m_text.length();
        }
    }
}

void CInsertTextCommand::Undo(CTextDocument* pDocument)
{
    if (pDocument)
    {
        pDocument->DeleteRange(m_position, m_endPosition);
    }
}

void CInsertTextCommand::Redo(CTextDocument* pDocument)
{
    Execute(pDocument);
}

// CDeleteTextCommand実装
void CDeleteTextCommand::Execute(CTextDocument* pDocument)
{
    if (pDocument)
    {
        m_deletedText = pDocument->GetTextRange(m_start, m_end);
        pDocument->DeleteRange(m_start, m_end);
    }
}

void CDeleteTextCommand::Undo(CTextDocument* pDocument)
{
    if (pDocument)
    {
        pDocument->InsertText(m_start, m_deletedText);
    }
}

void CDeleteTextCommand::Redo(CTextDocument* pDocument)
{
    if (pDocument)
    {
        pDocument->DeleteRange(m_start, m_end);
    }
}

// CReplaceTextCommand実装
void CReplaceTextCommand::Execute(CTextDocument* pDocument)
{
    if (pDocument)
    {
        m_oldText = pDocument->GetTextRange(m_start, m_end);
        pDocument->ReplaceRange(m_start, m_end, m_newText);
    }
}

void CReplaceTextCommand::Undo(CTextDocument* pDocument)
{
    if (pDocument)
    {
        // 新しいテキストの終了位置を計算
        TextPosition newEnd = m_start;
        size_t lineCount = 0;
        size_t lastLineLength = 0;
        
        for (wchar_t ch : m_newText)
        {
            if (ch == L'\n')
            {
                lineCount++;
                lastLineLength = 0;
            }
            else if (ch != L'\r')
            {
                lastLineLength++;
            }
        }
        
        if (lineCount > 0)
        {
            newEnd.line += lineCount;
            newEnd.column = lastLineLength;
        }
        else
        {
            newEnd.column += m_newText.length();
        }
        
        pDocument->ReplaceRange(m_start, newEnd, m_oldText);
    }
}

void CReplaceTextCommand::Redo(CTextDocument* pDocument)
{
    if (pDocument)
    {
        pDocument->ReplaceRange(m_start, m_end, m_newText);
    }
}

// CUndoManager実装
CUndoManager::CUndoManager()
    : m_currentIndex(0)
    , m_maxMemoryUsage(100 * 1024 * 1024) // デフォルト100MB
    , m_estimatedMemoryUsage(0)
{
}

CUndoManager::~CUndoManager()
{
}

void CUndoManager::ExecuteCommand(std::unique_ptr<ICommand> command, CTextDocument* pDocument)
{
    if (!command || !pDocument)
    {
        return;
    }

    // 現在位置より後ろのコマンドを削除
    if (m_currentIndex < m_commands.size())
    {
        m_commands.erase(m_commands.begin() + m_currentIndex, m_commands.end());
    }

    // コマンドを実行
    command->Execute(pDocument);

    // コマンドを履歴に追加
    m_estimatedMemoryUsage += EstimateCommandSize(command.get());
    m_commands.push_back(std::move(command));
    m_currentIndex++;

    // メモリ制限チェック
    TrimMemoryIfNeeded();
}

bool CUndoManager::CanUndo() const
{
    return m_currentIndex > 0;
}

bool CUndoManager::CanRedo() const
{
    return m_currentIndex < m_commands.size();
}

void CUndoManager::Undo(CTextDocument* pDocument)
{
    if (!CanUndo() || !pDocument)
    {
        return;
    }

    m_currentIndex--;
    m_commands[m_currentIndex]->Undo(pDocument);
}

void CUndoManager::Redo(CTextDocument* pDocument)
{
    if (!CanRedo() || !pDocument)
    {
        return;
    }

    m_commands[m_currentIndex]->Redo(pDocument);
    m_currentIndex++;
}

void CUndoManager::Clear()
{
    m_commands.clear();
    m_currentIndex = 0;
    m_estimatedMemoryUsage = 0;
}

void CUndoManager::TrimMemoryIfNeeded()
{
    // メモリ使用量が制限を超えた場合、古いコマンドを削除
    while (m_estimatedMemoryUsage > m_maxMemoryUsage && !m_commands.empty())
    {
        m_estimatedMemoryUsage -= EstimateCommandSize(m_commands.front().get());
        m_commands.erase(m_commands.begin());
        
        if (m_currentIndex > 0)
        {
            m_currentIndex--;
        }
    }
}

size_t CUndoManager::EstimateCommandSize(const ICommand* pCommand) const
{
    // コマンドのメモリ使用量を推定（簡易実装）
    // 実際にはコマンドの種類に応じてより正確な計算が必要
    return sizeof(ICommand) + 1024; // 基本サイズ + バッファ
}
