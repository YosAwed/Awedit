// KeyboardHandler.cpp - キーボード入力処理実装
#include "KeyboardHandler.h"
#include "MainWindow.h"

CKeyboardHandler::CKeyboardHandler()
    : m_initialized(false)
{
}

CKeyboardHandler::~CKeyboardHandler()
{
}

void CKeyboardHandler::HandleKeyDown(WPARAM wParam, LPARAM lParam, CMainWindow* pMainWindow)
{
    if (!pMainWindow)
    {
        return;
    }

    // 初回のみショートカットを初期化
    if (!m_initialized)
    {
        InitializeDefaultShortcuts(pMainWindow);
        m_initialized = true;
    }

    // 修飾キーの状態を取得
    bool ctrlPressed = IsKeyPressed(VK_CONTROL);
    bool shiftPressed = IsKeyPressed(VK_SHIFT);
    bool altPressed = IsKeyPressed(VK_MENU);

    // キーコンビネーションを作成
    KeyCombination key(static_cast<UINT>(wParam), ctrlPressed, shiftPressed, altPressed);

    // ショートカットを検索して実行
    auto it = m_shortcuts.find(key);
    if (it != m_shortcuts.end())
    {
        it->second();
    }
}

void CKeyboardHandler::RegisterShortcut(const KeyCombination& key, std::function<void()> callback)
{
    m_shortcuts[key] = callback;
}

void CKeyboardHandler::InitializeDefaultShortcuts(CMainWindow* pMainWindow)
{
    // ファイル操作
    RegisterShortcut(KeyCombination('N', true), [pMainWindow]() {
        pMainWindow->OnFileNew();
    });

    RegisterShortcut(KeyCombination('O', true), [pMainWindow]() {
        pMainWindow->OnFileOpen();
    });

    RegisterShortcut(KeyCombination('S', true), [pMainWindow]() {
        pMainWindow->OnFileSave();
    });

    // 編集操作
    RegisterShortcut(KeyCombination('Z', true), [pMainWindow]() {
        pMainWindow->OnEditUndo();
    });

    RegisterShortcut(KeyCombination('Y', true), [pMainWindow]() {
        pMainWindow->OnEditRedo();
    });

    RegisterShortcut(KeyCombination('X', true), [pMainWindow]() {
        pMainWindow->OnEditCut();
    });

    RegisterShortcut(KeyCombination('C', true), [pMainWindow]() {
        pMainWindow->OnEditCopy();
    });

    RegisterShortcut(KeyCombination('V', true), [pMainWindow]() {
        pMainWindow->OnEditPaste();
    });

    RegisterShortcut(KeyCombination('A', true), [pMainWindow]() {
        pMainWindow->OnEditSelectAll();
    });

    // 検索・置換
    RegisterShortcut(KeyCombination('F', true), [pMainWindow]() {
        pMainWindow->OnSearchFind();
    });

    RegisterShortcut(KeyCombination('H', true), [pMainWindow]() {
        pMainWindow->OnSearchReplace();
    });

    RegisterShortcut(KeyCombination(VK_F3), [pMainWindow]() {
        pMainWindow->OnSearchFindNext();
    });

    RegisterShortcut(KeyCombination(VK_F3, false, true), [pMainWindow]() {
        pMainWindow->OnSearchFindPrevious();
    });

    // ヘルプ
    RegisterShortcut(KeyCombination(VK_F1), [pMainWindow]() {
        pMainWindow->OnHelpContents();
    });

    // マルチカーソル（Alt+Shift+矢印）
    RegisterShortcut(KeyCombination(VK_UP, false, true, true), [pMainWindow]() {
        // マルチカーソルを上に追加
        // 実装は EditController を通じて行う
    });

    RegisterShortcut(KeyCombination(VK_DOWN, false, true, true), [pMainWindow]() {
        // マルチカーソルを下に追加
    });
}

bool CKeyboardHandler::IsKeyPressed(int vkCode) const
{
    return (GetKeyState(vkCode) & 0x8000) != 0;
}
