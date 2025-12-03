# 高機能テキストエディタ

Win32APIとC++で作られた高性能テキストエディタです。DirectWriteによる美しいレンダリングと、多彩な編集機能を備えています。

## 主要機能

### パフォーマンス
- **Win32API直接利用**: 軽量で高速な動作
- **C++による最適化**: 効率的なメモリ管理
- **メモリマップドファイル**: 10MB以上の大きなファイルを効率的に処理

### 編集機能
- **マルチカーソル**: 複数箇所を同時編集（Alt+Shift+矢印キーで追加）
- **矩形選択**: Altキーを押しながらドラッグで矩形選択
- **無制限Undo/Redo**: メモリが許す限り編集履歴を保存
- **クリップボード操作**: 切り取り、コピー、貼り付け

### 検索・置換
- **正規表現対応**: 強力なパターンマッチング
- **大文字小文字区別**: オプションで切り替え可能
- **単語単位検索**: 完全一致検索

### レンダリング
- **DirectWrite**: 高品質なテキスト表示
- **カラー絵文字対応**: 絵文字を美しく表示
- **スムーズスクロール**: マウスホイールでスムーズにスクロール

## 必要環境

- **OS**: Windows 10/11
- **開発環境**: Visual Studio 2019以降
- **Windows SDK**: 10.0以降
- **C++標準**: C++17

## ビルド方法

### Visual Studioでビルド

1. `TextEditor.sln` をVisual Studioで開く
2. ビルド構成を選択（Debug または Release）
3. プラットフォームを選択（x86 または x64）
4. メニューから「ビルド」→「ソリューションのビルド」を選択

### コマンドラインでビルド

```cmd
# Visual Studio開発者コマンドプロンプトを開く
msbuild TextEditor.sln /p:Configuration=Release /p:Platform=x64
```

## ショートカットキー

### ファイル操作
| キー | 機能 |
|------|------|
| Ctrl+N | 新規作成 |
| Ctrl+O | ファイルを開く |
| Ctrl+S | 保存 |

### 編集操作
| キー | 機能 |
|------|------|
| Ctrl+Z | 元に戻す |
| Ctrl+Y | やり直し |
| Ctrl+X | 切り取り |
| Ctrl+C | コピー |
| Ctrl+V | 貼り付け |
| Ctrl+A | すべて選択 |

### 検索・置換
| キー | 機能 |
|------|------|
| Ctrl+F | 検索 |
| Ctrl+H | 置換 |
| F3 | 次を検索 |
| Shift+F3 | 前を検索 |

### マルチカーソル
| キー | 機能 |
|------|------|
| Alt+Shift+↑ | 上にカーソルを追加 |
| Alt+Shift+↓ | 下にカーソルを追加 |

### その他
| キー | 機能 |
|------|------|
| F1 | ヘルプ |
| Alt+ドラッグ | 矩形選択 |

## プロジェクト構造

```
TextEditor/
├── TextEditor.sln              # Visual Studioソリューション
├── TextEditor/
│   ├── TextEditor.vcxproj      # プロジェクトファイル
│   ├── Main.cpp                # エントリーポイント
│   ├── MainWindow.h/cpp        # メインウィンドウ
│   ├── TextDocument.h/cpp      # ドキュメント管理
│   ├── TextRenderer.h/cpp      # DirectWriteレンダリング
│   ├── EditController.h/cpp    # 編集コントローラー
│   ├── SearchEngine.h/cpp      # 検索エンジン
│   ├── UndoManager.h/cpp       # Undo/Redo管理
│   ├── KeyboardHandler.h/cpp   # キーボード処理
│   ├── Resource.h              # リソース定義
│   └── Resource.rc             # リソーススクリプト
└── README.md                   # このファイル
```

## アーキテクチャ

### クラス設計

#### CMainWindow
メインウィンドウの管理とメッセージ処理を担当します。

#### CTextDocument
テキストデータの管理を行います。10MB以上のファイルはメモリマップドファイルとして読み込まれ、効率的に処理されます。

#### CTextRenderer
DirectWriteを使用してテキストを美しく描画します。カーソル、選択範囲、カラー絵文字もサポートしています。

#### CEditController
マルチカーソルと矩形選択を含む編集操作を制御します。

#### CSearchEngine
正規表現、大文字小文字区別、単語単位検索をサポートする検索エンジンです。

#### CUndoManager
コマンドパターンを使用して無制限のUndo/Redoを実装しています。メモリ使用量を監視し、必要に応じて古い履歴を削除します。

#### CKeyboardHandler
ショートカットキーの登録と処理を行います。

## 技術詳細

### メモリマップドファイル

10MB以上のファイルは自動的にメモリマップドファイルとして読み込まれます。これにより、巨大なファイルでもメモリを効率的に使用できます。

```cpp
m_hFile = CreateFile(filePath, GENERIC_READ, ...);
m_hMapping = CreateFileMapping(m_hFile, ...);
m_pView = MapViewOfFile(m_hMapping, ...);
```

### DirectWriteレンダリング

DirectWriteを使用することで、高品質なテキストレンダリングとカラー絵文字のサポートを実現しています。

```cpp
IDWriteFactory* pDWriteFactory;
DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, ...);
ID2D1HwndRenderTarget* pRenderTarget;
```

### マルチカーソル

複数のカーソル位置を管理し、各カーソルで同時に編集操作を実行できます。

```cpp
std::vector<TextPosition> m_cursors;
for (auto& cursor : m_cursors) {
    pDocument->InsertChar(cursor, ch);
}
```

### 正規表現検索

C++標準ライブラリの`std::regex`を使用して正規表現検索を実装しています。

```cpp
std::wregex regex(pattern, flags);
std::wsmatch match;
std::regex_search(text, match, regex);
```

## 既知の制限事項

1. **アイコンファイル**: デフォルトのアイコンファイルは含まれていません。必要に応じて`icon.ico`を追加してください。

2. **エンコーディング**: 現在はUTF-8とUTF-16（BOM付き）のみサポートしています。

3. **シンタックスハイライト**: 現在のバージョンではシンタックスハイライトは実装されていません。

4. **検索ダイアログ**: 検索・置換機能はAPIとして実装されていますが、ダイアログUIは未実装です。

## 今後の拡張予定

- [ ] シンタックスハイライト
- [ ] 検索・置換ダイアログ
- [ ] 行番号表示
- [ ] コードフォールディング
- [ ] タブ機能（複数ファイル編集）
- [ ] 設定ダイアログ
- [ ] プラグインシステム

## ライセンス

このプロジェクトはサンプルコードとして提供されています。自由に使用、改変、配布できます。

## 貢献

バグ報告や機能追加の提案は歓迎します。

## 作者

Win32API & C++テキストエディタプロジェクト

## 参考資料

- [Microsoft DirectWrite Documentation](https://docs.microsoft.com/en-us/windows/win32/directwrite/direct-write-portal)
- [Win32 API Documentation](https://docs.microsoft.com/en-us/windows/win32/api/)
- [C++ Standard Library](https://en.cppreference.com/w/)
