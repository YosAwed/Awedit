// SearchEngine.h - 検索・置換エンジン（正規表現対応）
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <regex>
#include "TextDocument.h"

// 検索オプション
struct SearchOptions
{
    bool useRegex;
    bool caseSensitive;
    bool wholeWord;
    bool wrapAround;

    SearchOptions()
        : useRegex(false)
        , caseSensitive(false)
        , wholeWord(false)
        , wrapAround(true)
    {}
};

// 検索結果
struct SearchResult
{
    TextPosition start;
    TextPosition end;
    std::wstring matchedText;

    SearchResult() {}
    SearchResult(const TextPosition& s, const TextPosition& e, const std::wstring& text)
        : start(s), end(e), matchedText(text) {}
};

class CSearchEngine
{
public:
    CSearchEngine();
    ~CSearchEngine();

    // 検索
    bool Find(CTextDocument* pDocument, const std::wstring& pattern, 
             const TextPosition& startPos, SearchResult& result);
    bool FindNext(CTextDocument* pDocument, SearchResult& result);
    bool FindPrevious(CTextDocument* pDocument, SearchResult& result);
    std::vector<SearchResult> FindAll(CTextDocument* pDocument, const std::wstring& pattern);

    // 置換
    bool Replace(CTextDocument* pDocument, const SearchResult& result, const std::wstring& replacement);
    int ReplaceAll(CTextDocument* pDocument, const std::wstring& pattern, const std::wstring& replacement);

    // オプション設定
    void SetOptions(const SearchOptions& options) { m_options = options; }
    const SearchOptions& GetOptions() const { return m_options; }

    // 現在の検索パターン
    void SetPattern(const std::wstring& pattern) { m_currentPattern = pattern; }
    const std::wstring& GetPattern() const { return m_currentPattern; }

private:
    bool SearchInLine(const std::wstring& line, const std::wstring& pattern, 
                     size_t& startCol, size_t& endCol);
    bool SearchWithRegex(const std::wstring& text, const std::wstring& pattern,
                        size_t& start, size_t& end);
    std::wstring CreateWordBoundaryPattern(const std::wstring& pattern);

    SearchOptions m_options;
    std::wstring m_currentPattern;
    TextPosition m_lastSearchPos;
};
