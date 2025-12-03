// TextDocument.cpp - テキストドキュメント実装
#include "TextDocument.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cstring>

// --- Encoding helpers (save) ---
static bool ConvertWideToUtf8(const std::wstring& text, std::string& out)
{
    out.clear();
    if (text.empty())
    {
        out = "";
        return true;
    }

    int needed = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), NULL, 0, NULL, NULL);
    if (needed <= 0)
    {
        return false;
    }
    out.resize(static_cast<size_t>(needed));
    int written = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), &out[0], needed, NULL, NULL);
    if (written <= 0)
    {
        out.clear();
        return false;
    }
    return true;
}

// --- Encoding helpers ---
static bool TryConvertMultiByte(int codePage, DWORD flags, const char* bytes, int byteLen, std::wstring& out)
{
    out.clear();
    if (byteLen == 0)
    {
        out = L"";
        return true;
    }

    int needed = MultiByteToWideChar(codePage, flags, bytes, byteLen, NULL, 0);
    if (needed <= 0)
    {
        return false;
    }
    out.resize(static_cast<size_t>(needed));
    int written = MultiByteToWideChar(codePage, flags, bytes, byteLen, &out[0], needed);
    if (written <= 0)
    {
        out.clear();
        return false;
    }
    return true;
}

static std::wstring ConvertBytesToWide(const char* bytes, size_t len)
{
    // BOM check
    if (len >= 3 && static_cast<unsigned char>(bytes[0]) == 0xEF && static_cast<unsigned char>(bytes[1]) == 0xBB && static_cast<unsigned char>(bytes[2]) == 0xBF)
    {
        std::wstring out;
        if (TryConvertMultiByte(CP_UTF8, MB_ERR_INVALID_CHARS, bytes + 3, static_cast<int>(len - 3), out))
            return out;
        if (TryConvertMultiByte(CP_UTF8, 0, bytes + 3, static_cast<int>(len - 3), out))
            return out;
    }
    if (len >= 2 && static_cast<unsigned char>(bytes[0]) == 0xFF && static_cast<unsigned char>(bytes[1]) == 0xFE)
    {
        // UTF-16LE
        size_t wcharCount = (len - 2) / 2;
        std::wstring out;
        out.resize(wcharCount);
        if (wcharCount > 0)
        {
            std::memcpy(&out[0], bytes + 2, wcharCount * sizeof(wchar_t));
        }
        return out;
    }
    if (len >= 2 && static_cast<unsigned char>(bytes[0]) == 0xFE && static_cast<unsigned char>(bytes[1]) == 0xFF)
    {
        // UTF-16BE → swap to LE
        size_t wcharCount = (len - 2) / 2;
        std::wstring out;
        out.resize(wcharCount);
        for (size_t i = 0; i < wcharCount; ++i)
        {
            unsigned char hi = static_cast<unsigned char>(bytes[2 + i * 2]);
            unsigned char lo = static_cast<unsigned char>(bytes[2 + i * 2 + 1]);
            wchar_t wc = static_cast<wchar_t>((lo << 8) | hi);
            out[i] = wc;
        }
        return out;
    }

    // No BOM: try UTF-8 strict → lax → ANSI
    std::wstring out;
    if (TryConvertMultiByte(CP_UTF8, MB_ERR_INVALID_CHARS, bytes, static_cast<int>(len), out))
        return out;
    if (TryConvertMultiByte(CP_UTF8, 0, bytes, static_cast<int>(len), out))
        return out;
    if (TryConvertMultiByte(CP_ACP, 0, bytes, static_cast<int>(len), out))
        return out;

    return L"";
}

const size_t MEMORY_MAPPED_THRESHOLD = 10 * 1024 * 1024; // 10MB以上でメモリマップド使用

CTextDocument::CTextDocument()
    : m_hFile(INVALID_HANDLE_VALUE)
    , m_hMapping(NULL)
    , m_pView(nullptr)
    , m_fileSize(0)
{
    m_lines.push_back(L""); // 空のドキュメントでも1行は存在
}

CTextDocument::~CTextDocument()
{
    if (m_pView)
    {
        UnmapViewOfFile(m_pView);
    }
    if (m_hMapping)
    {
        CloseHandle(m_hMapping);
    }
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
    }
}

bool CTextDocument::LoadFromFile(const wchar_t* filePath)
{
    // ファイルサイズを取得
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (!GetFileAttributesEx(filePath, GetFileExInfoStandard, &fileInfo))
    {
        return false;
    }

    ULARGE_INTEGER fileSize;
    fileSize.LowPart = fileInfo.nFileSizeLow;
    fileSize.HighPart = fileInfo.nFileSizeHigh;
    m_fileSize = static_cast<size_t>(fileSize.QuadPart);

    // ファイルサイズに応じて読み込み方法を選択
    if (m_fileSize > MEMORY_MAPPED_THRESHOLD)
    {
        LoadFromMemoryMappedFile(filePath);
    }
    else
    {
        LoadFromRegularFile(filePath);
    }

    return true;
}

