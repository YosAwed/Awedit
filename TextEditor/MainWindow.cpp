// MainWindow.cpp - メインウィンドウ実装
#include "MainWindow.h"
#include "Resource.h"
#include <commdlg.h>
#include <shellapi.h>
#include <windowsx.h>
#include <imm.h>
#include <vector>
#include <cmath>
#include <algorithm>

#pragma comment(lib, "imm32.lib")

const wchar_t CLASS_NAME[] = L"TextEditorWindowClass";

CMainWindow::CMainWindow()
    : m_hwnd(nullptr)
    , m_hInstance(nullptr)
    , m_isModified(false)
    , m_isRectSelectionMode(false)
{
}

CMainWindow::~CMainWindow()
{
}

bool CMainWindow::Create(HINSTANCE hInstance, int nCmdShow)
{
    m_hInstance = hInstance;

    // ウィンドウクラスの登録
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_IBEAM);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
    wcex.lpszClassName = CLASS_NAME;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, L"ウィンドウクラスの登録に失敗しました。", L"エラー", MB_OK | MB_ICONERROR);
        return false;
    }

    // ウィンドウの作成
    m_hwnd = CreateWindowEx(
        WS_EX_ACCEPTFILES,
        CLASS_NAME,
        L"Awedit",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1024, 768,
        NULL,
        NULL,
        hInstance,
        this
    );

    if (!m_hwnd)
    {
        MessageBox(NULL, L"ウィンドウの作成に失敗しました。", L"エラー", MB_OK | MB_ICONERROR);
        return false;
    }

    // ウィンドウの表示
    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);

    return true;
}

