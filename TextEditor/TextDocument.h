// TextDocument.h - テキストドキュメント管理
#pragma once
#include <windows.h>
#include <string>
#include <vector>

// テキスト位置を表す構造体
struct TextPosition
{
    size_t line;
    size_t column;

    TextPosition() : line(0), column(0) {}
    TextPosition(size_t l, size_t c) : line(l), column(c) {}

    bool operator==(const TextPosition& other) const
    {
        return line == other.line && column == other.column;
    }

    bool operator<(const TextPosition& other) const
    {
        if (line != other.line)
            return line < other.line;
        return column < other.column;
    }
};

class CTextDocument
{
public:
    CTextDocument();
    ~CTextDocument();

    // ファイル操作
    bool LoadFromFile(const wchar_t* filePath);
    bool SaveToFile(const wchar_t* filePath);
    void Clear();

    // テキスト取得
    size_t GetLineCount() const { return m_lines.size(); }
    const std::wstring& GetLine(size_t index) const;
    std::wstring GetText() const;
    std::wstring GetTextRange(const TextPosition& start, const TextPosition& end) const;

    // テキスト編集
    void InsertChar(const TextPosition& pos, wchar_t ch);
    void InsertText(const TextPosition& pos, const std::wstring& text);
    void DeleteChar(const TextPosition& pos);
    void DeleteRange(const TextPosition& start, const TextPosition& end);
    void ReplaceRange(const TextPosition& start, const TextPosition& end, const std::wstring& text);

    // ユーティリティ
    TextPosition ClampPosition(const TextPosition& pos) const;
    bool IsValidPosition(const TextPosition& pos) const;

private:
    void LoadFromMemoryMappedFile(const wchar_t* filePath);
    void LoadFromRegularFile(const wchar_t* filePath);
    void SplitIntoLines(const std::wstring& text);

    std::vector<std::wstring> m_lines;
    
    // メモリマップドファイル用
    HANDLE m_hFile;
    HANDLE m_hMapping;
    LPVOID m_pView;
    size_t m_fileSize;
};
