FROM ubuntu:rolling

RUN apt-get update && \
    apt-get install -y git xz-utils curl clang-20 clangd-20 lld-20 llvm-20-linker-tools autoconf automake libtool pkg-config && \
    curl -L https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-amd64 -o /usr/local/bin/bazel && chmod +x /usr/local/bin/bazel && \
    su ubuntu sh -c 'curl -fsSL https://get.pnpm.io/install.sh | ENV="$HOME/.bashrc" SHELL="$(which bash)" bash -'
