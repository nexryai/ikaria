FROM emscripten/emsdk as emsdk
FROM ubuntu:noble

# Copy include files from emsdk
COPY --from=emsdk /emsdk/upstream/emscripten/system/include /emsdk/upstream/emscripten/system/include/

# Install additional dependencies
RUN apt-get update && \
    apt-get install -y clangd-19 libavcodec-dev libavformat-dev libavutil-dev libswscale-dev