LRESULT CALLBACK CMainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMainWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<CMainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hwnd = hwnd;
    }
    else
    {
        pThis = reinterpret_cast<CMainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        return pThis->HandleMessage(uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CMainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        OnCreate();
        return 0;

    case WM_DESTROY:
        OnDestroy();
        return 0;

    case WM_SIZE:
        OnSize(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_PAINT:
        OnPaint();
        return 0;

    case WM_COMMAND:
        OnCommand(wParam);
        return 0;

    case WM_KEYDOWN:
        OnKeyDown(wParam, lParam);
        return 0;

    case WM_CHAR:
        OnChar(wParam);
        return 0;

    case WM_MOUSEMOVE:
        OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
        return 0;

    case WM_LBUTTONDOWN:
        OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
        return 0;

    case WM_LBUTTONUP:
        OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_MOUSEWHEEL:
        OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
        return 0;

    case WM_DROPFILES:
    {
        HDROP hDrop = (HDROP)wParam;
        wchar_t filePath[MAX_PATH];
        if (DragQueryFile(hDrop, 0, filePath, MAX_PATH))
        {
            m_pDocument->LoadFromFile(filePath);
            m_currentFilePath = filePath;
            InvalidateRect(m_hwnd, NULL, TRUE);
        }
        DragFinish(hDrop);
        return 0;
    }

    case WM_IME_SETCONTEXT:
        // すべてのIME UIを非表示にする
        lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
        lParam &= ~ISC_SHOWUIGUIDELINE;
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    case WM_IME_STARTCOMPOSITION:
        // IME入力開始時にIME位置を更新
        UpdateImePosition();
        return 0;

    case WM_IME_COMPOSITION:
        OnImeComposition(lParam);
        if (lParam & GCS_RESULTSTR)
        {
            return 0;
        }
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

case WM_IME_ENDCOMPOSITION:
    // 入力が終了したら未確定文字列をクリア
    m_imeCompositionString.clear();
    m_imeInfo.text.clear();
    m_imeInfo.targetStart = m_imeInfo.targetLength = 0;
    InvalidateRect(m_hwnd, NULL, FALSE);
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
}

void CMainWindow::OnCreate()
{
    // コンポーネントの初期化
    m_pDocument = std::make_unique<CTextDocument>();
    m_pRenderer = std::make_unique<CTextRenderer>();
    m_pEditController = std::make_unique<CEditController>();
    m_pSearchEngine = std::make_unique<CSearchEngine>();
    m_pUndoManager = std::make_unique<CUndoManager>();
    m_pKeyboardHandler = std::make_unique<CKeyboardHandler>();

    // レンダラーの初期化
    m_pRenderer->Initialize(m_hwnd);

    // Add View menu (Font Size) dynamically
    HMENU hMenu = GetMenu(m_hwnd);
    if (hMenu)
    {
        HMENU hView = CreateMenu();
        if (hView)
        {
            AppendMenu(hView, MF_STRING, ID_VIEW_FONTSIZE_12, L"Font Size 12pt");
            AppendMenu(hView, MF_STRING, ID_VIEW_FONTSIZE_14, L"Font Size 14pt");
            AppendMenu(hView, MF_STRING, ID_VIEW_FONTSIZE_16, L"Font Size 16pt");
            AppendMenu(hView, MF_STRING, ID_VIEW_FONTSIZE_18, L"Font Size 18pt");
            AppendMenu(hView, MF_STRING, ID_VIEW_FONTSIZE_20, L"Font Size 20pt");
            AppendMenu(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hView), L"View(&V)");
            float fs = m_pRenderer->GetFontSize();
            UINT id = ID_VIEW_FONTSIZE_14;
            if (fs <= 12.0f) id = ID_VIEW_FONTSIZE_12;
            else if (fs <= 14.0f) id = ID_VIEW_FONTSIZE_14;
            else if (fs <= 16.0f) id = ID_VIEW_FONTSIZE_16;
            else if (fs <= 18.0f) id = ID_VIEW_FONTSIZE_18;
            else id = ID_VIEW_FONTSIZE_20;
            CheckMenuRadioItem(hMenu, ID_VIEW_FONTSIZE_12, ID_VIEW_FONTSIZE_20, id, MF_BYCOMMAND);
            DrawMenuBar(m_hwnd);
        }
    }

    // 初期タイトル更新
    UpdateWindowTitle();
}

void CMainWindow::OnDestroy()
{
    PostQuitMessage(0);
}

void CMainWindow::OnSize(int width, int height)
{
    if (m_pRenderer)
    {
        m_pRenderer->Resize(width, height);
    }
}

void CMainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);

    if (m_pRenderer && m_pDocument)
    {
        const std::wstring* pImeComp = m_imeCompositionString.empty() ? nullptr : &m_imeCompositionString;
        const CompositionInfo* pIme = m_imeInfo.text.empty() ? nullptr : &m_imeInfo;
        m_pRenderer->Render(m_pDocument.get(), m_pEditController.get(), pIme);

    }
    EndPaint(m_hwnd, &ps);
}

