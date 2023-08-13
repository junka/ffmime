/* SPDX-License-Identifier: ISC
 * Copyright(c) 2023 junka
 */
#include <cstddef>
#include <string.h>
#include <vector>
#ifndef NATIVE_CLI
#include <emscripten.h>
#include <emscripten/bind.h>
#endif

#include <cstdlib>
#include <map>
#include <string>
#include <unordered_set>
#ifdef NATIVE_CLI
#include <iostream>
#endif

extern "C" {
#ifdef ENABLE_AVCODEC
#include <libavcodec/avcodec.h>
#endif
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

typedef void (*onPacket)(int type, uint8_t * data, int size, int64_t ts, int64_t duration);

std::map<int, int> streamidx;

AVFormatContext *fmt_ctx = NULL;

int demux_init(const std::string filename)
{
    av_log_set_level(AV_LOG_QUIET);

    AVStream *s;
    int ret;

    ret = avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL);
    if (ret != 0) {
      // std::cout << "fail to open " << filename << AVERROR(ret) << std::endl;
      return ret;
    }
    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0) {
      avformat_close_input(&fmt_ctx);
      return ret;
    }

#ifdef NATIVE_CLI
    std::cout << fmt_ctx->nb_streams << std::endl;
#endif
    for (int i = 0; i < (int)fmt_ctx->nb_streams; i++) {
      s = fmt_ctx->streams[i];
#ifdef NATIVE_CLI
      std::cout << s->id << "," << s->codecpar->codec_type << ","
                << s->nb_frames << "," << s->duration << std::endl;
#endif
      streamidx[s->id] = i;
    }

    return 0;
}

int demux_streams(std::vector<int> sids, onPacket onVideoPacket,
                          onPacket onAudioPacket) {
    AVPacket *packet = av_packet_alloc();
    std::unordered_set<int> indexes;
    for (int i = 0; i < sids.size(); i++) {
      indexes.insert(streamidx.find(i)->second);
    }
    while (av_read_frame(fmt_ctx, packet) >= 0) {
      if (indexes.find(packet->stream_index) != indexes.end()) {
        AVStream *s = fmt_ctx->streams[packet->stream_index];
        if (s->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && onVideoPacket != nullptr) {
          onVideoPacket((packet->flags & AV_PKT_FLAG_KEY), packet->data,
                        packet->size, packet->pts, packet->duration);
        } else if (s->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && onAudioPacket != nullptr) {
          onAudioPacket((packet->flags & AV_PKT_FLAG_KEY), packet->data,
                        packet->size, packet->pts, packet->duration);
        } else if (s->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
        }
      }
      av_packet_unref(packet);
    }

    avformat_close_input(&fmt_ctx);
    return 0;
}

#ifdef NATIVE_CLI

void onProcessPacket(int type, uint8_t * data, int size, int64_t ts, int64_t duration)
{
    std::cout << size << "," << type << "," << duration << ","<< std::endl;
}

int main(int argc, char **argv) {
  int i = 2;
  std::string filename = argv[1];
  std::vector<int> v;
  if (argc >=3 ) {
    while (i < argc) {
        v.push_back(atoi(argv[i]));
        i ++;
    }
  }
  demux_init(filename);
  int ret = demux_streams(v, onProcessPacket, nullptr);
//   std::cout << mime << std::endl;
  return 0;
}
#else
EMSCRIPTEN_BINDINGS(ffmime) {
    emscripten::function("demux_init", &demux_init);
    emscripten::function("demux_streams", &demux_streams,
                         emscripten::allow_raw_pointers());
}
#endif