void CTextDocument::LoadFromMemoryMappedFile(const wchar_t* filePath)
{
    // メモリマップドファイルを開く
    m_hFile = CreateFile(
        filePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

    m_hMapping = CreateFileMapping(
        m_hFile,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL
    );

    if (!m_hMapping)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
        return;
    }

    m_pView = MapViewOfFile(
        m_hMapping,
        FILE_MAP_READ,
        0,
        0,
        0
    );

    if (!m_pView)
    {
        CloseHandle(m_hMapping);
        CloseHandle(m_hFile);
        m_hMapping = NULL;
        m_hFile = INVALID_HANDLE_VALUE;
        return;
    }

    // UTF-8として読み込み（簡易実装）
    const char* bytes = static_cast<const char*>(m_pView);
    
    // UTF-8からUTF-16に変換
    std::wstring wideText = ConvertBytesToWide(bytes, m_fileSize);

    SplitIntoLines(wideText);
}

void CTextDocument::LoadFromRegularFile(const wchar_t* filePath)
{
    // Byte-based load with encoding detection (BOM/UTF-8/ANSI)
    std::ifstream s(filePath, std::ios::binary);
    if (s.is_open()) {
        s.seekg(0, std::ios::end);
        std::streamoff sz = s.tellg();
        s.seekg(0, std::ios::beg);
        if (sz > 0) {
            std::vector<char> buf(static_cast<size_t>(sz));
            s.read(buf.data(), sz);
            s.close();
            std::wstring text = ConvertBytesToWide(buf.data(), buf.size());
            SplitIntoLines(text);
            return;
        }
        s.close();
        SplitIntoLines(L"");
        return;
    }
    std::wifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return;
    }

    // BOMをチェック
    wchar_t bom;
    file.read(&bom, 1);
    if (bom != 0xFEFF)
    {
        file.seekg(0); // BOMがない場合は先頭に戻る
    }

    std::wstringstream buffer;
    buffer << file.rdbuf();
    file.close();

    SplitIntoLines(buffer.str());
}

void CTextDocument::SplitIntoLines(const std::wstring& text)
{
    m_lines.clear();
    
    std::wstring line;
    for (size_t i = 0; i < text.length(); ++i)
    {
        wchar_t ch = text[i];
        
        if (ch == L'\r')
        {
            if (i + 1 < text.length() && text[i + 1] == L'\n')
            {
                ++i; // \r\nをスキップ
            }
            m_lines.push_back(line);
            line.clear();
        }
        else if (ch == L'\n')
        {
            m_lines.push_back(line);
            line.clear();
        }
        else
        {
            line += ch;
        }
    }
    
    m_lines.push_back(line); // 最後の行
    
    if (m_lines.empty())
    {
        m_lines.push_back(L"");
    }
}

bool CTextDocument::SaveToFile(const wchar_t* filePath)
{
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }

    // BOMを書き込み
    const unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };
    file.write(reinterpret_cast<const char*>(bom), 3);
    if (!file)
    {
        return false;
    }

    // 各行を書き込み
    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        {
            std::string utf8;
            if (!ConvertWideToUtf8(m_lines[i], utf8))
            {
                return false;
            }
            if (!utf8.empty())
            {
                file.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));
                if (!file)
                {
                    return false;
                }
            }
        }
        if (i < m_lines.size() - 1)
        {
            file.write("\r\n", 2);
            if (!file)
            {
                return false;
            }
        }
    }

    file.close();
    return true;
}

void CTextDocument::Clear()
{
    m_lines.clear();
    m_lines.push_back(L"");
}

const std::wstring& CTextDocument::GetLine(size_t index) const
{
    static std::wstring empty;
    if (index >= m_lines.size())
    {
        return empty;
    }
    return m_lines[index];
}

std::wstring CTextDocument::GetText() const
{
    std::wstring result;
    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        result += m_lines[i];
        if (i < m_lines.size() - 1)
        {
            result += L"\r\n";
        }
    }
    return result;
}

std::wstring CTextDocument::GetTextRange(const TextPosition& start, const TextPosition& end) const
{
    if (start == end)
    {
        return L"";
    }

    TextPosition actualStart = start < end ? start : end;
    TextPosition actualEnd = start < end ? end : start;

    std::wstring result;

    if (actualStart.line == actualEnd.line)
    {
        // 同じ行内
        const std::wstring& line = GetLine(actualStart.line);
        size_t startCol = std::min(actualStart.column, line.length());
        size_t endCol = std::min(actualEnd.column, line.length());
        result = line.substr(startCol, endCol - startCol);
    }
    else
    {
        // 複数行にまたがる
        for (size_t i = actualStart.line; i <= actualEnd.line && i < m_lines.size(); ++i)
        {
            const std::wstring& line = GetLine(i);
            
            if (i == actualStart.line)
            {
                size_t startCol = std::min(actualStart.column, line.length());
                result += line.substr(startCol);
            }
            else if (i == actualEnd.line)
            {
                size_t endCol = std::min(actualEnd.column, line.length());
                result += line.substr(0, endCol);
            }
            else
            {
                result += line;
            }
            
            if (i < actualEnd.line)
            {
                result += L"\r\n";
            }
        }
    }

    return result;
}

