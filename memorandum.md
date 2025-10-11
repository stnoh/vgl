# 備忘録

## 他環境でのビルド

### Ubuntu + GCC

試しとして，Linux (Ubuntu)でのビルドにも成功したので，必要な過程を整理しておく．

1. 必要なものをインストールする．  
  必要なパッケージは以下のようであった (要確認)．  
    - libgl1-mesa-dev  
    - libglu1-mesa-dev  
    - libxrandr-dev  
    - libxinerama-dev  
    - libxcursor-dev  
    - libxi-dev  
    ```sh
    > sudo apt-get install {パッケージ}
    ```
2. ビルド用のフォルダを作り，CMakeを作動させる．これでビルド準備が整う．  
    ```sh
    > mkdir build
    > cd build
    > cmake ..
    ```
3. CMakeの生成したMakefileを作動させればビルドできる．  
    ```sh
    > make
    ```  
    因みに，複数ジョブビルド(16 jobs)したい場合は以下のようにすればよい．
    ```sh
    > make -j16
    ```  

### 他のLinux

- Ubuntu 以外の Linux でも，一定水準以上の使用される distro 版で問題が発生したら，なるべく支援したい．特にSteam OS は今後注目したい．  
- Custom build なものは支援はしない予定．  

### [注意] Mac系

私がMac系のPCを手に入れない限り(現在の想定では一生ない気がしますが！)試すことすらできません．  
すみませんが自力でなんとか解決して下さい．  


## ライブラリ

### 現在使用中の 3rd party libraries

基本的にOpenGL関連したライブラリの集めである:  

- [glm](https://github.com/g-truc/glm)-1.0.1: GL Mathematics．OpenGLで用いられる構造体(``float4``など)の使えるライブラリ．  
- [glew-cmake](https://github.com/Perlmint/glew-cmake)-2.2.0: GLEW の cmake 対応版．  
- [glfw](https://github.com/glfw/glfw)-3.3.10: GLコンテキストの使用可能な基本ウィンドウライブラリ．  
- [anttweakbar](https://github.com/stnoh/AntTweakBar): OpenGL screen上に備えられる簡単なGUIライブラリ．  
  本来，[tschw氏のレポジトリ](https://github.com/tschw/AntTweakBar)であったが，もはや Visual Studio 2019 がダウンロードできなくなり，Visual Studio 2022 にも対応できるように小さい変更を加えたものである．  

後，GLFWとは無関係なGUIダイアログや，ファイル入出力などC++14には足りない一般機能を含んでいる．  

- [tinyfiledialog](https://sourceforge.net/projects/tinyfiledialogs/): 環境依存しない File Dialog 用のGUIツール．  
  + このライブラリは現在git submoduleの形で入手が困難なため，直接ファイルを取り入れている(zlib/libpng license)．  
- [dirent](https://github.com/tronkko/dirent): POSIX 互換ファイル/ディレクトリ参照ライブラリ．*NIX 系では基本的に入っているものの，Win32系にはないので追加．  
  + VS2019のサポートしているC++17の`std::filesystem`で代替可能と思われるが，動作する過去のやり方なのでまず残しておく．  

### 追加予定ライブラリ

恐らく必要に応じて追加の必要なライブラリである．

- [stb](https://github.com/nothings/stb): かの有名な``stb_image.h``の元となるらいぶらり．  
  + 2023年当たりにセキュリティ問題が多く報告され，まだ完璧には直っていないようだが，教育用途の単純利用ならばそこまで問題ないと思われる．  
  + 長期的には，何か方策を考えるのが良いかも知れない．  

### 使っているライブラリにおける考慮事項

- dirent: もう必要ないのでは？  
  初期の想定としては，実行ファイルのパスでの特定が必要なC/C++の特性上，
  シェーダを異なるファイルから読めるような対策として入れたものの，  
  授業事例でのシェーダーは全部コード内部にリテラルに含めた状態なのでパス問題も不要になっており，  
  仮に必要としてもC++17の`filesystem`で解決可能に思われる．  


#### [RGFW](https://github.com/ColleagueRiley/RGFW)

Type: Windowing library. GLFWの代わりとして考慮．  

- [✔] header-only library: ヘッダのみで処理可能な，最大の利点  
- [✘] 情報が足りない: あまり広く使われておらず，エラー対処が難しそう  
- [✘] 延々とでる警告(warning): 警告は本当はよろしくない．まだ時期尚早な気がする  


#### [GLAD](https://github.com/Dav1dde/glad)

Type: OpenGL loading library. GLEWの代わりとして考慮．  

- [✔] Modern GL: 時代に沿うようなGLの呼び方．  
- [✘] コード生成する仕組みがあって，かえって面倒に思える．  


#### [nanogui](https://github.com/wjakob/nanogui)

Type: AntTweakBarの代わりとして使えるかも知れない．  

- [✔] Modern GUI  
- [△] 情報がちょっと足りない  


#### [libgd](https://github.com/libgd/libgd)

Type: image loader

- [✔] Security-safe image loading library  
- [△] 画像ライブラリにしてはちょっと大き目 (30MB)  


#### [tinyply](https://github.com/ddiakopoulos/tinyply)

Type: ply mesh importer  

- [✔] バイナリ形式にも対応可能．  
- [△] 何れメッシュデータに直す部分はコード作成が必要．  


#### [assimp](https://github.com/assimp/assimp)

Type: 3D asset (mesh) importer

- [✔] 多様なメッシュ形式に対応可能なインポーター．  
- [△] 重すぎる．授業程度でメッシュ形式を扱っても仕方ないので扱わなくてもよさそうな．  
