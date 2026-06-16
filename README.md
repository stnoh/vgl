# vgl

C++によるOpenGLの簡単な実装を可能とするための項目を含めているライブラリである．  

- ライブラリ (git submodule による 3rd party library の導入)  
- ビルド (CMake スクリプトを採用した cross compilation)  
- OpenGLに必要なボイラープレートコードをまとめた，小規模ライブラリ `vgl`．  


## インストール

1. まず，Visual Studio 2026 をインストールする．  
  以下の公式ページへのリンクを辿り，ダウンロードすること:  
  [https://visualstudio.microsoft.com/vs/community/](https://visualstudio.microsoft.com/vs/community/)  

2. 次に，git を利用して clone を行う．  
    `> git clone https://github.com/stnoh/vgl .`  

3. git submodule のアップデートを行う．submodule内部のも全部アップデートするために，以下の命令を実行する．  
    `> git submodule update --init --recursive`  
    **[CAUTION]** この過程をダウンロードで行うには，個別のレポジトリを全部ダウンロードして解凍して置く必要があり，非常に面倒である！


## ビルド: Windows + Visual Studio 2026

このライブラリは基本的に Visual C++ (VS 2026) on Windows を想定して作られてあるものの，  
クロースコンパイル(Ubuntuなど)を支援するために CMakeLists.txt によるコンパイルを想定して開発している．  
Visual Studio/C++ のビルドソリューションを作りたいものは，CMakeの基本的な使い方を参照されたい．  

1. Visual Studio (>=2017) を開く．因みに，Visual Studio は **"Desktop development with C++"** をインストールしておく必要がある．  
2. "Open a local folder"を選択し，このレポジトリをクローンしたフォルダを選択して開く．  
3. ninja build が自動的に CMakeLists.txt を開いてビルドの構成をしてくれる．  

因みに，Ubuntu 22.04 でのビルド成功も確認したが，今後もしばらくの開発は Windows + VS2026 で行う予定であり，  
Ubuntu や WSL でのビルド可能性は保証されない．  


## 実行可能ファイルの目録

### Basic rendering (rasterization に基づく描画)

- app101: TestAppGL, GLFWによる簡単な Windows 立ち上げテストプログラム．  
- app102: CameraFrustumViewerApp, カメラ視野や姿勢などとレンダリングされた結果との対応がみれるプログラム．  
- app103: SimpleDrawApp, 非常に簡単な三角形を描くプログラム．  
- app104: DepthTestApp, 隠面消去に関わる例題プログラム．  
- app105: SimpleShaderApp, Gouraud shader の例題プログラム．  
- app106: SimpleShadowApp, GLSL による Shadow map の例題プログラム．  
- app107: SimpleTextureApp, 法線マップの例題プログラム．  

### Basic modeling (形状の扱い)  

- app201: CurveDrawingApp, ちょっとした修正で曲線描画が可能なプログラム．  
- app202: TriMeshViewerApp, ちょっとした修正で細分割の適用可能なプログラム．  
- app203: **[TODO]** ~~簡単なボリュームレンダラのプログラム~~  

### Basic animation (アニメーションの原理)  

- app301: KinematicViewerApp, ちょっとした修正で逆機構学の適用可能なプログラム．  
- app302: Simple rigidbody simulation, 簡単な衝突シミュレーション．  
- app303: **[TODO]** ~~簡単な布シミュレーション~~  
- app304: Stable Fluids (2D simple version), プログラム．  

### Advanced rendering (ray-tracing に基づく描画)

- app401: mrdoob氏の [glsl-sandbox](https://github.com/mrdoob/glsl-sandbox) の機能を真似したローカル簡略版．  


## 注意及び補足

### Visual Studio 2026 

コンパイルできないことを確認している．もちろん支援も考慮しているが，何時になるか未定である．  

### C++コードの書き方

本コードはC++11以後のModern C++やModern GLではなく，C++03のような古い書き方とLegacy GLを多く採用している．  
C++やGLの規格は，何れも2000年代から2010年代中盤まで多くの変革を経ており，書き方がコロコロ変わっていた時期であった．  
そこで，インターネット上の記事を探しても複数のやり方が混在しているため，参照に注意が必要である．  

このレポジトリも，Modern C++はあまり多く使っておらず，C with Classes(<=C++98)からLegacy C++(~=C++03)の書き方に留まっている．  

概ねの方針は以下のようになる:  

- コンテナは可能な限り[STL](https://en.cppreference.com/w/cpp/standard_library.html)を利用し，ポインタによる配列はなるべく使わない．  
  `std::vector<T>`, `std::set<T>`, `std::map<T>` など，C++で実装された標準的なコンテナを多数利用している．  
  - しかし，GL系のライブラリは規格が古くSTLや[RAII](https://en.cppreference.com/w/cpp/language/raii.html)の適用が困難なもので，そこは古いやり方に則ってやっている．  
- Modern C++ 的な書き方はあまり多く導入しない．  
  - **例外**: [lambda](https://en.cppreference.com/w/cpp/language/lambda.html)は柔軟性と利便性で，頻繁に使っている．  
