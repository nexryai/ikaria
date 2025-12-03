FROM emscripten/emsdk:4.0.21 as build

ARG PREFIX=/opt/ffmpeg

RUN apt-get update && apt-get install -y autoconf libtool build-essential npm && npm install -g typescript

# Install Task
RUN sh -c "$(curl --location https://taskfile.dev/install.sh)" -- -d -b /usr/local/bin

COPY Taskfile.yml .

RUN task build-deps

WORKDIR /build

ENV PATH="${PATH}:/opt/ffmpeg/bin"
#RUN make
