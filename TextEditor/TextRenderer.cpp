// TextRenderer.cpp - DirectWriteテキストレンダラー実装
#include "TextRenderer.h"
#include <algorithm>
#include <cmath>
#include <vector>

CTextRenderer::CTextRenderer()
    : m_hwnd(nullptr)
    , m_pD2DFactory(nullptr)
    , m_pRenderTarget(nullptr)
    , m_pTextBrush(nullptr)
    , m_pSelectionBrush(nullptr)
    , m_pCursorBrush(nullptr)
    , m_pCurrentLineUnderlineBrush(nullptr)
    , m_pBackgroundBrush(nullptr)
    , m_pImeTargetBrush(nullptr)
    , m_pDWriteFactory(nullptr)
    , m_pTextFormat(nullptr)
    , m_scrollOffsetY(0)
    , m_scrollOffsetX(0)
    , m_fontSize(14.0f)
    , m_lineHeight(20.0f)
    , m_charWidth(8.0f)
    , m_viewportWidth(0)
    , m_viewportHeight(0)
{
}

CTextRenderer::~CTextRenderer()
{
    DiscardDeviceResources();

    if (m_pTextFormat)
    {
        m_pTextFormat->Release();
    }
    if (m_pDWriteFactory)
    {
        m_pDWriteFactory->Release();
    }
    if (m_pD2DFactory)
    {
        m_pD2DFactory->Release();
    }
}

bool CTextRenderer::Initialize(HWND hwnd)
{
    m_hwnd = hwnd;

    // Direct2Dファクトリーの作成
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &m_pD2DFactory
    );

    if (FAILED(hr))
    {
        return false;
    }

    // DirectWriteファクトリーの作成
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
    );

    if (FAILED(hr))
    {
        return false;
    }

    // テキストフォーマットの作成
    hr = m_pDWriteFactory->CreateTextFormat(
        L"Consolas",
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_fontSize,
        L"ja-jp",
        &m_pTextFormat
    );

    if (FAILED(hr))
    {
        return false;
    }

    m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    // 既定は NO_WRAP のまま。折り返しは TextLayout 毎に設定する。

    // デバイスリソースの作成
    CreateDeviceResources();

    return true;
}

void CTextRenderer::CreateDeviceResources()
{
    if (m_pRenderTarget)
    {
        return;
    }

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(
        rc.right - rc.left,
        rc.bottom - rc.top
    );

    HRESULT hr = m_pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hwnd, size),
        &m_pRenderTarget
    );

    if (SUCCEEDED(hr))
    {
        // ブラシの作成
        m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &m_pTextBrush
        );

        m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(0.7f, 0.85f, 1.0f, 0.5f),
            &m_pSelectionBrush
        );

        m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &m_pCursorBrush
        );

        // Current line underline brush (subtle blue)
        m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(0.60f, 0.80f, 1.0f, 0.8f),
            &m_pCurrentLineUnderlineBrush
        );

        // Background brush (white)
        m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            &m_pBackgroundBrush
        );

        // IME target highlight brush (accent color)
        m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(0.10f, 0.45f, 0.90f, 0.75f),
            &m_pImeTargetBrush
        );

        m_viewportWidth = size.width;
        m_viewportHeight = size.height;
    }
}

void CTextRenderer::DiscardDeviceResources()
{
    if (m_pImeTargetBrush)
    {
        m_pImeTargetBrush->Release();
        m_pImeTargetBrush = nullptr;
    }
    if (m_pBackgroundBrush)
    {
        m_pBackgroundBrush->Release();
        m_pBackgroundBrush = nullptr;
    }
    if (m_pCurrentLineUnderlineBrush)
    {
        m_pCurrentLineUnderlineBrush->Release();
        m_pCurrentLineUnderlineBrush = nullptr;
    }
    if (m_pCursorBrush)
    {
        m_pCursorBrush->Release();
        m_pCursorBrush = nullptr;
    }
    if (m_pSelectionBrush)
    {
        m_pSelectionBrush->Release();
        m_pSelectionBrush = nullptr;
    }
    if (m_pTextBrush)
    {
        m_pTextBrush->Release();
        m_pTextBrush = nullptr;
    }
    if (m_pRenderTarget)
    {
        m_pRenderTarget->Release();
        m_pRenderTarget = nullptr;
    }
}

