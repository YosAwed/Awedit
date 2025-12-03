// SearchEngine.cpp - 検索・置換エンジン実装
#include "SearchEngine.h"
#include <algorithm>

CSearchEngine::CSearchEngine()
{
}

CSearchEngine::~CSearchEngine()
{
}

bool CSearchEngine::Find(CTextDocument* pDocument, const std::wstring& pattern, 
                        const TextPosition& startPos, SearchResult& result)
{
    if (!pDocument || pattern.empty())
    {
        return false;
    }

    m_currentPattern = pattern;
    m_lastSearchPos = startPos;

    // 開始位置から検索
    for (size_t line = startPos.line; line < pDocument->GetLineCount(); ++line)
    {
        const std::wstring& lineText = pDocument->GetLine(line);
        size_t startCol = (line == startPos.line) ? startPos.column : 0;
        size_t endCol = 0;

        if (SearchInLine(lineText, pattern, startCol, endCol))
        {
            result.start = TextPosition(line, startCol);
            result.end = TextPosition(line, endCol);
            result.matchedText = lineText.substr(startCol, endCol - startCol);
            m_lastSearchPos = result.end;
            return true;
        }
    }

    // ラップアラウンド
    if (m_options.wrapAround && startPos.line > 0)
    {
        for (size_t line = 0; line < startPos.line; ++line)
        {
            const std::wstring& lineText = pDocument->GetLine(line);
            size_t startCol = 0;
            size_t endCol = 0;

            if (SearchInLine(lineText, pattern, startCol, endCol))
            {
                result.start = TextPosition(line, startCol);
                result.end = TextPosition(line, endCol);
                result.matchedText = lineText.substr(startCol, endCol - startCol);
                m_lastSearchPos = result.end;
                return true;
            }
        }
    }

    return false;
}

bool CSearchEngine::FindNext(CTextDocument* pDocument, SearchResult& result)
{
    if (m_currentPattern.empty())
    {
        return false;
    }

    TextPosition nextPos = m_lastSearchPos;
    nextPos.column++; // 次の位置から検索

    return Find(pDocument, m_currentPattern, nextPos, result);
}

bool CSearchEngine::FindPrevious(CTextDocument* pDocument, SearchResult& result)
{
    if (!pDocument || m_currentPattern.empty())
    {
        return false;
    }

    // 逆方向検索（簡易実装）
    TextPosition searchPos = m_lastSearchPos;
    
    if (searchPos.column > 0)
    {
        searchPos.column--;
    }
    else if (searchPos.line > 0)
    {
        searchPos.line--;
        searchPos.column = pDocument->GetLine(searchPos.line).length();
    }
    else
    {
        return false;
    }

    // 現在位置から前方を検索
    for (int line = static_cast<int>(searchPos.line); line >= 0; --line)
    {
        const std::wstring& lineText = pDocument->GetLine(line);
        
        // 行内を後ろから検索
        for (int col = static_cast<int>(lineText.length()) - 1; col >= 0; --col)
        {
            size_t startCol = static_cast<size_t>(col);
            size_t endCol = 0;

            if (SearchInLine(lineText, m_currentPattern, startCol, endCol))
            {
                result.start = TextPosition(line, startCol);
                result.end = TextPosition(line, endCol);
                result.matchedText = lineText.substr(startCol, endCol - startCol);
                m_lastSearchPos = result.start;
                return true;
            }
        }
    }

    return false;
}

std::vector<SearchResult> CSearchEngine::FindAll(CTextDocument* pDocument, const std::wstring& pattern)
{
    std::vector<SearchResult> results;

    if (!pDocument || pattern.empty())
    {
        return results;
    }

    m_currentPattern = pattern;

    for (size_t line = 0; line < pDocument->GetLineCount(); ++line)
    {
        const std::wstring& lineText = pDocument->GetLine(line);
        size_t searchPos = 0;

        while (searchPos < lineText.length())
        {
            size_t startCol = searchPos;
            size_t endCol = 0;

            if (SearchInLine(lineText, pattern, startCol, endCol))
            {
                SearchResult result;
                result.start = TextPosition(line, startCol);
                result.end = TextPosition(line, endCol);
                result.matchedText = lineText.substr(startCol, endCol - startCol);
                results.push_back(result);
                
                searchPos = endCol + 1;
            }
            else
            {
                break;
            }
        }
    }

    return results;
}

bool CSearchEngine::Replace(CTextDocument* pDocument, const SearchResult& result, const std::wstring& replacement)
{
    if (!pDocument)
    {
        return false;
    }

    pDocument->ReplaceRange(result.start, result.end, replacement);
    return true;
}

int CSearchEngine::ReplaceAll(CTextDocument* pDocument, const std::wstring& pattern, const std::wstring& replacement)
{
    std::vector<SearchResult> results = FindAll(pDocument, pattern);
    
    // 後ろから置換して位置ずれを防ぐ
    for (auto it = results.rbegin(); it != results.rend(); ++it)
    {
        Replace(pDocument, *it, replacement);
    }

    return static_cast<int>(results.size());
}

bool CSearchEngine::SearchInLine(const std::wstring& line, const std::wstring& pattern, 
                                size_t& startCol, size_t& endCol)
{
    if (m_options.useRegex)
    {
        // 正規表現検索
        std::wstring searchText = line.substr(startCol);
        size_t relativeStart = 0;
        size_t relativeEnd = 0;

        if (SearchWithRegex(searchText, pattern, relativeStart, relativeEnd))
        {
            startCol += relativeStart;
            endCol = startCol + (relativeEnd - relativeStart);
            return true;
        }
        return false;
    }
    else
    {
        // 通常の検索
        std::wstring searchPattern = pattern;
        std::wstring searchText = line.substr(startCol);

        // 大文字小文字を区別しない場合
        if (!m_options.caseSensitive)
        {
            std::transform(searchPattern.begin(), searchPattern.end(), searchPattern.begin(), ::towlower);
            std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::towlower);
        }

        size_t pos = searchText.find(searchPattern);
        if (pos != std::wstring::npos)
        {
            startCol += pos;
            endCol = startCol + pattern.length();

            // 単語単位検索のチェック
            if (m_options.wholeWord)
            {
                bool startOk = (startCol == 0) || !iswalnum(line[startCol - 1]);
                bool endOk = (endCol >= line.length()) || !iswalnum(line[endCol]);
                
                if (!startOk || !endOk)
                {
                    startCol = endCol;
                    return SearchInLine(line, pattern, startCol, endCol);
                }
            }

            return true;
        }
    }

    return false;
}

bool CSearchEngine::SearchWithRegex(const std::wstring& text, const std::wstring& pattern,
                                   size_t& start, size_t& end)
{
    try
    {
        std::wregex::flag_type flags = std::wregex::ECMAScript;
        if (!m_options.caseSensitive)
        {
            flags |= std::wregex::icase;
        }

        std::wstring actualPattern = pattern;
        if (m_options.wholeWord)
        {
            actualPattern = CreateWordBoundaryPattern(pattern);
        }

        std::wregex regex(actualPattern, flags);
        std::wsmatch match;

        if (std::regex_search(text, match, regex))
        {
            start = match.position();
            end = start + match.length();
            return true;
        }
    }
    catch (const std::regex_error&)
    {
        // 正規表現エラーの場合は失敗
        return false;
    }

    return false;
}

std::wstring CSearchEngine::CreateWordBoundaryPattern(const std::wstring& pattern)
{
    return L"\\b" + pattern + L"\\b";
}
