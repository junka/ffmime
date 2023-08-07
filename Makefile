
.PHONY: all

all: mime

clean:
	rm -f dist/ffmime

mime:
	g++ main.cpp -o dist/ffmime -DNATIVE_CLI -lavformat -lavcodec -lavutil

dist/ffmime-wasm.js:
	mkdir -p dist && \
	emcc --bind \
	-O3 \
	-L/opt/ffmpeg/lib \
	-I/opt/ffmpeg/include/ \
	-s EXPORTED_RUNTIME_METHODS="[FS, cwrap, ccall, getValue, setValue, writeAsciiToMemory]" \
	-s INITIAL_MEMORY=268435456 \
	-lavformat -lavutil -lavcodec -lm \
	-lnodefs.js \
	-o dist/ffmime-wasm.js \
	main.cpp