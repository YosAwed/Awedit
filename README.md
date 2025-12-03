[![CI](https://github.com/YosAwed/Awedit/actions/workflows/ci.yml/badge.svg)](https://github.com/YosAwed/Awedit/actions/workflows/ci.yml)

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


Win32APIとC++で作られた高性能テキストエディタです。DirectWriteによる美しいレンダリングと、多彩な編集機能を備えています。

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

- **Win32API直接利用**: 軽量で高速な動作
- **C++による最適化**: 効率的なメモリ管理
- **メモリマップドファイル**: 10MB以上の大きなファイルを効率的に処理

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

- **マルチカーソル**: 複数箇所を同時編集（Alt+Shift+矢印キーで追加）
- **矩形選択**: Altキーを押しながらドラッグで矩形選択
- **無制限Undo/Redo**: メモリが許す限り編集履歴を保存
- **クリップボード操作**: 切り取り、コピー、貼り付け

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

- **正規表現対応**: 強力なパターンマッチング
- **大文字小文字区別**: オプションで切り替え可能
- **単語単位検索**: 完全一致検索

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

- **DirectWrite**: 高品質なテキスト表示
- **カラー絵文字対応**: 絵文字を美しく表示
- **スムーズスクロール**: マウスホイールでスムーズにスクロール

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


- **OS**: Windows 10/11
- **開発環境**: Visual Studio 2019以降
- **Windows SDK**: 10.0以降
- **C++標準**: C++17

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


1. `TextEditor.sln` をVisual Studioで開く
2. ビルド構成を選択（Debug または Release）
3. プラットフォームを選択（x86 または x64）
4. メニューから「ビルド」→「ソリューションのビルド」を選択

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


```cmd
# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

msbuild TextEditor.sln /p:Configuration=Release /p:Platform=x64
```

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

| キー | 機能 |
|------|------|
| Ctrl+N | 新規作成 |
| Ctrl+O | ファイルを開く |
| Ctrl+S | 保存 |

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

| キー | 機能 |
|------|------|
| Ctrl+Z | 元に戻す |
| Ctrl+Y | やり直し |
| Ctrl+X | 切り取り |
| Ctrl+C | コピー |
| Ctrl+V | 貼り付け |
| Ctrl+A | すべて選択 |

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

| キー | 機能 |
|------|------|
| Ctrl+F | 検索 |
| Ctrl+H | 置換 |
| F3 | 次を検索 |
| Shift+F3 | 前を検索 |

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

| キー | 機能 |
|------|------|
| Alt+Shift+↑ | 上にカーソルを追加 |
| Alt+Shift+↓ | 下にカーソルを追加 |

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

| キー | 機能 |
|------|------|
| F1 | ヘルプ |
| Alt+ドラッグ | 矩形選択 |

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


```
TextEditor/
├── TextEditor.sln              # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

├── TextEditor/
│   ├── TextEditor.vcxproj      # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

│   ├── Main.cpp                # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

│   ├── MainWindow.h/cpp        # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

│   ├── TextDocument.h/cpp      # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

│   ├── TextRenderer.h/cpp      # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

│   ├── EditController.h/cpp    # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

│   ├── SearchEngine.h/cpp      # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

│   ├── UndoManager.h/cpp       # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

│   ├── KeyboardHandler.h/cpp   # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

│   ├── Resource.h              # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

│   └── Resource.rc             # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

└── README.md                   # Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

```

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

メインウィンドウの管理とメッセージ処理を担当します。

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

テキストデータの管理を行います。10MB以上のファイルはメモリマップドファイルとして読み込まれ、効率的に処理されます。

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

DirectWriteを使用してテキストを美しく描画します。カーソル、選択範囲、カラー絵文字もサポートしています。

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

マルチカーソルと矩形選択を含む編集操作を制御します。

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

正規表現、大文字小文字区別、単語単位検索をサポートする検索エンジンです。

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

コマンドパターンを使用して無制限のUndo/Redoを実装しています。メモリ使用量を監視し、必要に応じて古い履歴を削除します。

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。

ショートカットキーの登録と処理を行います。

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


10MB以上のファイルは自動的にメモリマップドファイルとして読み込まれます。これにより、巨大なファイルでもメモリを効率的に使用できます。

```cpp
m_hFile = CreateFile(filePath, GENERIC_READ, ...);
m_hMapping = CreateFileMapping(m_hFile, ...);
m_pView = MapViewOfFile(m_hMapping, ...);
```

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


DirectWriteを使用することで、高品質なテキストレンダリングとカラー絵文字のサポートを実現しています。

```cpp
IDWriteFactory* pDWriteFactory;
DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, ...);
ID2D1HwndRenderTarget* pRenderTarget;
```

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


複数のカーソル位置を管理し、各カーソルで同時に編集操作を実行できます。

```cpp
std::vector<TextPosition> m_cursors;
for (auto& cursor : m_cursors) {
    pDocument->InsertChar(cursor, ch);
}
```

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


C++標準ライブラリの`std::regex`を使用して正規表現検索を実装しています。

```cpp
std::wregex regex(pattern, flags);
std::wsmatch match;
std::regex_search(text, match, regex);
```

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


1. **アイコンファイル**: デフォルトのアイコンファイルは含まれていません。必要に応じて`icon.ico`を追加してください。

2. **エンコーディング**: 現在はUTF-8とUTF-16（BOM付き）のみサポートしています。

3. **シンタックスハイライト**: 現在のバージョンではシンタックスハイライトは実装されていません。

4. **検索ダイアログ**: 検索・置換機能はAPIとして実装されていますが、ダイアログUIは未実装です。

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


- [ ] シンタックスハイライト
- [ ] 検索・置換ダイアログ
- [ ] 行番号表示
- [ ] コードフォールディング
- [ ] タブ機能（複数ファイル編集）
- [ ] 設定ダイアログ
- [ ] プラグインシステム

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


このプロジェクトはサンプルコードとして提供されています。自由に使用、改変、配布できます。

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


バグ報告や機能追加の提案は歓迎します。

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


Win32API & C++テキストエディタプロジェクト

# Awedit

Windows 向け C++/DirectWrite ベースのテキストエディタ。


- [Microsoft DirectWrite Documentation](https://docs.microsoft.com/en-us/windows/win32/directwrite/direct-write-portal)
- [Win32 API Documentation](https://docs.microsoft.com/en-us/windows/win32/api/)
- [C++ Standard Library](https://en.cppreference.com/w/)


## 実行ファイル

- Debug: TextEditor/x64/Debug/Awedit.exe
- Release: TextEditor/x64/Release/Awedit.exe

