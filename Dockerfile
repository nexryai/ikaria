FROM emscripten/emsdk:4.0.8 as build

ARG PREFIX=/opt/ffmpeg
ARG MAKEFLAGS="-j4"

RUN apt-get update && apt-get install -y autoconf libtool build-essential npm && npm install -g typescript

# libvpx
ARG LIBVPX_VERSION=1.15.1

RUN cd /tmp && git clone https://chromium.googlesource.com/webm/libvpx && \
  cd libvpx && \
  git checkout v${LIBVPX_VERSION} && \
  emconfigure ./configure \
  --target=generic-gnu \
  --prefix=${PREFIX} \
  --disable-examples \
  --disable-unit-tests \
  --disable-install-bins \
  --disable-tools \
  --disable-docs \
  --enable-pic

RUN cd /tmp/libvpx && \
  emmake make && emmake make install

# opus
ARG OPUS_VERSION=1.5.2

# ref: https://github.com/ffmpegwasm/ffmpeg.wasm/blob/efac2471225bf7b3dc7f394886437f394a74f0c9/build/opus.sh
RUN cd /tmp && git clone https://github.com/xiph/opus && \
  cd opus && \
  git checkout v${OPUS_VERSION} && \
  emconfigure ./autogen.sh && \
  emconfigure ./configure \
  --prefix=${PREFIX} \
  --host=i686-none \
  --enable-static \
  --disable-shared \
  --disable-asm \
  --disable-rtcd \
  --disable-intrinsics \
  --disable-doc \
  --disable-extra-programs

RUN cd /tmp/opus && \
  emmake make && emmake make install

# libmp3lame
ARG LAME_VERSION=3.100 

RUN cd /tmp && \
  wget -O lame-${LAME_VERSION}.tar.gz https://sourceforge.net/projects/lame/files/lame/${LAME_VERSION}/lame-${LAME_VERSION}.tar.gz/download && \
  tar zxf lame-${LAME_VERSION}.tar.gz

RUN cd /tmp/lame-${LAME_VERSION} && \
  emconfigure ./configure \
  --prefix=${PREFIX} \
  --host=i686-gnu \
  --enable-static \
  --disable-frontend

RUN cd /tmp/lame-${LAME_VERSION} && \
  emmake make && emmake make install 

# Get ffmpeg source.
ARG FFMPEG_VERSION=7.1.1

RUN cd /tmp/ && \
  wget http://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.gz && \
  tar zxf ffmpeg-${FFMPEG_VERSION}.tar.gz && rm ffmpeg-${FFMPEG_VERSION}.tar.gz

ARG CFLAGS="-s USE_PTHREADS=1 -O3 -I${PREFIX}/include"
ARG LDFLAGS="$CFLAGS -L${PREFIX}/lib -s INITIAL_MEMORY=33554432"

# Compile ffmpeg.
RUN cd /tmp/ffmpeg-${FFMPEG_VERSION} && \
  emconfigure ./configure \
  --prefix=${PREFIX} \
  --target-os=none \
  --arch=x86_32 \
  --enable-cross-compile \
  --disable-debug \
  --disable-x86asm \
  --disable-inline-asm \
  --disable-stripping \
  --disable-programs \
  --disable-doc \
  --disable-all \
  --enable-avcodec \
  --enable-avformat \
  --enable-avfilter \
  --enable-avdevice \
  --enable-avutil \
  --enable-swresample \
  --enable-postproc \
  --enable-swscale \
  --enable-protocol=file \
  --enable-decoder=aac,pcm_s16le,mp3 \
  # `ffmpeg -formats` で確認できるmuxer,demuxerを指定
  --enable-demuxer=mov,matroska,mp3,webm \
  --enable-muxer=mp4,webm \
  --disable-gpl \
  --enable-libvpx \
  --enable-libopus \
  --enable-libmp3lame \
  --extra-cflags="$CFLAGS" \
  --extra-cxxflags="$CFLAGS" \
  --extra-ldflags="$LDFLAGS" \
  --nm="llvm-nm -g" \
  --ar=emar \
  --as=llvm-as \
  --ranlib=emranlib \
  --cc=emcc \
  --cxx=em++ \
  --objcc=emcc \
  --dep-cc=emcc

RUN cd /tmp/ffmpeg-${FFMPEG_VERSION} && \
  emmake make -j4 && \
  emmake make install


COPY ./src/*.cpp /build/src/
COPY ./Makefile /build/Makefile

WORKDIR /build

ENV PATH="${PATH}:/opt/ffmpeg/bin"
#RUN make