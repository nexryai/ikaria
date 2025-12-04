## `ikaria (Ikaria.js)`
WIP: ffmpegとffprobeを利用した、WASMで動作するGPL/特許フリーのメディア処理ライブラリ


### Development


#### .env ファイルを作成
devcontainer使用時は自動的に設定されるため以下の操作は不要です。

Docker in Dockerなど特殊な環境でない限りは、以下の環境変数を設定します。
```
DOCKER_COMPOSE_SRC_VOLUME_DIR=./
```

Docker in Docker環境で作業する場合などは、ホストから見たこのリポジトリのディレクトリを指定してください。
```
DOCKER_COMPOSE_SRC_VOLUME_DIR=/Users/myname/Developer/ikaria
```

#### ビルド
```
# 通常のビルド
docker compose run ikaria task build

# Dockerfile/依存ライブラリを含めて再コンパイル
docker compose run --build ikaria task build
```

#### Debug UI にアクセス
```
cd demo
pnpm install
pnpm dev
```

### Legal
本ライブラリで使用しているffmpegは`--disable-gpl`フラグを使用してビルドしているため、このライブラリのリンクによってGPLの派生条項が適用されることはないと本ライブラリの開発者は認識しています。
またH.264などの一般的に特許が絡むとされているコーデックのデコーダー/エンコーダーもリンクしておらず、本ライブラリはそれらのコーデックに対応していません。
ただしこれらは本ライブラリの使用によって発生した法的な問題の責任を利用者が回避できるということではありません。
このライブラリの利用によって発生したいかなる損害（法的責任の追求、訴訟などを含む）に対しても、本ライブラリの開発者、コントリビューター、所属団体、ライセンサーは一切の責任を負わないものとします。
