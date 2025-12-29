# `ikaria (Ikaria.js)`
A GPL/patent-free browser-side media processing library using ffmpeg that runs on WASM.

## Features
- Unlike ffmpeg.wasm, this library uses OPFS for all operations. It can remux even large files that exceed memory size.
- Currently, it only supports converting video files to DASH, but plans to support more operations in the future.

## Develop
```bash
bazel run @hedron_compile_commands//:refresh_all

bazel test --config=native //:dash_remuxer_test --test_output=all

bazel build --config=wasm //:ikaria_wasm
```

## Legal
The ffmpeg used in this library was built using the `--disable-gpl` flag. The developer of this library recognizes that linking this library does not violate the GPL's derivative clauses.

Furthermore, this library does not link decoders/encoders for codecs that are generally considered to be patent-protected, such as H.264, and does not support such codecs.

However, this does not mean that users can avoid legal liability arising from the use of this library.

The developers, contributors, affiliated organizations, and licensors of this library are not liable for any damages (including legal claims and lawsuits) arising from the use of this library.
