// Main.cpp - テキストエディタのエントリーポイント
#include <windows.h>
#include "MainWindow.h"

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // COM初期化（DirectWrite/Direct2D用）
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
    {
        MessageBox(NULL, L"COM初期化に失敗しました。", L"エラー", MB_OK | MB_ICONERROR);
        return -1;
    }

    // メインウィンドウの作成と表示
    CMainWindow mainWindow;
    if (!mainWindow.Create(hInstance, nCmdShow))
    {
        CoUninitialize();
        return -1;
    }

    // メッセージループ
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // COM終了処理
    CoUninitialize();

    return static_cast<int>(msg.wParam);
}
