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

- [glm](https://github.com/g-truc/glm): GL Mathematics．OpenGLで用いられる構造体(``float4``など)の使えるライブラリ  
- [glew-cmake](https://github.com/Perlmint/glew-cmake): GLEW の cmake 対応版  
- [glfw](https://github.com/glfw/glfw): GLコンテキストの使用可能な基本ウィンドウライブラリ．  
