# vgl

画像工学研究室(Vision and Graphics Lab)で管理する C++ ライブラリである．  
C++によるOpenGLの基本的な実装を含んでいる．  


## インストール

1. まず，gitによってクローンを行う．もちろん，ダウンロードでも構わないが，  
  後の過程が複雑になるだけであり，gitの使用をお奨めする．  
    > git clone https://github.com/stnoh/vgl .

2. git submodule のアップデートを行う．submodule内部のも  
    > git submodule update --recursive


## 現在使用ライブラリ

基本的にOpenGL関連したライブラリの集めである:  

- [glm](https://github.com/g-truc/glm)-1.0.1: GL Mathematics．OpenGLで用いられる構造体(``float4``など)の使えるライブラリ  
- [glew-cmake](https://github.com/Perlmint/glew-cmake)-2.2.0: GLEW の cmake 対応版  
- [glfw](https://github.com/glfw/glfw)-3.3.10: GLコンテキストの使用可能な基本ウィンドウライブラリ．  
- [anttweakbar](https://github.com/tschw/AntTweakBar): OpenGL screen上に備えられる簡単なGUIライブラリ．  


## コンパイル

このライブラリは基本的に Visual C++ / Windows を想定して作られてあるものの，  
将来クロースコンパイル(Ubuntuなど)を支援するために CMakeLists.txt によるコンパイルをしている．  
Visual Studio/C++ のビルドソリューションを作りたいものは，CMakeの基本的な使い方を参照されたし．  

1. Visual Studio (>=2017) を開く．因みに，Visual Studio は "Desktop development with C++"をインストールしておく必要がある．  
2. "Open a local folder"を選択し，このレポジトリをクローンしたフォルダを選択して開く．  
3. ninja build によって自動的に CMakeLists.txt を開いてビルド構成をしてくれる．  