void CTextDocument::InsertChar(const TextPosition& pos, wchar_t ch)
{
    TextPosition clampedPos = ClampPosition(pos);
    
    if (ch == L'\r' || ch == L'\n')
    {
        // 改行
        std::wstring& line = m_lines[clampedPos.line];
        std::wstring newLine = line.substr(clampedPos.column);
        line = line.substr(0, clampedPos.column);
        m_lines.insert(m_lines.begin() + clampedPos.line + 1, newLine);
    }
    else
    {
        // 通常の文字
        std::wstring& line = m_lines[clampedPos.line];
        line.insert(clampedPos.column, 1, ch);
    }
}

void CTextDocument::InsertText(const TextPosition& pos, const std::wstring& text)
{
    if (text.empty())
    {
        return;
    }

    TextPosition clampedPos = ClampPosition(pos);
    
    // テキストを行に分割
    std::vector<std::wstring> newLines;
    std::wstring line;
    for (wchar_t ch : text)
    {
        if (ch == L'\r')
        {
            continue; // \rは無視
        }
        else if (ch == L'\n')
        {
            newLines.push_back(line);
            line.clear();
        }
        else
        {
            line += ch;
        }
    }
    newLines.push_back(line);

    // 挿入
    std::wstring& currentLine = m_lines[clampedPos.line];
    std::wstring beforeInsert = currentLine.substr(0, clampedPos.column);
    std::wstring afterInsert = currentLine.substr(clampedPos.column);

    if (newLines.size() == 1)
    {
        // 単一行
        currentLine = beforeInsert + newLines[0] + afterInsert;
    }
    else
    {
        // 複数行
        currentLine = beforeInsert + newLines[0];
        for (size_t i = 1; i < newLines.size() - 1; ++i)
        {
            m_lines.insert(m_lines.begin() + clampedPos.line + i, newLines[i]);
        }
        m_lines.insert(m_lines.begin() + clampedPos.line + newLines.size() - 1, 
                      newLines.back() + afterInsert);
    }
}

void CTextDocument::DeleteChar(const TextPosition& pos)
{
    if (!IsValidPosition(pos))
    {
        return;
    }

    std::wstring& line = m_lines[pos.line];
    
    if (pos.column < line.length())
    {
        // 行内の文字を削除
        line.erase(pos.column, 1);
    }
    else if (pos.line < m_lines.size() - 1)
    {
        // 次の行と結合
        line += m_lines[pos.line + 1];
        m_lines.erase(m_lines.begin() + pos.line + 1);
    }
}

void CTextDocument::DeleteRange(const TextPosition& start, const TextPosition& end)
{
    if (start == end)
    {
        return;
    }

    TextPosition actualStart = start < end ? start : end;
    TextPosition actualEnd = start < end ? end : start;

    if (actualStart.line == actualEnd.line)
    {
        // 同じ行内
        std::wstring& line = m_lines[actualStart.line];
        size_t startCol = std::min(actualStart.column, line.length());
        size_t endCol = std::min(actualEnd.column, line.length());
        line.erase(startCol, endCol - startCol);
    }
    else
    {
        // 複数行にまたがる
        std::wstring& startLine = m_lines[actualStart.line];
        const std::wstring& endLine = GetLine(actualEnd.line);
        
        startLine = startLine.substr(0, actualStart.column) + 
                   endLine.substr(std::min(actualEnd.column, endLine.length()));
        
        m_lines.erase(m_lines.begin() + actualStart.line + 1, 
                     m_lines.begin() + std::min(actualEnd.line + 1, m_lines.size()));
    }
}

void CTextDocument::ReplaceRange(const TextPosition& start, const TextPosition& end, const std::wstring& text)
{
    DeleteRange(start, end);
    InsertText(start, text);
}

TextPosition CTextDocument::ClampPosition(const TextPosition& pos) const
{
    TextPosition result = pos;
    
    if (result.line >= m_lines.size())
    {
        result.line = m_lines.size() - 1;
    }
    
    const std::wstring& line = m_lines[result.line];
    if (result.column > line.length())
    {
        result.column = line.length();
    }
    
    return result;
}

bool CTextDocument::IsValidPosition(const TextPosition& pos) const
{
    if (pos.line >= m_lines.size())
    {
        return false;
    }
    
    return pos.column <= m_lines[pos.line].length();
}
