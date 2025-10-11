# vgl

C++によるOpenGLの簡単な実装を可能とするための項目を含めているライブラリである．  

- ライブラリ (git submodule による 3rd party library の導入)  
- ビルド (CMake スクリプトを採用した cross compilation)  
- OpenGLに必要なボイラープレートコードをまとめた，小規模ライブラリ `vgl`．  


## インストール

1. まず，gitによってクローンを行う．ここまではダウンロードでも構わないが，  
  以後のsubmodule更新過程が非常に煩雑になるため，gitの使用を強くお奨めする．  
    `> git clone https://github.com/stnoh/vgl .  `

2. git submodule のアップデートを行う．submodule内部のも全部アップデートするために，以下の命令を実行する．  
    `> git submodule update --init --recursive  `

    [CAUTION] この過程をダウンロードで行うには，個別のレポジトリを全部ダウンロードして解凍して置く必要があり，非常に面倒である！


## ビルド: Windows + Visual Studio 2022

このライブラリは基本的に Visual C++ (VS 2022) on Windows を想定して作られてあるものの，  
将来クロースコンパイル(Ubuntuなど)を支援するために CMakeLists.txt によるコンパイルを想定して開発している．  
Visual Studio/C++ のビルドソリューションを作りたいものは，CMakeの基本的な使い方を参照されたい．  

1. Visual Studio (>=2017) を開く．因みに，Visual Studio は **"Desktop development with C++"** をインストールしておく必要がある．  
2. "Open a local folder"を選択し，このレポジトリをクローンしたフォルダを選択して開く．  
3. ninja build が自動的に CMakeLists.txt を開いてビルドの構成をしてくれる．  

因みに，Ubuntu 22.04 でのビルドも成功が確認されたが，今後もしばらくの開発は Windows + VS2022 で行う予定であり，  
Ubuntu でのビルド可能性は保証されない．  


## 実行可能ファイルの目録

### Basic rendering (rasterization に基づくもの)

- app101: TestAppGL, GLFWによる簡単な Windows 立ち上げテストプログラム．  
- app102: CameraFrustumViewerApp, カメラ視野や姿勢などとレンダリングされた結果との対応がみれるプログラム．  
- app103: SimpleDrawApp, 非常に簡単な三角形を描くプログラム．  
- app104: DepthTestApp, 隠面消去に関わる例題プログラム．  
- app105: SimpleShaderApp, Gouraud shader の例題プログラム．  
- app106: SimpleShadowApp, GLSL による Shadow map の例題プログラム．  
- app107: SimpleTextureApp, 法線マップの例題プログラム．  

### Basic modeling (形状の扱い)  

- app201: [TODO] Quadratic Bezier drawing + control  
- app202: [TODO] Triangle mesh loader  
- app203: [TODO] 簡単なボリュームレンダラ (?)  

### Basic animation  

- app301: [TODO] Forward Kinematics  
- app302: [TODO] Elastic pendulum  
- app303: [TODO] Stable Fluids (2D simple version)  

### Advanced rendering (ray-tracing に基づくもの)

- app401: mrdoob氏の [glsl-sandbox](https://github.com/mrdoob/glsl-sandbox) の機能を真似したローカル簡略版．  


## 注意及び補足

本コードはC++11以後のModern C++やModern GLではなく，C++03のような古い書き方とLegacy GLを多く採用している．C++やGLは，何れも2000年代から2010年代中盤まで多くの変革を経ており，書き方がコロコロ変わっていた時期であった．そこで，インターネット上の記事を探しても，複数のやり方がかなり混在しており，参照に注意を要する．  

このレポジトリも，Modern C++はあまり多く使っておらず，C with Classes(<=C++98)からLegacy C++(~=C++03)の書き方に留まっている．  
- [lambda](https://en.cppreference.com/w/cpp/language/lambda.html)以外のModern C++の書き方はあまり多く使われていない．  
- 使っているライブラリの多くは[RAII](https://en.cppreference.com/w/cpp/language/raii.html)以前のものであるため，導入に制約が多く古いやり方に則ってやっている．  
- [TODO] 生のポインタでなく，[smart pointer](https://en.cppreference.com/w/cpp/memory.html#Smart_pointers)を使用  