void CTextRenderer::Resize(int width, int height)
{
    if (m_pRenderTarget)
    {
        D2D1_SIZE_U size = D2D1::SizeU(width, height);
        m_pRenderTarget->Resize(size);
        m_viewportWidth = width;
        m_viewportHeight = height;
    }
}

void CTextRenderer::SetFontSize(float size)
{
    if (size <= 6.0f) size = 6.0f;
    if (size > 200.0f) size = 200.0f;
    m_fontSize = size;

    // Recreate text format with new size
    if (m_pTextFormat)
    {
        m_pTextFormat->Release();
        m_pTextFormat = nullptr;
    }
    if (m_pDWriteFactory)
    {
        HRESULT hr = m_pDWriteFactory->CreateTextFormat(
            L"Consolas",
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            m_fontSize,
            L"ja-jp",
            &m_pTextFormat
        );
        if (SUCCEEDED(hr) && m_pTextFormat)
        {
            m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
            m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        }
    }

    // Estimate line height using a simple layout
    if (m_pDWriteFactory && m_pTextFormat)
    {
        IDWriteTextLayout* pLayout = nullptr;
        if (SUCCEEDED(m_pDWriteFactory->CreateTextLayout(L"Ag", 2, m_pTextFormat, 10000.0f, 1000.0f, &pLayout)) && pLayout)
        {
            DWRITE_TEXT_METRICS m{};
            if (SUCCEEDED(pLayout->GetMetrics(&m)))
            {
                m_lineHeight = m.height;
            }
            pLayout->Release();
        }
        else
        {
            // Fallback heuristic
            m_lineHeight = m_fontSize * 1.4f;
        }
    }
}

