# 備忘録

## 使っているライブラリにおける考慮事項


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


### [tinyply](https://github.com/ddiakopoulos/tinyply)

Type: ply mesh importer  

- [✔] バイナリ形式にも対応可能．  
- [△] 何れメッシュデータに直す部分はコード作成が必要．  


### [assimp](https://github.com/assimp/assimp)

Type: 3D asset (mesh) importer

- [✔] 多様なメッシュ形式に対応可能なインポーター．  
- [△] 重すぎる．授業程度でメッシュ形式を扱っても仕方ないので扱わなくてもよさそうな．  
