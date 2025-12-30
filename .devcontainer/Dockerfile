FROM ubuntu:rolling

RUN apt-get update && \
    # DO NOT REMOVE wget - required to build libopus ¯\_(ツ)_/¯
    apt-get install -y git git-lfs xz-utils curl wget clang-21 clangd-21 autoconf automake libtool pkg-config && \
    curl -L https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-amd64 -o /usr/local/bin/bazel && chmod +x /usr/local/bin/bazel && \
    curl https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz -o ffmpeg-static.tar.xz && \
    mkdir ffmpeg-static && tar -xf ffmpeg-static.tar.xz -C ffmpeg-static --strip-components=1 && rm ffmpeg-static.tar.xz && mv ffmpeg-static/ffmpeg /usr/local/bin && rm -rf ffmpeg-static.tar.xz ffmpeg-static
