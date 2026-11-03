# 備忘録

## 古い Visual Studio Community の入手について  

最新バージョン(VS 2026)は，次のサイトで入手可能である: [https://visualstudio.microsoft.com/vs/community/](https://visualstudio.microsoft.com/vs/community/)  

年々，最新バージョンに合わせてアップデートをしているものの，本レポジトリは最初は Visual Studio 2019 を想定して作られたものであり，最新の Visual Studio のサポートはあまり考慮していなかった．なので，過去のバージョンに戻す可能性も考慮している．その際に問題なのは，恐らく過去バージョンの Visual Studio の入手であろう．  

Visual Studio Community は最新バージョンが正式にリリースされると，既存バージョンへの公式的なリンクは消すようになっている．しかし，何時途切れるか分からないものの，しばらくの間はインストールを防がず，辿っていけばインストール自体は可能になる．以下はそのようなインストーラへのリンクである:  

- Visual Studio 2022 [https://aka.ms/vs/17/release/vs_community.exe](https://aka.ms/vs/17/release/vs_community.exe)  
- Visual Studio 2019 [https://aka.ms/vs/16/release/vs_community.exe](https://aka.ms/vs/16/release/vs_community.exe)  
- Visual Studio 2017 [https://aka.ms/vs/15/release/vs_community.exe](https://aka.ms/vs/15/release/vs_community.exe)  


## 他環境でのビルド

### Ubuntu + GCC

試しとして，Linux (Ubuntu)でのビルドにも成功したので，必要な過程を整理しておく．

0. 開発関連で必要なパッケージをインストールする．
    - gcc  
    - make  
    - build-essential  
1. 必要なものをインストールする．  
    必要なパッケージは以下のようになる:  
    - libgl1-mesa-dev  
    - libglu1-mesa-dev  
    - libxrandr-dev  
    - libxinerama-dev  
    - libxcursor-dev  
    - libxi-dev  
    - zenity  
    この中でzenityはNative GUI Ubuntu環境ならば既インストールされているはずであるが，  
    WSLではCLI用の最小限のパッケージしかインストールされないため，  
    zenityを別途インストールしないと，tinyfiledialog関連の動作がCLI仕様になる．  
    因みに，Ubuntuでのパッケージのインストールコマンドは以下のようになる:  
    ```sh
    > sudo apt install {パッケージ}
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
    あるいは，ジョブの数を指定せず無限ジョブ(-j)にしつつ，のシステム負荷を設定(-l4)にアプローチも可能である．  
    ```sh
    > make -j -l4
    ```  

### 他のLinuxへの対応について

非常に多くの distro があるため，全部を支援するのは難しい．せいぜい一定水準以上のシェアのものに限る．  
更に，Custom build なものまで考えるとどうしようもない．Ubuntu のやり方から類推してほしい．  

### [注意] Mac系

私がMac系のPCを手に入れない限り(現在の想定では一生ない気がしますが)試すことすらできません．  
すみませんが自力でなんとか解決して下さい．Mac遣いならばそれくらいは自力で解決するのが望ましいです．  


## ライブラリ

### 現在使用中の 3rd party libraries

基本的にOpenGL関連したライブラリの集めである:  

- [glm](https://github.com/g-truc/glm)-1.0.1: GL Mathematics．OpenGLで用いられる構造体(``float4``など)の使えるライブラリ．  
- [glew-cmake](https://github.com/Perlmint/glew-cmake)-2.2.0: GLEW の cmake 対応版．  
- [glfw](https://github.com/glfw/glfw)-3.3.10: GLコンテキストの使用可能な基本ウィンドウライブラリ．  
- [anttweakbar](https://github.com/stnoh/AntTweakBar): OpenGL screen上に備えられる簡単なGUIライブラリ．  
  + 本来 [tschw氏のレポジトリ](https://github.com/tschw/AntTweakBar)であったが，Visual Studio 2022 以後も対応できるように小さい変更を加えたものである．  
  + このライブラリだけビルドが非常に遅いため大体も検討している．しかし，trackball の実装を含んでいる GUI はなかなか見つからないため維持している．  

後，GLFWとは無関係なGUIダイアログや，ファイル入出力などC++14には足りない一般機能を含んでいる．  

- [tinyfiledialog](https://sourceforge.net/projects/tinyfiledialogs/): 環境依存しない File Dialog 用のGUIツール．  
  + git submoduleの形で入手が困難なため，直接ファイルを取り入れている(zlib/libpng license)．  
- [stb](https://github.com/nothings/stb): 環境依存しない画像読込み・書き込みライブラリ．  
  + 生のポインタを使う特性上，[近年セキュリティ問題が報告されている](https://github.com/nothings/stb?tab=security-ov-file)ものの，  
  教育用途の単純利用では問題ないと判断されるため使用．  
  + 現在は stb_image.h, stb_image_resize2.h, stb_write.h のみ含んでいる．  


### 候補ライブラリ

検討ライブラリ中，優先順位の高いライブラリだけ取り出したもの．  

#### [ImGui](https://github.com/ocornut/imgui)

Type: AntTweakBarの代わりとして考慮．  

- [✔] GL GUI系のデファクトライブラリ  
- [△] 導入の邪魔になる事項  
  + CMake build をもう少し整理する  
  + 回転の trackball 実装が含まれていない  


#### [Eigen](https://gitlab.com/libeigen/libeigen.gitlab.io) 

Type: 線形代数ライブラリ．  

- [✔] C++で線形代数を扱う最も上等なライブラリ  
- まだ事例として線形代数処理が含まれていないため，導入の必要性もなさそう．  
  + 候補は物理シミュレーション: softbody (未実装のapp303) や stable fluids (app304)  
  + もしくは，導入検討中の ARAP などの形状変形手法や，mesh parametrization 系の手法  


### 追加検討中のライブラリ

必要に応じて追加や変更を検討しているライブラリの目録である．  

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


#### [tinyply](https://github.com/ddiakopoulos/tinyply)

Type: ply mesh importer  

- [✔] バイナリ形式にも対応可能．  
- [△] 何れメッシュデータに直す部分はコード作成が必要．  


### 使用/検討中止となったライブラリ

以後検討しなくてもよさそうなライブラリの目録．  

#### [dirent](https://github.com/tronkko/dirent)

Win32 用 POSIX 互換ファイル/ディレクトリ参照ライブラリ．  
シェーダをファイルで読み込もうとした初期の想定では必要であったが，  
しかし，アプリ事例にシェーダーをリテラルとして含む政策を取ることでパス解決も不要になった．  
以後，必要だとしても C++17 の`filesystem`で解決可能に思われる．  

#### [libgd](https://github.com/libgd/libgd)

現在，stbを部分的に取り入れているため，追加検討しないこととなった．  

- [✔] Security-safe image loading library  
- [✘] 画像ライブラリにしては大き目 (30MB)  

#### [assimp](https://github.com/assimp/assimp)

現在，メッシュ関連は簡単なものを自作コードで取り入れており，追加検討しないこととなった．  

- [✔] [30種ほどのアセット形式](https://github.com/assimp/assimp/blob/master/doc/Fileformats.md)に対応可能なライブラリ．  
- [✘] 重すぎる (レポジトリが 511MB!)．授業プラットフォームとして不適切．  
  + Prebuiltは比較的小さい ([インストーラ](https://kimkulling.itch.io/the-asset-importer-lib)が24MB)が，cross-platformが守れなくなる．  