void CTextRenderer::Render(CTextDocument* pDocument, CEditController* pEditController, const CompositionInfo* pImeInfo)
{
    if (!m_pRenderTarget || !pDocument)
    {
        return;
    }

    CreateDeviceResources();

    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

    // 表示範囲の計算
    size_t firstVisibleLine = static_cast<size_t>(std::max(0, m_scrollOffsetY) / m_lineHeight);
    size_t lastVisibleLine = std::min(
        pDocument->GetLineCount(),
        firstVisibleLine + static_cast<size_t>(m_viewportHeight / m_lineHeight) + 2
    );

    // 選択範囲の描画
    if (pEditController && pEditController->HasSelection())
    {
        TextPosition selStart, selEnd;
        pEditController->GetSelection(selStart, selEnd);
        RenderSelection(selStart, selEnd, pDocument);
    }

    // テキストの描画
    // 折り返し: DrawTextLayout で可変行高描画
    {
        constexpr float kLeftPadding = 5.0f;
        float availableWidth = static_cast<float>(m_viewportWidth) - kLeftPadding + static_cast<float>(m_scrollOffsetX);
        if (availableWidth <= 0.0f) availableWidth = 1.0f;

        float currentY = -static_cast<float>(m_scrollOffsetY);
        bool imeActive = (pImeInfo && !pImeInfo->text.empty() && pEditController && !pEditController->GetCursors().empty());
        size_t imeLine = 0;
        size_t imeCol = 0;
        if (imeActive)
        {
            const auto& cs = pEditController->GetCursors();
            imeLine = cs[0].line;
            imeCol = cs[0].column;
        }
        for (size_t i = 0; i < pDocument->GetLineCount(); ++i)
        {
            const std::wstring& srcLine = pDocument->GetLine(i);
            std::wstring line = srcLine;
            if (imeActive && i == imeLine)
            {
                size_t col = std::min(imeCol, srcLine.length());
                line = srcLine.substr(0, col) + pImeInfo->text + srcLine.substr(col);
            }
            IDWriteTextLayout* pLayout = CreateTextLayoutForLine(line, availableWidth);
            if (!pLayout) continue;

            DWRITE_TEXT_METRICS metrics{};
            if (SUCCEEDED(pLayout->GetMetrics(&metrics)))
            {
                float totalHeight = metrics.height;
                if (currentY + totalHeight >= 0 && currentY <= m_viewportHeight)
                {
                    D2D1_POINT_2F origin = D2D1::Point2F(kLeftPadding - static_cast<float>(m_scrollOffsetX), currentY);
                    if (imeActive && i == imeLine)
                    {
                        DWRITE_TEXT_RANGE r{ static_cast<UINT32>(std::min(imeCol, srcLine.length())), static_cast<UINT32>(pImeInfo->text.length()) };
                        pLayout->SetUnderline(TRUE, r);
                        if (pImeInfo->targetLength > 0)
                        {
                            UINT32 tStart = static_cast<UINT32>(std::min(imeCol, srcLine.length())) + pImeInfo->targetStart;
                            UINT32 tLen = pImeInfo->targetLength;
                            UINT32 actualCount = 0;
                            DWRITE_HIT_TEST_METRICS tmp[16];
                            HRESULT hr = pLayout->HitTestTextRange(tStart, tLen, 0.0f, 0.0f, tmp, 16, &actualCount);
                            std::vector<DWRITE_HIT_TEST_METRICS> frags;
                            if (hr == E_NOT_SUFFICIENT_BUFFER)
                            {
                                frags.resize(actualCount);
                                hr = pLayout->HitTestTextRange(tStart, tLen, 0.0f, 0.0f, frags.data(), static_cast<UINT32>(frags.size()), &actualCount);
                            }
                            else if (SUCCEEDED(hr))
                            {
                                frags.assign(tmp, tmp + actualCount);
                            }
                            if (SUCCEEDED(hr))
                            {
                                for (UINT32 fi = 0; fi < actualCount; ++fi)
                                {
                                    const auto& f = frags[fi];
                                    float x1 = origin.x + f.left;
                                    float x2 = x1 + f.width;
                                    float y1 = origin.y + f.top + f.height - 2.0f;
                                    float y2 = origin.y + f.top + f.height;
                                    D2D1_RECT_F band = D2D1::RectF(x1, y1, x2, y2);
                                    m_pRenderTarget->FillRectangle(band, m_pImeTargetBrush);
                                }
                            }
                        }
                    }
                    m_pRenderTarget->DrawTextLayout(origin, pLayout, m_pTextBrush);
                }
                currentY += totalHeight;
            }

            pLayout->Release();
            if (currentY > m_viewportHeight) break;
        }
    }

    // カーソルとIME入力中の文字列の描画
    if (pEditController)
    {
        const auto& cursors = pEditController->GetCursors();

        // Underline the current cursor line(s)
        if (m_pCurrentLineUnderlineBrush && !cursors.empty())
        {
            for (const auto& cursorPos : cursors)
            {
                POINT pt = TextPositionToScreen(cursorPos, pDocument);

                float lineHeight = m_lineHeight;
                const std::wstring& lineText = pDocument->GetLine(cursorPos.line);
                constexpr float kLeftPadding = 5.0f;
                float availableWidth = static_cast<float>(m_viewportWidth) - kLeftPadding + static_cast<float>(m_scrollOffsetX);
                if (availableWidth <= 0.0f) availableWidth = 1.0f;
                IDWriteTextLayout* pLayout = CreateTextLayoutForLine(lineText, availableWidth);
                if (pLayout)
                {
                    DWRITE_HIT_TEST_METRICS h{};
                    FLOAT x = 0, y = 0;
                    if (SUCCEEDED(pLayout->HitTestTextPosition(static_cast<UINT32>(std::min(cursorPos.column, lineText.length())), FALSE, &x, &y, &h)))
                    {
                        lineHeight = h.height;
                    }
                    pLayout->Release();
                }

                float underlineY = static_cast<float>(pt.y) + lineHeight - 1.0f;
                if (underlineY >= 0.0f && underlineY <= static_cast<float>(m_viewportHeight))
                {
                    D2D1_POINT_2F p1 = D2D1::Point2F(0.0f, underlineY);
                    D2D1_POINT_2F p2 = D2D1::Point2F(static_cast<float>(m_viewportWidth), underlineY);
                    m_pRenderTarget->DrawLine(p1, p2, m_pCurrentLineUnderlineBrush, 1.0f);
                }
            }
        }

        // IME入力中の文字列を描画
        float imeCompositionWidth = 0.0f;
        if (pImeInfo &&  !pImeInfo->text.empty() && !cursors.empty()) 
        {
            const TextPosition& cursor = cursors[0];
            POINT pt = TextPositionToScreen(cursor, pDocument);

            // テキストレイアウトを作成
            IDWriteTextLayout* pTextLayout = nullptr;
            HRESULT hr = m_pDWriteFactory->CreateTextLayout(
                pImeInfo->text.c_str(),
                static_cast<UINT32>(pImeInfo->text.length()),
                m_pTextFormat,
                10000.0f,
                m_lineHeight,
                &pTextLayout
            );

            if (SUCCEEDED(hr) && pTextLayout)
            {
                // underline range (measure only)
                DWRITE_TEXT_RANGE textRange = { 0, static_cast<UINT32>(pImeInfo->text.length()) };
                pTextLayout->SetUnderline(TRUE, textRange);


                // 未確定文字列の幅を取得
                DWRITE_TEXT_METRICS metrics;
                pTextLayout->GetMetrics(&metrics);
                imeCompositionWidth = metrics.width;

                pTextLayout->Release();
            }
        }

        // カーソルの描画（IME入力中は未確定文字列の後ろに表示）
        for (size_t i = 0; i < cursors.size(); ++i)
        {
            const auto& cursor = cursors[i];

            if (i == 0 && imeCompositionWidth > 0.0f)
            {
                // 最初のカーソルはIME文字列の幅分だけ右にずらす
                POINT pt = TextPositionToScreen(cursor, pDocument);
                pt.x += static_cast<LONG>(imeCompositionWidth);

                // カーソルを直接描画
                D2D1_RECT_F cursorRect = D2D1::RectF(
                    static_cast<float>(pt.x),
                    static_cast<float>(pt.y),
                    static_cast<float>(pt.x + 2),
                    static_cast<float>(pt.y + m_lineHeight)
                );
                m_pRenderTarget->FillRectangle(cursorRect, m_pCursorBrush);
            }
            else
            {
                RenderCursor(cursor, pDocument);
            }
        }
    }

    HRESULT hr = m_pRenderTarget->EndDraw();

    if (hr == D2DERR_RECREATE_TARGET)
    {
        DiscardDeviceResources();
    }
}

