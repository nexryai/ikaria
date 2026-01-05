# `ikaria (Ikaria.js)`
A GPL/patent-free browser-side media processing library using ffmpeg that runs on WASM.

## Features
- Unlike ffmpeg.wasm, this library uses OPFS for all operations. It can remux even large files that exceed memory size.
- Currently, it only supports converting video files to DASH, but plans to support more operations in the future.

## Design Philosophy
- This library is not intended as a replacement for ffmpeg.wasm. It does not support patented codecs such as H.264 or features that require GPL-licensed libraries.
- ikaria is designed for browser-side remuxing and "lossless cutting" of large files (which is less expensive than encoding). Unlike ffmpeg.wasm, ikaria relies almost exclusively on external libraries and intentionally does not support codecs that require them. ikaria uses only ffmpeg's native muxer and decoder.
- While the WASM runtime environment has strong memory protection features, ikaria uses clang's sanitizer as an additional protection. This is to prevent arbitrary code execution when a user processes a maliciously crafted file.

## Develop
```bash
bazel run @hedron_compile_commands//:refresh_all

bazel query "filter('test_', //...)" | xargs bazel test --test_output=all --cache_test_results=no --config=native
# or
bazel test --config=native //:test_dash_remuxer --test_output=all

bazel build --config=wasm //:ikaria_js
```

## Legal
The ffmpeg used in this library was built using the `--disable-gpl` flag. The developer of this library recognizes that linking this library does not violate the GPL's derivative clauses.

Furthermore, this library does not link decoders/encoders for codecs that are generally considered to be patent-protected, such as H.264, and does not support such codecs.

However, this does not mean that users can avoid legal liability arising from the use of this library.

The developers, contributors, affiliated organizations, and licensors of this library are not liable for any damages (including legal claims and lawsuits) arising from the use of this library.
