// TextRenderer.h - DirectWriteテキストレンダラー
#pragma once
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include "TextDocument.h"
#include "EditController.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

class CEditController; // 前方宣言


// IME composition info
struct CompositionInfo
{
    std::wstring text;
    UINT32 targetStart = 0;
    UINT32 targetLength = 0;
};

class CTextRenderer
{
public:
    CTextRenderer();
    ~CTextRenderer();

    bool Initialize(HWND hwnd);
    void Resize(int width, int height);
    void Render(CTextDocument* pDocument, CEditController* pEditController, const CompositionInfo* pImeInfo = nullptr);
    void Scroll(int delta);
    void SetFontSize(float size);
    float GetFontSize() const { return m_fontSize; }

    // 座標変換
    TextPosition ScreenToTextPosition(int x, int y, CTextDocument* pDocument);
    POINT TextPositionToScreen(const TextPosition& pos, CTextDocument* pDocument);

    // スクロール制御
    void EnsurePositionVisible(const TextPosition& pos, CTextDocument* pDocument);
    void SetScrollPosition(int lineNumber);
    int GetScrollPositionInLines() const;
    int GetVisibleLineCount() const;

private:
    void CreateDeviceResources();
    void DiscardDeviceResources();
    void RenderLine(size_t lineIndex, const std::wstring& text, float y);
    void RenderCursor(const TextPosition& pos, CTextDocument* pDocument);
    void RenderSelection(const TextPosition& start, const TextPosition& end, CTextDocument* pDocument);
    float GetTextWidth(const std::wstring& text);
    IDWriteTextLayout* CreateTextLayoutForLine(const std::wstring& text, float maxWidth, float maxHeight = 100000.0f);

    HWND m_hwnd;
    ID2D1Factory* m_pD2DFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush* m_pTextBrush;
    ID2D1SolidColorBrush* m_pSelectionBrush;
    ID2D1SolidColorBrush* m_pCursorBrush;
    ID2D1SolidColorBrush* m_pCurrentLineUnderlineBrush;
    // 背景の再描画用（必要になった場合に使用）
    ID2D1SolidColorBrush* m_pBackgroundBrush;
    ID2D1SolidColorBrush* m_pImeTargetBrush;
    
    IDWriteFactory* m_pDWriteFactory;
    IDWriteTextFormat* m_pTextFormat;

    // スクロール状態
    int m_scrollOffsetY;
    int m_scrollOffsetX;

    // フォント設定
    float m_fontSize;
    float m_lineHeight;
    float m_charWidth;

    // ビューポート
    int m_viewportWidth;
    int m_viewportHeight;
};