void CTextRenderer::RenderLine(size_t lineIndex, const std::wstring& text, float y)
{
    // Not used in wrap mode; kept for compatibility
    if (text.empty()) return;
    D2D1_RECT_F rect = D2D1::RectF(5.0f - m_scrollOffsetX, y, static_cast<float>(m_viewportWidth), y + m_lineHeight);
    m_pRenderTarget->DrawText(text.c_str(), static_cast<UINT32>(text.length()), m_pTextFormat, rect, m_pTextBrush);
}

void CTextRenderer::RenderCursor(const TextPosition& pos, CTextDocument* pDocument)
{
    // Use TextLayout hit-testing to place caret with wrapping
    POINT screenPos = TextPositionToScreen(pos, pDocument);
    float caretHeight = m_lineHeight;
    // Estimate caret height via layout metrics at position
    const std::wstring& line = pDocument->GetLine(pos.line);
    constexpr float kLeftPadding = 5.0f;
    float availableWidth = static_cast<float>(m_viewportWidth) - kLeftPadding + static_cast<float>(m_scrollOffsetX);
    if (availableWidth <= 0.0f) availableWidth = 1.0f;
    IDWriteTextLayout* pLayout = CreateTextLayoutForLine(line, availableWidth);
    if (pLayout)
    {
        DWRITE_HIT_TEST_METRICS h{};
        FLOAT x=0, y=0;
        if (SUCCEEDED(pLayout->HitTestTextPosition(static_cast<UINT32>(std::min(pos.column, line.length())), FALSE, &x, &y, &h)))
        {
            caretHeight = h.height;
        }
        pLayout->Release();
    }

    D2D1_RECT_F rect = D2D1::RectF(
        static_cast<float>(screenPos.x),
        static_cast<float>(screenPos.y),
        static_cast<float>(screenPos.x + 2),
        static_cast<float>(screenPos.y) + caretHeight
    );
    m_pRenderTarget->FillRectangle(rect, m_pCursorBrush);
}

