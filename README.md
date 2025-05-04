## `ikaria (Ikaria.js)`
WIP: ffmpegとffprobeを利用した、WASMで動作するGPL/特許フリーのメディア処理ライブラリ


### Development

#### ビルド
```
# 通常のビルド
rm -rf dist && docker-compose run ffprobe-wasm make && cp -a dist/. www/public/

# Dockerfile変更時
rm -rf dist && docker-compose run --build ffprobe-wasm make && cp -a dist/. www/public/
```

#### Debug UI にアクセス
```
cd www
pnpm install
pnpm serve
```

### Legal
本ライブラリで使用しているffmpegは`--disable-gpl`フラグを使用してビルドしているため、このライブラリのリンクによってGPLの派生条項が適用されることはないと本ライブラリの開発者は認識しています。  
またH.264などの一般的に特許が絡むとされているコーデックのデコーダー/エンコーダーもリンクしておらず、本ライブラリはそれらのコーデックに対応していません。  
ただしこれらは本ライブラリの使用によって発生した法的な問題の責任を利用者が回避できるということではありません。  
このライブラリの利用によって発生したいかなる損害（法的責任の追求、訴訟などを含む）に対しても、本ライブラリの開発者、コントリビューター、所属団体、ライセンサーは一切の責任を負わないものとします。
