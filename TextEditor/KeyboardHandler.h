// KeyboardHandler.h - キーボード入力処理
#pragma once
#include <windows.h>
#include <map>
#include <functional>

class CMainWindow; // 前方宣言

// キーコンビネーション
struct KeyCombination
{
    UINT vkCode;
    bool ctrl;
    bool shift;
    bool alt;

    KeyCombination(UINT vk, bool c = false, bool s = false, bool a = false)
        : vkCode(vk), ctrl(c), shift(s), alt(a) {}

    bool operator<(const KeyCombination& other) const
    {
        if (vkCode != other.vkCode) return vkCode < other.vkCode;
        if (ctrl != other.ctrl) return ctrl < other.ctrl;
        if (shift != other.shift) return shift < other.shift;
        return alt < other.alt;
    }
};

class CKeyboardHandler
{
public:
    CKeyboardHandler();
    ~CKeyboardHandler();

    void HandleKeyDown(WPARAM wParam, LPARAM lParam, CMainWindow* pMainWindow);
    void RegisterShortcut(const KeyCombination& key, std::function<void()> callback);

private:
    void InitializeDefaultShortcuts(CMainWindow* pMainWindow);
    bool IsKeyPressed(int vkCode) const;

    std::map<KeyCombination, std::function<void()>> m_shortcuts;
    bool m_initialized;
};