void CTextRenderer::RenderSelection(const TextPosition& start, const TextPosition& end, CTextDocument* pDocument)
{
    if (!pDocument || !m_pDWriteFactory || !m_pTextFormat)
    {
        return;
    }

    TextPosition actualStart = start < end ? start : end;
    TextPosition actualEnd = start < end ? end : start;

    constexpr float kLeftPadding = 5.0f;
    float availableWidth = static_cast<float>(m_viewportWidth) - kLeftPadding + static_cast<float>(m_scrollOffsetX);
    if (availableWidth <= 0.0f) availableWidth = 1.0f;

    // 可視位置Yの起点（先頭から各行の高さを積算）
    float currentY = -static_cast<float>(m_scrollOffsetY);

    size_t lineCount = pDocument->GetLineCount();
    size_t lastLine = std::min(actualEnd.line, lineCount ? lineCount - 1 : size_t(0));

    for (size_t lineIndex = 0; lineIndex < lineCount; ++lineIndex)
    {
        const std::wstring& lineText = pDocument->GetLine(lineIndex);
        IDWriteTextLayout* pLayout = CreateTextLayoutForLine(lineText, availableWidth);
        if (!pLayout)
        {
            continue;
        }

        DWRITE_TEXT_METRICS metrics{};
        if (SUCCEEDED(pLayout->GetMetrics(&metrics)))
        {
            if (lineIndex >= actualStart.line && lineIndex <= lastLine)
            {
                UINT32 startPos = 0;
                UINT32 length = static_cast<UINT32>(lineText.length());
                if (lineIndex == actualStart.line)
                {
                    startPos = static_cast<UINT32>(std::min(actualStart.column, lineText.length()));
                }
                if (lineIndex == actualEnd.line)
                {
                    UINT32 endPos = static_cast<UINT32>(std::min(actualEnd.column, lineText.length()));
                    if (endPos >= startPos)
                    {
                        length = endPos - startPos;
                    }
                    else
                    {
                        length = 0;
                    }
                }
                else
                {
                    length = static_cast<UINT32>(lineText.length()) - startPos;
                }

                if (length > 0)
                {
                    // HitTestTextRange は複数フラグメントを返すのでバッファを2回確保して取得
                    UINT32 actualCount = 0;
                    std::vector<DWRITE_HIT_TEST_METRICS> frags(16);
                    HRESULT hr = pLayout->HitTestTextRange(startPos, length, 0.0f, 0.0f, frags.data(), static_cast<UINT32>(frags.size()), &actualCount);
                    if (hr == E_NOT_SUFFICIENT_BUFFER)
                    {
                        frags.resize(actualCount);
                        hr = pLayout->HitTestTextRange(startPos, length, 0.0f, 0.0f, frags.data(), static_cast<UINT32>(frags.size()), &actualCount);
                    }
                    if (SUCCEEDED(hr))
                    {
                        float originX = kLeftPadding - static_cast<float>(m_scrollOffsetX);
                        float originY = currentY;
                        for (UINT32 i = 0; i < actualCount; ++i)
                        {
                            const auto& f = frags[i];
                            D2D1_RECT_F rect = D2D1::RectF(
                                originX + f.left,
                                originY + f.top,
                                originX + f.left + f.width,
                                originY + f.top + f.height
                            );
                            // 画面外はスキップ
                            if (rect.bottom >= 0.0f && rect.top <= static_cast<float>(m_viewportHeight))
                            {
                                m_pRenderTarget->FillRectangle(rect, m_pSelectionBrush);
                            }
                        }
                    }
                }
            }

            currentY += metrics.height;
        }

        pLayout->Release();

        if (lineIndex >= lastLine && currentY > m_viewportHeight)
        {
            break;
        }
    }
}

