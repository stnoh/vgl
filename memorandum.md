# 備忘録

## 追加予定ライブラリ

恐らく必要に応じて追加の必要なライブラリである．

- [stb](https://github.com/nothings/stb): かの有名な``stb_image.h``の元となるらいぶらり．  
  + 2023年当たりにセキュリティ問題が多く報告され，まだ完璧には直っていないようだが，教育用途の単純利用ならばそこまで問題ないと思われる．  
  + 長期的には，何か方策を考えるのが良いかも知れない．  


## 使っているライブラリにおける考慮事項

- dirent: もう必要ないのでは？  
  初期の想定としては，実行ファイルのパスでの特定が必要なC/C++の特性上，
  シェーダを異なるファイルから読めるような対策として入れたものの，  
  授業事例でのシェーダーは全部コード内部にリテラルに含めた状態なのでパス問題も不要になっており，  
  仮に必要としてもC++17の`filesystem`で解決可能に思われる．  


### [RGFW](https://github.com/ColleagueRiley/RGFW)

Type: Windowing library. GLFWの代わりとして考慮．  

- [✔] header-only library: ヘッダのみで処理可能な，最大の利点  
- [✘] 情報が足りない: あまり広く使われておらず，エラー対処が難しそう  
- [✘] 延々とでる警告(warning): 警告は本当はよろしくない．まだ時期尚早な気がする  


### [GLAD](https://github.com/Dav1dde/glad)

Type: OpenGL loading library. GLEWの代わりとして考慮．  

- [✔] Modern GL: 時代に沿うようなGLの呼び方．  
- [✘] コード生成する仕組みがあって，かえって面倒に思える．  


### [nanogui](https://github.com/wjakob/nanogui)

Type: AntTweakBarの代わりとして使えるかも知れない．  

- [✔] Modern GUI  
- [△] 情報がちょっと足りない  


## [libgd](https://github.com/libgd/libgd)

Type: image loader

- [✔] Security-safe image loading library  
- [△] 画像ライブラリにしてはちょっと大き目 (30MB)  


### [tinyply](https://github.com/ddiakopoulos/tinyply)

Type: ply mesh importer  

- [✔] バイナリ形式にも対応可能．  
- [△] 何れメッシュデータに直す部分はコード作成が必要．  


### [assimp](https://github.com/assimp/assimp)

Type: 3D asset (mesh) importer

- [✔] 多様なメッシュ形式に対応可能なインポーター．  
- [△] 重すぎる．授業程度でメッシュ形式を扱っても仕方ないので扱わなくてもよさそうな．  



## Linux でのビルド

Linux (Ubuntu) でビルドを試したものの，以下の二つの問題が発見された．  

1. [anttweakbar]のビルドが上手くいかない．個別の[anttweakbar]だけビルドしたら起きない問題なので，どこかオプションが間違っているかも知れない．  
2. GLU.h の問題．パッケージをインストールしたものの header ファイルが探せない問題なので，何か Linux 専用のパス設定が必要かも知れない．  

