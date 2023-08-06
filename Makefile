

all: mime dist/ffmime-wasm.js

clean:
	rm -f ffmime

mime:
	g++ main.cpp -o ffmime -DNATIVE_CLI -lavformat

dist/ffmime-wasm.js:
	mkdir -p dist && \
	emcc --bind \
	-O3 \
	-L/opt/ffmpeg/lib \
	-I/opt/ffmpeg/include/ \
	-s EXPORTED_RUNTIME_METHODS="[FS, cwrap, ccall, getValue, setValue, writeAsciiToMemory, getmime]" \
	-s INITIAL_MEMORY=268435456 \
	-lavformat -lavutil -lavcodec -lm \
	-lnodefs.js \
	-o dist/ffmime-wasm.js \
	main.cpp