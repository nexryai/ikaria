dist/ffprobe-wasm.js:
	mkdir -p dist && \
	emcc --bind \
	-O3 \
	-L/opt/ffmpeg/lib \
	-I/opt/ffmpeg/include/ \
	-s EXPORTED_RUNTIME_METHODS="[FS, cwrap, ccall, getValue, setValue, writeAsciiToMemory]" \
	-s INITIAL_MEMORY=268435456 \
	-s EXPORT_ES6=1 --emit-tsd ikaria.d.ts \
	-lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -lm \
	-pthread \
	-lworkerfs.js \
	-o dist/ffprobe-wasm.js \
	src/ffprobe-wasm-wrapper.cpp