void CMainWindow::OnCommand(WPARAM wParam)
{
    switch (LOWORD(wParam))
    {
    case ID_FILE_NEW:
        OnFileNew();
        break;
    case ID_FILE_OPEN:
        OnFileOpen();
        break;
    case ID_FILE_SAVE:
        OnFileSave();
        break;
    case ID_FILE_SAVEAS:
        OnFileSaveAs();
        break;
    case ID_FILE_EXIT:
        PostMessage(m_hwnd, WM_CLOSE, 0, 0);
        break;
    case ID_EDIT_UNDO:
        OnEditUndo();
        break;
    case ID_EDIT_REDO:
        OnEditRedo();
        break;
    case ID_EDIT_CUT:
        OnEditCut();
        break;
    case ID_EDIT_COPY:
        OnEditCopy();
        break;
    case ID_EDIT_PASTE:
        OnEditPaste();
        break;
    case ID_EDIT_SELECTALL:
        OnEditSelectAll();
        break;
    case ID_SEARCH_FIND:
        OnSearchFind();
        break;
    case ID_SEARCH_REPLACE:
        OnSearchReplace();
        break;
    case ID_HELP_CONTENTS:
        OnHelpContents();
        break;
    case ID_HELP_ABOUT:
        OnHelpAbout();
        break;
    case ID_VIEW_FONTSIZE_12:
    case ID_VIEW_FONTSIZE_14:
    case ID_VIEW_FONTSIZE_16:
    case ID_VIEW_FONTSIZE_18:
    case ID_VIEW_FONTSIZE_20:
    {
        float newSize = 14.0f;
        switch (LOWORD(wParam))
        {
        case ID_VIEW_FONTSIZE_12: newSize = 12.0f; break;
        case ID_VIEW_FONTSIZE_14: newSize = 14.0f; break;
        case ID_VIEW_FONTSIZE_16: newSize = 16.0f; break;
        case ID_VIEW_FONTSIZE_18: newSize = 18.0f; break;
        case ID_VIEW_FONTSIZE_20: newSize = 20.0f; break;
        }
        if (m_pRenderer)
        {
            m_pRenderer->SetFontSize(newSize);
        }
        HMENU hMenu = GetMenu(m_hwnd);
        if (hMenu)
        {
            CheckMenuRadioItem(hMenu, ID_VIEW_FONTSIZE_12, ID_VIEW_FONTSIZE_20, LOWORD(wParam), MF_BYCOMMAND);
            DrawMenuBar(m_hwnd);
        }
        InvalidateRect(m_hwnd, NULL, TRUE);
        break;
    }
    }
}

void CMainWindow::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    // ショートカットキーの処理
    if (m_pKeyboardHandler)
    {
        m_pKeyboardHandler->HandleKeyDown(wParam, lParam, this);
    }

    // カーソル移動とその他のキー処理
    if (m_pEditController && m_pDocument)
    {
        bool isShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        switch (wParam)
        {
        case VK_LEFT:
            if (isShiftPressed)
            {
                const auto& cursors = m_pEditController->GetCursors();
                TextPosition cur = cursors.empty() ? TextPosition(0, 0) : cursors[0];
                size_t lineLen = m_pDocument->GetLine(cur.line).length();
                size_t newCol = (cur.column > 0) ? (cur.column - 1) : 0;
                m_pEditController->SelectToPosition(TextPosition(cur.line, newCol), m_pDocument.get());
            }
            else
            {
                m_pEditController->MoveCursor(-1, 0, m_pDocument.get());
            }
            InvalidateRect(m_hwnd, NULL, FALSE);
            break;

        case VK_RIGHT:
            if (isShiftPressed)
            {
                const auto& cursors = m_pEditController->GetCursors();
                TextPosition cur = cursors.empty() ? TextPosition(0, 0) : cursors[0];
                size_t lineLen = m_pDocument->GetLine(cur.line).length();
                size_t newCol = std::min(cur.column + 1, lineLen);
                m_pEditController->SelectToPosition(TextPosition(cur.line, newCol), m_pDocument.get());
            }
            else
            {
                m_pEditController->MoveCursor(1, 0, m_pDocument.get());
            }
            InvalidateRect(m_hwnd, NULL, FALSE);
            break;

        case VK_UP:
            if (isShiftPressed)
            {
                const auto& cursors = m_pEditController->GetCursors();
                TextPosition cur = cursors.empty() ? TextPosition(0, 0) : cursors[0];
                size_t newLine = (cur.line > 0) ? (cur.line - 1) : 0;
                size_t lineLen = m_pDocument->GetLine(newLine).length();
                size_t newCol = std::min(cur.column, lineLen);
                m_pEditController->SelectToPosition(TextPosition(newLine, newCol), m_pDocument.get());
            }
            else
            {
                m_pEditController->MoveCursor(0, -1, m_pDocument.get());
            }
            InvalidateRect(m_hwnd, NULL, FALSE);
            break;

        case VK_DOWN:
            if (isShiftPressed)
            {
                const auto& cursors = m_pEditController->GetCursors();
                TextPosition cur = cursors.empty() ? TextPosition(0, 0) : cursors[0];
                size_t maxLine = m_pDocument->GetLineCount() ? (m_pDocument->GetLineCount() - 1) : 0;
                size_t newLine = std::min(cur.line + 1, maxLine);
                size_t lineLen = m_pDocument->GetLine(newLine).length();
                size_t newCol = std::min(cur.column, lineLen);
                m_pEditController->SelectToPosition(TextPosition(newLine, newCol), m_pDocument.get());
            }
            else
            {
                m_pEditController->MoveCursor(0, 1, m_pDocument.get());
            }
            InvalidateRect(m_hwnd, NULL, FALSE);
            break;

        case VK_HOME:
            if (isShiftPressed)
            {
                const auto& cursors = m_pEditController->GetCursors();
                TextPosition cur = cursors.empty() ? TextPosition(0, 0) : cursors[0];
                m_pEditController->SelectToPosition(TextPosition(cur.line, 0), m_pDocument.get());
            }
            else
            {
                m_pEditController->MoveToLineStart();
            }
            InvalidateRect(m_hwnd, NULL, FALSE);
            break;

        case VK_END:
            if (isShiftPressed)
            {
                const auto& cursors = m_pEditController->GetCursors();
                TextPosition cur = cursors.empty() ? TextPosition(0, 0) : cursors[0];
                size_t lineLen = m_pDocument->GetLine(cur.line).length();
                m_pEditController->SelectToPosition(TextPosition(cur.line, lineLen), m_pDocument.get());
            }
            else
            {
                m_pEditController->MoveToLineEnd(m_pDocument.get());
            }
            InvalidateRect(m_hwnd, NULL, FALSE);
            break;

        case VK_BACK:
            m_pEditController->DeleteChar(m_pDocument.get(), false);
            m_isModified = true;
            InvalidateRect(m_hwnd, NULL, FALSE);
            UpdateWindowTitle();
            break;

        case VK_DELETE:
            m_pEditController->DeleteChar(m_pDocument.get(), true);
            m_isModified = true;
            InvalidateRect(m_hwnd, NULL, FALSE);
            UpdateWindowTitle();
            break;

        case VK_RETURN:
            m_pEditController->InsertChar(m_pDocument.get(), L'\n');
            m_isModified = true;
            InvalidateRect(m_hwnd, NULL, FALSE);
            UpdateWindowTitle();
            break;
        }
    }
}

