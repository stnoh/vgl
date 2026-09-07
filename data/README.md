# vgl/data

## bunny_small.ply 及び bunny_small.obj

- かの["The Stanford Bunny"](https://faculty.cc.gatech.edu/~turk/bunny/bunny.html)の簡略版．  
- 本来の"Stanford bunny"は[Greg Turk氏 (現GATECH教授)](https://faculty.cc.gatech.edu/~turk)のポスドクの頃にスキャンして得られたもの．  
- 1994年には["Mesh Zippering"](https://graphics.stanford.edu/software/zippack/)という手法で得られたものである．しかし，この手法によるメッシュにはやや不都合があり，現在はあまりよく使われていない．  
- 1997年には[Brian Curless氏 (現UWashington教授)](https://homes.cs.washington.edu/~curless/)の博士課程当時の[VRIP](https://graphics.stanford.edu/software/vrip/)という手法で更新．再構築されたメッシュの滑らかさ故，この手法によるモデルが広く使われている．    
- このレポジトリに含まれている bunny は，形状処理ライブラリ[libigl](https://github.com/libigl/libigl)のチュートリアルに含まれていた，bunny.OFFファイルを[MeshLab](https://www.meshlab.net/)で再びplyに再出力したものである．  

## skybox/*.jpg (License: CC BY-NC-4.0)

- [LearnOpenGL](https://learnopengl.com/) の公式レポジトリ [resources/textures/skybox](https://github.com/JoeyDeVries/LearnOpenGL/tree/master/resources/textures/skybox) から入手可能な画像ファイル．  
- [humus.name の Textures](https://www.humus.name/index.php?page=Textures) から入手可能なキューブマップと連動可能にファイル名に修正を行っている．  

## Netherland_beach.jpg (License: CC BY-SA-4.0)

- [wikimedia commons](https://commons.wikimedia.org/)から入手可能なequirectangular画像の内，環境マップとして相応しいものを選定．  
  - [File:Texel netherland beach equirectangular.png
](https://commons.wikimedia.org/wiki/File:Texel_netherland_beach_equirectangular.png)
  - 容量削減のため，1280x640のサイズをダウンロードした上，jpgで保存している．  