void CTextRenderer::Scroll(int delta)
{
    m_scrollOffsetY -= delta / 2;
    if (m_scrollOffsetY < 0)
    {
        m_scrollOffsetY = 0;
    }
}

SIZE CTextRenderer::CalculateContentSize(CTextDocument* pDocument)
{
    SIZE contentSize = { 0,0 };
    if (!pDocument || !m_pDWriteFactory || !m_pTextFormat)
    {
        return contentSize;
    }

    constexpr float kLeftPadding = 5.0f;
    float availableWidth = static_cast<float>(m_viewportWidth) - kLeftPadding + static_cast<float>(m_scrollOffsetX);
    if (availableWidth <= 0.0f) availableWidth = 1.0f;

    float totalHeight = 0.0f;
    float maxWidth = 0.0f;

    size_t lineCount = pDocument->GetLineCount();
    for (size_t i = 0; i < lineCount; ++i)
    {
        IDWriteTextLayout* pLayout = CreateTextLayoutForLine(pDocument->GetLine(i), availableWidth);
        if (!pLayout)
        {
            continue;
        }
        DWRITE_TEXT_METRICS metrics{};
        if (SUCCEEDED(pLayout->GetMetrics(&metrics)))
        {
            totalHeight += metrics.height;
            maxWidth = std::max(maxWidth, metrics.widthIncludingTrailingWhitespace);
        }
        pLayout->Release();
    }

    // Ensure at least viewport size to avoid zero sized scroll range
    contentSize.cx = static_cast<LONG>(std::max<double>(std::ceil(maxWidth + kLeftPadding), static_cast<double>(m_viewportWidth)));
    contentSize.cy = static_cast<LONG>(std::max<double>(std::ceil(totalHeight), static_cast<double>(m_viewportHeight)));
    return contentSize;
}

void CTextRenderer::SetScrollOffset(int offsetX, int offsetY)
{
    m_scrollOffsetX = std::max(0, offsetX);
    m_scrollOffsetY = std::max(0, offsetY);
}

TextPosition CTextRenderer::ScreenToTextPosition(int x, int y, CTextDocument* pDocument)
{
    TextPosition pos;
    constexpr float kLeftPadding = 5.0f;
    float availableWidth = static_cast<float>(m_viewportWidth) - kLeftPadding + static_cast<float>(m_scrollOffsetX);
    if (availableWidth <= 0.0f) availableWidth = 1.0f;
    float targetY = static_cast<float>(y + m_scrollOffsetY);
    float yAccum = 0.0f;
    for (size_t lineIndex = 0; lineIndex < pDocument->GetLineCount(); ++lineIndex)
    {
        IDWriteTextLayout* pLayout = CreateTextLayoutForLine(pDocument->GetLine(lineIndex), availableWidth);
        if (!pLayout) continue;
        DWRITE_TEXT_METRICS m{};
        if (SUCCEEDED(pLayout->GetMetrics(&m)))
        {
            if (targetY < yAccum + m.height)
            {
                BOOL trailing=FALSE, inside=FALSE;
                DWRITE_HIT_TEST_METRICS hit{};
                FLOAT hitX = static_cast<FLOAT>(x + m_scrollOffsetX - kLeftPadding);
                FLOAT hitY = static_cast<FLOAT>(targetY - yAccum);
                pLayout->HitTestPoint(hitX, hitY, &trailing, &inside, &hit);
                pos.line = lineIndex;
                size_t lineLen = pDocument->GetLine(lineIndex).length();
                size_t col = static_cast<size_t>(hit.textPosition) + (trailing ? 1 : 0);
                pos.column = (col > lineLen) ? lineLen : col;
                pLayout->Release();
                return pos;
            }
            yAccum += m.height;
        }
        pLayout->Release();
    }
    if (pDocument->GetLineCount() > 0)
    {
        pos.line = pDocument->GetLineCount() - 1;
        pos.column = pDocument->GetLine(pos.line).length();
    }
    else
    {
        pos.line = 0; pos.column = 0;
    }
    return pos;
}

