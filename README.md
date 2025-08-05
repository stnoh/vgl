# vgl

画像工学研究室(Vision and Graphics Lab)で管理する C++ ライブラリである．  
C++によるOpenGLの基本的な実装を含んでいる．  


## 現在使用ライブラリ

基本的にOpenGL関連したライブラリの集めである:  

- [glm](https://github.com/g-truc/glm)-1.0.1: GL Mathematics．OpenGLで用いられる構造体(``float4``など)の使えるライブラリ  
- [glew-cmake](https://github.com/Perlmint/glew-cmake)-2.2.0: GLEW の cmake 対応版  
- [glfw](https://github.com/glfw/glfw)-3.3.10: GLコンテキストの使用可能な基本ウィンドウライブラリ．  
- [anttweakbar](https://github.com/tschw/AntTweakBar): OpenGL screen上に備えられる簡単なGUIライブラリ．  
- [tinyfiledialog](https://sourceforge.net/projects/tinyfiledialogs/): 環境依存しない File Dialog 用のGUIツール．  
  + このライブラリは現在 git submoduleの形で入手が困難なため，直接ファイルを取り入れている(zlib/libpng license)．  


## インストール

1. まず，gitによってクローンを行う．もちろん，ダウンロードでも構わないが，  
  後の過程が複雑になるだけであり，gitの使用をお奨めする．  
    > git clone https://github.com/stnoh/vgl .

2. git submodule のアップデートを行う．submodule内部のも全部アップデートするために，以下の命令を実行する  
    > git submodule update --recursive


## コンパイル

このライブラリは基本的に Visual C++ (VS 2019) / Windows を想定して作られてあるものの，  
将来クロースコンパイル(Ubuntuなど)を支援するために CMakeLists.txt によるコンパイルを想定して開発している．  
Visual Studio/C++ のビルドソリューションを作りたいものは，CMakeの基本的な使い方を参照されたい．  

1. Visual Studio (>=2017) を開く．因みに，Visual Studio は **"Desktop development with C++"**をインストールしておく必要がある．  
2. "Open a local folder"を選択し，このレポジトリをクローンしたフォルダを選択して開く．  
3. ninja build が自動的に CMakeLists.txt を開いてビルドの構成をしてくれる．  


