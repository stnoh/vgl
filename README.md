# vgl

画像工学研究室(Vision and Graphics Lab)で管理する C++ ライブラリである．  
C++によるOpenGLの基本的な実装を含んでいる．  


## 現在使用ライブラリ

基本的にOpenGL関連したライブラリの集めである:  

- [glm](https://github.com/g-truc/glm)-1.0.1: GL Mathematics．OpenGLで用いられる構造体(``float4``など)の使えるライブラリ．  
- [glew-cmake](https://github.com/Perlmint/glew-cmake)-2.2.0: GLEW の cmake 対応版．  
- [glfw](https://github.com/glfw/glfw)-3.3.10: GLコンテキストの使用可能な基本ウィンドウライブラリ．  
- [anttweakbar](https://github.com/tschw/AntTweakBar): OpenGL screen上に備えられる簡単なGUIライブラリ．  

後，GLFWとは無関係なGUIダイアログや，ファイル入出力などC++14には足りない一般機能を含んでいる．  

- [tinyfiledialog](https://sourceforge.net/projects/tinyfiledialogs/): 環境依存しない File Dialog 用のGUIツール．  
  + このライブラリは現在git submoduleの形で入手が困難なため，直接ファイルを取り入れている(zlib/libpng license)．  
- [dirent](https://github.com/tronkko/dirent): POSIX 互換ファイル/ディレクトリ参照ライブラリ．*NIX 系では基本的に入っているものの，Win32系にはないので追加．  
  + VS2019のサポートしているC++17の`std::filesystem`で代替可能と思われるが，動作する過去のやり方なのでまず残しておく．  


## インストール

1. まず，gitによってクローンを行う．もちろん，ダウンロードでも構わないが，  
  後のsubmodule更新過程が複雑になるため，gitの使用を強くお奨めする．  
    > git clone https://github.com/stnoh/vgl .

2. git submodule のアップデートを行う．submodule内部のも全部アップデートするために，以下の命令を実行する．  
    > git submodule update --recursive  

    [CAUTION] この過程をダウンロードで行うには，個別のレポジトリを全部ダウンロードして解凍して置く必要があり，非常に面倒である！


## コンパイル

このライブラリは基本的に Visual C++ (VS 2019) / Windows を想定して作られてあるものの，  
将来クロースコンパイル(Ubuntuなど)を支援するために CMakeLists.txt によるコンパイルを想定して開発している．  
Visual Studio/C++ のビルドソリューションを作りたいものは，CMakeの基本的な使い方を参照されたい．  

1. Visual Studio (>=2017) を開く．因みに，Visual Studio は **"Desktop development with C++"**をインストールしておく必要がある．  
2. "Open a local folder"を選択し，このレポジトリをクローンしたフォルダを選択して開く．  
3. ninja build が自動的に CMakeLists.txt を開いてビルドの構成をしてくれる．  


## 注意及び補足

本コードはC++11以後のModern C++やModern GLではなく，C++03のような古い書き方とLegacy GLを多く採用している．  
C++やGLは，何れも2000年代から2010年代中盤まで多くの変革を経ており，書き方がコロコロ変わっていた時期であった．  
そこで，インターネット上の記事を探しても，複数のやり方がかなり混在しており，参照に注意を要する．  

このレポジトリも，Modern C++はあまり多く使っておらず，C with Classes(<=C++98)からLegacy C++(~=C++03)の書き方に留まっている．  
- [lambda](https://en.cppreference.com/w/cpp/language/lambda.html)以外のModern C++の書き方はあまり多く使われていない．  
- 使っているライブラリの多くは[RAII](https://en.cppreference.com/w/cpp/language/raii.html)以前のものであるため，導入に制約が多く古いやり方に則ってやっている．  
- [TODO] 生のポインタでなく，[smart pointer](https://en.cppreference.com/w/cpp/memory.html#Smart_pointers)を使用  


## アプリのリスト

- app101: GLFWによる簡単な Windows 立ち上げテストプログラム．  
- app102: AppGLBaseでboilerplate codeの代替テストプログラム．簡単なGL描画も試している．  
- app103: メッシュ読み込み及び奥行き画像による計算のテストプログラム．  
- app105: [glsl sandbox](https://glslsandbox.com/) のローカル簡略版．  
