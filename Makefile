js/src/ikaria-wasm.js:
	mkdir -p dist && \
	emcc --bind \
	-O3 \
	-L/opt/ffmpeg/lib \
	-I/opt/ffmpeg/include/ \
	-s EXPORTED_RUNTIME_METHODS="[FS, cwrap, ccall, getValue, setValue, writeAsciiToMemory]" \
	-s INITIAL_MEMORY=268435456 \
	-s EXPORT_ES6=1 --emit-tsd ikaria-wasm.d.ts \
	-lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil -lm \
	-pthread \
	-lworkerfs.js \
	-o js/src/ikaria-wasm.js \
	src/ikaria.cpp

clean:
	rm -f js/src/ikaria-wasm.js
	rm -f js/src/ikaria-wasm.wasm