void CMainWindow::OnChar(WPARAM wParam)
{
    if (m_pEditController && m_pDocument)
    {
        wchar_t ch = static_cast<wchar_t>(wParam);

        // 制御文字はOnKeyDownで処理済みなのでスキップ
        // Tabは許可（WM_CHARで処理）
        if (ch < 32 && ch != L'\t')
        {
            return;
        }

        m_pEditController->InsertChar(m_pDocument.get(), ch);
        m_isModified = true;
        InvalidateRect(m_hwnd, NULL, FALSE);
        UpdateWindowTitle();
    }
}

void CMainWindow::OnMouseMove(int x, int y, WPARAM wParam)
{
    if (wParam & MK_LBUTTON && m_pEditController)
    {
        if (m_pRenderer && m_pDocument)
        {
            TextPosition pos = m_pRenderer->ScreenToTextPosition(x, y, m_pDocument.get());
            m_pEditController->UpdateSelectionToPosition(pos);
        }
        else
        {
            m_pEditController->UpdateSelection(x, y);
        }
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

void CMainWindow::OnLButtonDown(int x, int y, WPARAM wParam)
{
    if (m_pEditController)
    {
        bool isAltPressed = (GetKeyState(VK_MENU) & 0x8000) != 0;
        bool isShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        m_isRectSelectionMode = isAltPressed;
        if (m_pRenderer && m_pDocument)
        {
            TextPosition pos = m_pRenderer->ScreenToTextPosition(x, y, m_pDocument.get());
            if (isShiftPressed && !isAltPressed)
            {
                // Shift+クリック: 現在のカーソルからクリック地点まで選択
                m_pEditController->SelectToPosition(pos, m_pDocument.get());
                InvalidateRect(m_hwnd, NULL, FALSE);
                return;
            }
            m_pEditController->BeginSelectionAtPosition(pos, m_isRectSelectionMode);
            InvalidateRect(m_hwnd, NULL, FALSE);
        }
        else
        {
            if (isShiftPressed && !isAltPressed)
            {
                TextPosition approx(y / 20, x / 8);
                m_pEditController->SelectToPosition(approx, m_pDocument.get());
                InvalidateRect(m_hwnd, NULL, FALSE);
                return;
            }
            m_pEditController->BeginSelection(x, y, m_isRectSelectionMode);
            InvalidateRect(m_hwnd, NULL, FALSE);
        }
    }
}

void CMainWindow::OnLButtonUp(int x, int y)
{
    if (m_pEditController)
    {
        m_pEditController->EndSelection(x, y);
    }
}

void CMainWindow::OnMouseWheel(int delta)
{
    if (m_pRenderer)
    {
        m_pRenderer->Scroll(delta);
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

void CMainWindow::OnFileNew()
{
    if (m_pDocument)
    {
        m_pDocument->Clear();
        m_currentFilePath.clear();
        m_isModified = false;
        InvalidateRect(m_hwnd, NULL, TRUE);
        UpdateWindowTitle();
    }
}

void CMainWindow::OnFileOpen()
{
    OPENFILENAME ofn = {};
    wchar_t fileName[MAX_PATH] = L"";

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFilter = L"テキストファイル (*.txt)\0*.txt\0すべてのファイル (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = L"txt";

    if (GetOpenFileName(&ofn))
    {
        if (m_pDocument->LoadFromFile(fileName))
        {
            m_currentFilePath = fileName;
            m_isModified = false;
            InvalidateRect(m_hwnd, NULL, TRUE);
            UpdateWindowTitle();
        }
        else
        {
            MessageBox(m_hwnd, L"ファイルを開けませんでした。", L"エラー", MB_OK | MB_ICONERROR);
        }
    }
}

void CMainWindow::OnFileSave()
{
    if (m_currentFilePath.empty())
    {
        OnFileSaveAs();
    }
    else
    {
        if (m_pDocument->SaveToFile(m_currentFilePath.c_str()))
        {
            m_isModified = false;
            UpdateWindowTitle();
        }
        else
        {
            MessageBox(m_hwnd, L"ファイルを保存できませんでした。", L"エラー", MB_OK | MB_ICONERROR);
        }
    }
}

void CMainWindow::OnFileSaveAs()
{
    OPENFILENAME ofn = {};
    wchar_t fileName[MAX_PATH] = L"";

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFilter = L"テキストファイル (*.txt)\0*.txt\0すべてのファイル (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"txt";

    if (GetSaveFileName(&ofn))
    {
        if (m_pDocument->SaveToFile(fileName))
        {
            m_currentFilePath = fileName;
            m_isModified = false;
            UpdateWindowTitle();
        }
        else
        {
            MessageBox(m_hwnd, L"ファイルを保存できませんでした。", L"エラー", MB_OK | MB_ICONERROR);
        }
    }
}

void CMainWindow::OnEditUndo()
{
    if (m_pUndoManager && m_pUndoManager->CanUndo())
    {
        m_pUndoManager->Undo(m_pDocument.get());
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

void CMainWindow::OnEditRedo()
{
    if (m_pUndoManager && m_pUndoManager->CanRedo())
    {
        m_pUndoManager->Redo(m_pDocument.get());
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

void CMainWindow::OnEditCut()
{
    OnEditCopy();
    if (m_pEditController && m_pDocument)
    {
        m_pEditController->DeleteSelection(m_pDocument.get());
        m_isModified = true;
        InvalidateRect(m_hwnd, NULL, TRUE);
        UpdateWindowTitle();
    }
}

void CMainWindow::OnEditCopy()
{
    if (m_pEditController && m_pDocument)
    {
        std::wstring text = m_pEditController->GetSelectedText(m_pDocument.get());
        if (!text.empty())
        {
            if (OpenClipboard(m_hwnd))
            {
                EmptyClipboard();
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
                if (hMem)
                {
                    wchar_t* pMem = static_cast<wchar_t*>(GlobalLock(hMem));
                    wcscpy_s(pMem, text.length() + 1, text.c_str());
                    GlobalUnlock(hMem);
                    SetClipboardData(CF_UNICODETEXT, hMem);
                }
                CloseClipboard();
            }
        }
    }
}

void CMainWindow::OnEditPaste()
{
    if (OpenClipboard(m_hwnd))
    {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData)
        {
            wchar_t* pText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pText && m_pEditController && m_pDocument)
            {
                m_pEditController->InsertText(m_pDocument.get(), pText);
                m_isModified = true;
                InvalidateRect(m_hwnd, NULL, TRUE);
                UpdateWindowTitle();
            }
            GlobalUnlock(hData);
        }
        CloseClipboard();
    }
}

void CMainWindow::OnEditSelectAll()
{
    if (m_pEditController && m_pDocument)
    {
        m_pEditController->SelectAll(m_pDocument.get());
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

void CMainWindow::OnSearchFind()
{
    MessageBox(m_hwnd, L"検索機能は実装中です。", L"情報", MB_OK | MB_ICONINFORMATION);
}

void CMainWindow::OnSearchReplace()
{
    MessageBox(m_hwnd, L"置換機能は実装中です。", L"情報", MB_OK | MB_ICONINFORMATION);
}

void CMainWindow::OnHelpContents()
{
    MessageBox(m_hwnd, 
        L"Awedit ヘルプ\n\n"
        L"ショートカットキー:\n"
        L"Ctrl+N: 新規作成\n"
        L"Ctrl+O: 開く\n"
        L"Ctrl+S: 保存\n"
        L"Ctrl+F: 検索\n"
        L"Ctrl+H: 置換\n"
        L"Ctrl+Z: 元に戻す\n"
        L"Ctrl+Y: やり直し\n"
        L"Ctrl+A: すべて選択\n"
        L"Alt+Shift+矢印: マルチカーソル追加\n"
        L"Alt+ドラッグ: 矩形選択\n"
        L"F1: ヘルプ",
        L"ヘルプ", MB_OK | MB_ICONINFORMATION);
}

void CMainWindow::OnHelpAbout()
{
    MessageBox(m_hwnd,
        L"シンプルテキストエディタ Awedit\n"
        L"Version 1.0",
        L"バージョン情報", MB_OK | MB_ICONINFORMATION);
}

void CMainWindow::OnImeComposition(LPARAM lParam)
{
    HIMC hIMC = ImmGetContext(m_hwnd);
    if (!hIMC)
    {
        return;
    }

    // 入力中の文字列を取得
    if (lParam & GCS_COMPSTR)
    {
        LONG len = ImmGetCompositionStringW(hIMC, GCS_COMPSTR, NULL, 0);
        if (len > 0)
        {
            m_imeCompositionString.resize(len / sizeof(wchar_t));
            ImmGetCompositionStringW(hIMC, GCS_COMPSTR, &m_imeCompositionString[0], len);
            // CompositionInfo を更新
            m_imeInfo.text = m_imeCompositionString;
            m_imeInfo.targetStart = 0;
            m_imeInfo.targetLength = static_cast<UINT32>(m_imeInfo.text.length());
            // 属性からターゲット範囲を特定
            LONG alen = ImmGetCompositionStringW(hIMC, GCS_COMPATTR, NULL, 0);
            if (alen > 0)
            {
                std::vector<BYTE> attrs(alen);
                ImmGetCompositionStringW(hIMC, GCS_COMPATTR, attrs.data(), alen);
                UINT32 start = UINT32_MAX, end = 0;
                for (UINT32 i = 0; i < attrs.size(); ++i)
                {
                    BYTE a = attrs[i];
                    if (a == ATTR_TARGET_CONVERTED || a == ATTR_TARGET_NOTCONVERTED)
                    {
                        if (start == UINT32_MAX) start = i;
                        end = i + 1;
                    }
                }
                if (start != UINT32_MAX && end > start)
                {
                    m_imeInfo.targetStart = start;
                    m_imeInfo.targetLength = end - start;
                }
            }
        }
        else
        {
            m_imeCompositionString.clear();
            m_imeInfo.text.clear();
            m_imeInfo.targetStart = m_imeInfo.targetLength = 0;
        }

        UpdateImePosition();
        InvalidateRect(m_hwnd, NULL, FALSE);
    }

    if (lParam & GCS_RESULTSTR)
    {
        // 未確定文字列をクリア
        m_imeCompositionString.clear();

        // 確定文字列を取得
        LONG len = ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, NULL, 0);
        if (len > 0)
        {
            std::wstring resultStr(len / sizeof(wchar_t), L'\0');
            ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, &resultStr[0], len);

            // 確定文字列を挿入
            if (m_pEditController && m_pDocument)
            {
                m_pEditController->InsertText(m_pDocument.get(), resultStr);
                m_isModified = true;
                InvalidateRect(m_hwnd, NULL, FALSE);
                UpdateWindowTitle();
            }
        }
        // 未確定情報クリア
        m_imeInfo.text.clear();
        m_imeInfo.targetStart = m_imeInfo.targetLength = 0;
    }

    ImmReleaseContext(m_hwnd, hIMC);
}

void CMainWindow::UpdateImePosition()
{
    if (!m_pEditController || !m_pRenderer)
    {
        return;
    }

    HIMC hIMC = ImmGetContext(m_hwnd);
    if (!hIMC)
    {
        return;
    }

    // カーソル位置を取得
    const auto& cursors = m_pEditController->GetCursors();
    if (!cursors.empty())
    {
        const TextPosition& cursor = cursors[0];
        POINT pt = m_pRenderer->TextPositionToScreen(cursor, m_pDocument.get());

        // クライアント座標に変換
        RECT clientRect;
        GetClientRect(m_hwnd, &clientRect);

        // IME変換ウィンドウの位置を強制設定
        COMPOSITIONFORM cf;
        cf.dwStyle = CFS_FORCE_POSITION;
        cf.ptCurrentPos.x = pt.x;
        cf.ptCurrentPos.y = pt.y;
        ImmSetCompositionWindow(hIMC, &cf);

        // IME候補ウィンドウの位置を設定
        CANDIDATEFORM cdf;
        cdf.dwIndex = 0;
        cdf.dwStyle = CFS_CANDIDATEPOS;
        cdf.ptCurrentPos.x = pt.x;
        cdf.ptCurrentPos.y = pt.y + 20;  // カーソルの下に配置
        ImmSetCandidateWindow(hIMC, &cdf);

        // IMEフォントを設定
        LOGFONTW lf = {};
        lf.lfHeight = -static_cast<LONG>(m_pRenderer ? (int)std::round(m_pRenderer->GetFontSize()) : 14);
        lf.lfCharSet = DEFAULT_CHARSET;
        wcscpy_s(lf.lfFaceName, L"Consolas");
        ImmSetCompositionFontW(hIMC, &lf);
    }

    ImmReleaseContext(m_hwnd, hIMC);
}

void CMainWindow::UpdateWindowTitle()
{
    std::wstring name;
    if (m_currentFilePath.empty())
    {
        name = L"無題";
    }
    else
    {
        size_t pos = m_currentFilePath.find_last_of(L"\\/");
        name = (pos == std::wstring::npos) ? m_currentFilePath : m_currentFilePath.substr(pos + 1);
    }

    std::wstring title = L"Awedit - " + name;
    if (m_isModified)
    {
        title += L"*";
    }
    SetWindowText(m_hwnd, title.c_str());
}
