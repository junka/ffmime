FROM emscripten/emsdk:3.1.40 as build

ARG FFMPEG_VERSION=5.1.3
ARG PREFIX=/opt/ffmpeg

RUN apt-get update && apt-get install -y autoconf libtool build-essential llvm

# Get ffmpeg source.
RUN cd /root/ && \
  wget http://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.gz && \
  tar zxf ffmpeg-${FFMPEG_VERSION}.tar.gz && rm ffmpeg-${FFMPEG_VERSION}.tar.gz

ARG CFLAGS="-O3"
ARG LDFLAGS="$CFLAGS -s INITIAL_MEMORY=33554432"

# Compile ffmpeg.
RUN cd /root/ffmpeg-${FFMPEG_VERSION} && \
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
  --disable-bsfs \
  --disable-network \
  --disable-optimizations \
  --disable-runtime-cpudetect \
  --disable-hwaccels \
  --enable-avcodec \
  --enable-avformat \
  --disable-logging \
  --disable-avfilter \
  --disable-avdevice \
  --enable-avutil \
  --disable-swresample \
  --disable-postproc \
  --disable-swscale \
  --disable-everything \
  --enable-protocol=file \
  --disable-encoders \
  --disable-muxers \
  --disable-filters \
  --disable-outdevs \
  --disable-decoders \
  --enable-demuxers \
  --disable-iconv \
  --disable-v4l2-m2m \
  --disable-amd3dnow \
  --disable-mmx \
  --disable-avx512 \
  --disable-vulkan \
  --disable-dct \
  --disable-fft --disable-dwt \
  --disable-lsp \
  --disable-mdct \
  --disable-rdft --disable-faan \
  --disable-pixelutils \
  --disable-valgrind-backtrace \
  --enable-protocol=file \
  --enable-gpl \
  --enable-version3 \
  --disable-pthreads \
  --disable-w32threads \
  --disable-os2threads \
  --extra-cflags="$CFLAGS" \
  --extra-cxxflags="$CFLAGS" \
  --extra-ldflags="$LDFLAGS" \
  --nm="llvm-nm -g" \
  --ar=emar \
  --as=llvm-as \
  --ranlib=llvm-ranlib \
  --cc=emcc \
  --cxx=em++ \
  --objcc=emcc \
  --dep-cc=emcc

RUN cd /root/ffmpeg-${FFMPEG_VERSION} && \
  emmake make -j4 && \
  emmake make install


COPY ./main.cpp /build/main.cpp
COPY ./Makefile /build/Makefile

WORKDIR /build

RUN make dist/ffmime-wasm.js


FROM scratch AS mime-exporter
COPY --from=build /build/dist/* .