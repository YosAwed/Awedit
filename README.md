# Awedit (TextEditor)

**概要**
- **軽量エディタ**: C++17/Win32 + DirectWrite ベースのシンプルで高速なテキストエディタ。
- **大容量対応**: メモリマップド I/O を用い、10MB 以上のファイルでもスムーズに操作。
- **快適操作**: 検索/置換、Undo/Redo、スクロール最適化、IME 入力に対応。

**主な機能**
- **レンダリング**: DirectWrite による高品質描画とスクロールパフォーマンス。
- **編集**: カーソル移動/選択、複数行編集、クリップボードとの連携。
- **検索/置換**: `CSearchEngine` による前方/後方検索、範囲選択内検索、置換。
- **履歴**: `CUndoManager` による段階的な Undo/Redo。
- **キーボード**: `CKeyboardHandler` のショートカット/キー入力処理。
- **大きなファイル**: メモリマップド I/O で UI スレッドをブロックしない読み書き設計。

**動作環境**
- **OS**: Windows 10/11 (64bit 推奨)
- **IDE**: Visual Studio 2019 以降（C++ デスクトップ開発、Windows 10+ SDK）
- **言語/標準**: C++17
- **依存**: Direct2D/DirectWrite（Windows SDK 標準ライブラリ）

**プロジェクト構成**
- `Awedit.sln`: ソリューション（VS で開く）
- `TextEditor/`: ソースコード一式
  - `Main.cpp`: エントリポイント
  - `MainWindow.*`: ウィンドウ生成/メッセージループ/メインフレーム
  - `TextDocument.*`: ドキュメントモデル・テキストバッファ
  - `TextRenderer.*`: DirectWrite ベースの描画とレイアウト
  - `EditController.*`: 編集操作・カーソル/選択・貼り付けなど
  - `SearchEngine.*`: 検索/置換ロジック
  - `UndoManager.*`: Undo/Redo スタック管理
  - `KeyboardHandler.*`: キー入力/ショートカット処理
  - `Resource.rc`/`Resource.h`: リソース（アイコン/メニュー等）。`icon_placeholder.txt` 参照
- `x64/` または `Win32/`: ビルド成果物（構成別にサブフォルダが作成）

**ビルド方法（Visual Studio）**
- `Awedit.sln` を Visual Studio で開く
- ツールバーで `Debug`/`Release` と `x64`/`Win32` を選択
- `Ctrl+Shift+B` でビルド

**ビルド方法（CLI / Developer Command Prompt）**
- `x64 Debug`: `msbuild Awedit.sln /p:Configuration=Debug /p:Platform=x64`
- `x64 Release`: `msbuild Awedit.sln /p:Configuration=Release /p:Platform=x64`
- `Win32 Debug`: `msbuild Awedit.sln /p:Configuration=Debug /p:Platform=Win32`
- `Win32 Release`: `msbuild Awedit.sln /p:Configuration=Release /p:Platform=Win32`

**実行**
- `x64/Debug/Awedit.exe` または `x64/Release/Awedit.exe`
- Win32 構成の場合は `Win32/Debug/` などの出力先に生成

**コーディング規約**
- **言語/標準**: C++17、Windows SDK 10+
- **スタイル**: インデント 4 スペース、Allman ブレース、UTF-8 ソース
- **命名**: クラスは `C`+PascalCase（例: `CMainWindow`）、メソッドは PascalCase、メンバーは `m_` 接頭（例: `m_hwnd`）
- **方針**: 可能な限り STL とセーフ関数を使用（レガシー/unsafe API は回避）

**開発のヒント / セキュリティ**
- **UI 非ブロッキング**: 大きなファイル I/O は UI スレッドをブロックしない（メモリマップド I/O を利用）
- **安全な API**: 文字列/ファイル操作は `_s` 系または STL を優先
- **ツールセット**: Windows SDK とプラットフォームツールセットが正しくインストールされていることを確認
- **標準設定**: プロジェクトの C++ 標準は C++17 に設定

**手動テスト（暫定）**
- **ファイル操作**: 大きめ（10MB+）を含むファイルの開閉/保存
- **編集系**: 入力/削除/改行、カーソル移動、矩形/通常選択の挙動
- **Undo/Redo**: 操作履歴の戻す/やり直しの正確性
- **検索/置換**: 前/後方、選択範囲内、全置換の確認
- **IME 入力**: 日本語 IME での変換/確定の描画と確実性
- **描画/スクロール**: DirectWrite の描画破綻がないか、スクロール時のパフォーマンス

**コントリビュート方法**
- **コミット規約**: Conventional Commits（例: `feat: renderer`, `fix: editor`）
- **Pull Request**: 変更内容の説明、関連 Issue、UI 変更時は before/after スクリーンショット、検証手順、対象構成（`Debug/Release` と `x64/Win32`）を記載
- **スコープ**: 変更はフォーカスを保ち、挙動やビルド手順が変わる場合はドキュメント更新

**アイコン/リソース**
- 既定ではプレースホルダ。独自アイコンを使う場合は `TextEditor/icon.ico` を用意し、`Resource.rc` の参照を更新（`icon_placeholder.txt` を参照）

**トラブルシューティング**
- **Windows SDK が見つからない**: VS Installer で最新の Windows 10/11 SDK を追加
- **PlatformToolset 不一致**: `v142` などに合わせるか、インストール済みのツールセットへ変更
- **文字化け**: ソースは UTF-8、コンパイルオプション `/utf-8` を維持
- **リンクエラー(d2d1/dwrite)**: Windows SDK のライブラリパス/コンポーネントが導入済みか確認

**ライセンス**
- ライセンスファイルが存在する場合はプロジェクトルートの `LICENSE` を参照してください。

