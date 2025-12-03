// MainWindow.h - メインウィンドウクラス
#pragma once
#include <windows.h>
#include <memory>
#include "TextDocument.h"
#include "TextRenderer.h"
#include "EditController.h"
#include "SearchEngine.h"
#include "UndoManager.h"
#include "KeyboardHandler.h"

class CMainWindow
{
    friend class CKeyboardHandler;

public:
    CMainWindow();
    ~CMainWindow();

    bool Create(HINSTANCE hInstance, int nCmdShow);
    HWND GetHwnd() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    // メッセージハンドラ
    void OnCreate();
    void OnDestroy();
    void OnSize(int width, int height);
    void OnPaint();
    void OnCommand(WPARAM wParam);
    void OnKeyDown(WPARAM wParam, LPARAM lParam);
    void OnChar(WPARAM wParam);
    void OnMouseMove(int x, int y, WPARAM wParam);
    void OnLButtonDown(int x, int y, WPARAM wParam);
    void OnLButtonUp(int x, int y);
    void OnMouseWheel(int delta);
    void OnImeComposition(LPARAM lParam);
    void UpdateImePosition();

    // コマンドハンドラ
    void OnFileNew();
    void OnFileOpen();
    void OnFileSave();
    void OnFileSaveAs();
    void OnEditUndo();
    void OnEditRedo();
    void OnEditCut();
    void OnEditCopy();
    void OnEditPaste();
    void OnEditSelectAll();
    void OnSearchFind();
    void OnSearchReplace();
    void OnHelpAbout();
    void OnHelpContents();
    void UpdateFontSizeMenuCheck(UINT id);
    void UpdateWindowTitle();

    HWND m_hwnd;
    HINSTANCE m_hInstance;
    
    // コンポーネント
    std::unique_ptr<CTextDocument> m_pDocument;
    std::unique_ptr<CTextRenderer> m_pRenderer;
    std::unique_ptr<CEditController> m_pEditController;
    std::unique_ptr<CSearchEngine> m_pSearchEngine;
    std::unique_ptr<CUndoManager> m_pUndoManager;
    std::unique_ptr<CKeyboardHandler> m_pKeyboardHandler;

    // 状態
    std::wstring m_currentFilePath;
    bool m_isModified;
    bool m_isRectSelectionMode;
    std::wstring m_imeCompositionString;  // IME入力中の未確定文字列
    CompositionInfo m_imeInfo;            // IME未確定の詳細（ターゲット範囲含む）
};
