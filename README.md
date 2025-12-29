## `ikaria (Ikaria.js)`
WIP: ffmpegを利用した、WASMで動作するGPL/特許フリーのメディア処理ライブラリ

### Develop
```bash
bazel run @hedron_compile_commands//:refresh_all

bazel test --config=native //:dash_remuxer_test --test_output=all

bazel build --config=wasm //:ikaria_wasm
```

### Legal
本ライブラリで使用しているffmpegは`--disable-gpl`フラグを使用してビルドしているため、このライブラリのリンクによってGPLの派生条項が適用されることはないと本ライブラリの開発者は認識しています。
またH.264などの一般的に特許が絡むとされているコーデックのデコーダー/エンコーダーもリンクしておらず、本ライブラリはそれらのコーデックに対応していません。
ただしこれらは本ライブラリの使用によって発生した法的な問題の責任を利用者が回避できるということではありません。
このライブラリの利用によって発生したいかなる損害（法的責任の追求、訴訟などを含む）に対しても、本ライブラリの開発者、コントリビューター、所属団体、ライセンサーは一切の責任を負わないものとします。
