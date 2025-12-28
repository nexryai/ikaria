FROM emscripten/emsdk as emsdk
FROM ubuntu:noble

# Copy include files from emsdk
COPY --from=emsdk /emsdk/upstream/emscripten/system/include /emsdk/upstream/emscripten/system/include/

# Install additional dependencies
RUN apt-get update && \
    apt-get install -y git curl wget clangd-19 build-essential autoconf automake libtool pkg-config gnupg ffmpeg googletest libavcodec-dev libavformat-dev libavutil-dev libswscale-dev && \
    curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor >bazel-archive-keyring.gpg && \
    mv bazel-archive-keyring.gpg /usr/share/keyrings && \
    echo "deb [arch=amd64 signed-by=/usr/share/keyrings/bazel-archive-keyring.gpg] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list && \
    apt update && apt install -y bazel
