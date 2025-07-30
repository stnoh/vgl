# 備忘録

## 使っているライブラリにおける考慮

### [RGFW](https://github.com/ColleagueRiley/RGFW)

Type: Windowing library. GLFWの代わりとして考慮可能．  

- [✔] header-only library: ヘッダのみで処理可能な，最大の利点  
- [✘] 情報が足りない: あまり広く使われておらず，エラー対処が難しそう  
- [✘] 延々とでる警告(warning): 警告は本当はよろしくない．まだ時期尚早な気がする  


### [GLAD](https://github.com/Dav1dde/glad)

Type: OpenGL loading library. GLEWの代わりとして考慮．  

- [✔] Modern GL: on-time, tailored code generation.  
- [✘] コード生成する仕組みがあって，かえって面倒に思える  


### [nanogui](https://github.com/wjakob/nanogui)

Type: AntTweakBarの代わりとして使えるかも知れない．  

- [✔] Modern GUI  
- [△] 情報がちょっと足りない  
