// UndoManager.h - Undo/Redo管理（無制限対応）
#pragma once
#include <windows.h>
#include <memory>
#include <vector>
#include "TextDocument.h"

// コマンドの基底クラス
class ICommand
{
public:
    virtual ~ICommand() {}
    virtual void Execute(CTextDocument* pDocument) = 0;
    virtual void Undo(CTextDocument* pDocument) = 0;
    virtual void Redo(CTextDocument* pDocument) = 0;
};

// テキスト挿入コマンド
class CInsertTextCommand : public ICommand
{
public:
    CInsertTextCommand(const TextPosition& pos, const std::wstring& text)
        : m_position(pos), m_text(text) {}

    void Execute(CTextDocument* pDocument) override;
    void Undo(CTextDocument* pDocument) override;
    void Redo(CTextDocument* pDocument) override;

private:
    TextPosition m_position;
    std::wstring m_text;
    TextPosition m_endPosition;
};

// テキスト削除コマンド
class CDeleteTextCommand : public ICommand
{
public:
    CDeleteTextCommand(const TextPosition& start, const TextPosition& end)
        : m_start(start), m_end(end) {}

    void Execute(CTextDocument* pDocument) override;
    void Undo(CTextDocument* pDocument) override;
    void Redo(CTextDocument* pDocument) override;

private:
    TextPosition m_start;
    TextPosition m_end;
    std::wstring m_deletedText;
};

// テキスト置換コマンド
class CReplaceTextCommand : public ICommand
{
public:
    CReplaceTextCommand(const TextPosition& start, const TextPosition& end, const std::wstring& newText)
        : m_start(start), m_end(end), m_newText(newText) {}

    void Execute(CTextDocument* pDocument) override;
    void Undo(CTextDocument* pDocument) override;
    void Redo(CTextDocument* pDocument) override;

private:
    TextPosition m_start;
    TextPosition m_end;
    std::wstring m_oldText;
    std::wstring m_newText;
};

// Undo/Redo管理クラス
class CUndoManager
{
public:
    CUndoManager();
    ~CUndoManager();

    // コマンド実行
    void ExecuteCommand(std::unique_ptr<ICommand> command, CTextDocument* pDocument);

    // Undo/Redo
    bool CanUndo() const;
    bool CanRedo() const;
    void Undo(CTextDocument* pDocument);
    void Redo(CTextDocument* pDocument);

    // 履歴管理
    void Clear();
    size_t GetUndoCount() const { return m_currentIndex; }
    size_t GetRedoCount() const { return m_commands.size() - m_currentIndex; }

    // メモリ管理
    void SetMaxMemoryUsage(size_t bytes) { m_maxMemoryUsage = bytes; }
    size_t GetEstimatedMemoryUsage() const { return m_estimatedMemoryUsage; }

private:
    void TrimMemoryIfNeeded();
    size_t EstimateCommandSize(const ICommand* pCommand) const;

    std::vector<std::unique_ptr<ICommand>> m_commands;
    size_t m_currentIndex;
    size_t m_maxMemoryUsage;
    size_t m_estimatedMemoryUsage;
};