POINT CTextRenderer::TextPositionToScreen(const TextPosition& pos, CTextDocument* pDocument)
{
    POINT point{0,0};
    constexpr float kLeftPadding = 5.0f;
    float availableWidth = static_cast<float>(m_viewportWidth) - kLeftPadding + static_cast<float>(m_scrollOffsetX);
    if (availableWidth <= 0.0f) availableWidth = 1.0f;
    float yOffset = 0.0f;
    size_t totalLines = pDocument->GetLineCount();
    size_t targetLine = std::min(pos.line, totalLines ? totalLines - 1 : size_t(0));
    for (size_t i = 0; i < targetLine; ++i)
    {
        IDWriteTextLayout* pL = CreateTextLayoutForLine(pDocument->GetLine(i), availableWidth);
        if (pL)
        {
            DWRITE_TEXT_METRICS m{};
            if (SUCCEEDED(pL->GetMetrics(&m))) yOffset += m.height;
            pL->Release();
        }
    }
    const std::wstring& line = pDocument->GetLine(targetLine);
    IDWriteTextLayout* pLayout = CreateTextLayoutForLine(line, availableWidth);
    if (pLayout)
    {
        DWRITE_HIT_TEST_METRICS hit{};
        FLOAT x=0, y=0;
        UINT32 idx = static_cast<UINT32>(std::min(pos.column, line.length()));
        if (SUCCEEDED(pLayout->HitTestTextPosition(idx, FALSE, &x, &y, &hit)))
        {
            point.x = static_cast<LONG>(kLeftPadding + x - m_scrollOffsetX);
            point.y = static_cast<LONG>(yOffset + y - m_scrollOffsetY);
        }
        pLayout->Release();
    }
    return point;
}

float CTextRenderer::GetTextWidth(const std::wstring& text)
{
    if (!m_pDWriteFactory || !m_pTextFormat || text.empty())
    {
        return 0.0f;
    }

    IDWriteTextLayout* pTextLayout = nullptr;
    HRESULT hr = m_pDWriteFactory->CreateTextLayout(
        text.c_str(),
        static_cast<UINT32>(text.length()),
        m_pTextFormat,
        10000.0f,  // maxWidth
        100.0f,    // maxHeight
        &pTextLayout
    );

    if (FAILED(hr) || !pTextLayout)
    {
        return 0.0f;
    }

    DWRITE_TEXT_METRICS metrics;
    pTextLayout->GetMetrics(&metrics);
    float width = metrics.width;

    pTextLayout->Release();
    return width;
}

IDWriteTextLayout* CTextRenderer::CreateTextLayoutForLine(const std::wstring& text, float maxWidth, float maxHeight)
{
    if (!m_pDWriteFactory || !m_pTextFormat)
    {
        return nullptr;
    }
    IDWriteTextLayout* layout = nullptr;
    HRESULT hr = m_pDWriteFactory->CreateTextLayout(
        text.c_str(),
        static_cast<UINT32>(text.length()),
        m_pTextFormat,
        maxWidth <= 0 ? 1.0f : maxWidth,
        maxHeight,
        &layout
    );
    if (FAILED(hr) || !layout)
    {
        return nullptr;
    }
    layout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
    return layout;
}

