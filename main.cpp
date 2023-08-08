/* SPDX-License-Identifier: ISC
 * Copyright(c) 2023 junka
 */
#include <string.h>
#ifndef NATIVE_CLI
#include <emscripten.h>
#include <emscripten/bind.h>
#endif

#include <string>
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

#ifdef av_fourcc2str
#undef av_fourcc2str
av_always_inline char *av_fourcc2str(unsigned int fourcc) {
  char str[AV_FOURCC_MAX_STRING_SIZE];
  memset(str, 0, sizeof(str));
  return av_fourcc_make_string(str, fourcc);
}
#endif

// using namespace emscripten;

// https://developer.mozilla.org/en-US/docs/Web/Media/Formats/codecs_parameter
/*
audio/mpeg
An audio file using the MPEG file type, such as an MP3.

video/ogg; codecs=vorbis
A video file using the Ogg file type.

video/mp4; codecs="avc1.4d002a"
A video file using the MPEG-4 file type.

video/quicktime
A video file in Apple's QuickTime format. As noted elsewhere, this format was
once commonly used on the web but no longer is, since it required a plugin to
use.

video/webm; codecs="vp8, vorbis"
A WebM file containing VP8 video and/or Vorbis audio.

audio/3gpp
video/3gpp
audio/3gp2
video/3gp2
audio/mp4
application/mp4

---

av01.P.LLT.DD[.M.CCC.cp.tc.mc.F]
cccc[.pp]* (Generic ISO BMFF)
mp4v.oo[.V] (MPEG-4 video)
mp4a.oo[.A] (MPEG-4 audio)
avc1[.PPCCLL] (AVC video)


see samples here
https://cconcolato.github.io/media-mime-support/

https://wiki.whatwg.org/wiki/video_type_parameters#MPEG

*/

// standard of std::format is not supported by emscripten now
static std::string string_format(const char *fmt, ...) {
  char *ret;
  va_list ap;

  va_start(ap, fmt);
  vasprintf(&ret, fmt, ap);
  va_end(ap);

  std::string str(ret);
  free(ret);

  return str;
}

std::string getmime(const std::string filename) {
  av_log_set_level(AV_LOG_QUIET);
  const char * aot_str[0x20] = {
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "45",
    "40",
    "41",
    "4A",
    "4C",
    "4B",
    "",
    "42",
    "",
    "43",
    "",
    "",
    "",
    "44",
  };
  const AVOutputFormat *format = av_guess_format(NULL, filename.c_str(), NULL);
  enum AVMediaType type = AVMEDIA_TYPE_UNKNOWN;
  AVFormatContext * fmt_ctx = NULL;
#ifdef ENABLE_AVCODEC
  AVCodecContext *avctx = NULL;
#endif
  std::string codecs;
  std::string mime;

  if (format && format->mime_type && !strncmp(format->mime_type, "text", 4)) {
    mime = format->mime_type;
    return mime;
  }

  if (avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL) != 0) {
    return NULL;
  }
  const AVInputFormat *iformat = fmt_ctx->iformat;

  if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
    // printf("error in get stream info\n");
    avformat_close_input(&fmt_ctx);
    return NULL;
  }
#ifdef ENABLE_AVCODEC
  avctx = avcodec_alloc_context3(NULL);
  if (!avctx) {
    avformat_close_input(&fmt_ctx);
    return NULL;
  }
#endif

  // av_dump_format(fmt_ctx, 0, filename, 0);
  for (int i = 0; i < (int)fmt_ctx->nb_streams; i++) {
    AVStream *s = fmt_ctx->streams[i];
#ifdef ENABLE_AVCODEC
    if (avcodec_parameters_to_context(avctx, s->codecpar) < 0) {
      avcodec_free_context(&avctx);
      avformat_close_input(&fmt_ctx);
      return NULL;
    }
#endif
    if (codecs.length() > 0) {
      codecs += ", ";
    }
    if (AVMEDIA_TYPE_VIDEO == s->codecpar->codec_type) {
      type = AVMEDIA_TYPE_VIDEO;
    } else if (AVMEDIA_TYPE_AUDIO == s->codecpar->codec_type) {
      if (type == AVMEDIA_TYPE_UNKNOWN) {
        type = AVMEDIA_TYPE_AUDIO;
      }
    }
#if 0
    char streamCodecsString[256];
    avcodec_string(streamCodecsString, sizeof(streamCodecsString), avctx, 0);
    printf("%d: %s\n", s->codecpar->extradata_size, streamCodecsString);
#endif
    switch (s->codecpar->codec_id) {
      case AV_CODEC_ID_H264:
      //see AVCDecoderConfigurationRecord
      // ff_h264_decode_extradata(s->codecpar->extradata, s->codecpar->extradata_size,);
      if (s->codecpar->extradata_size > 4 &&
          s->codecpar->extradata[0] == 1) {
        codecs += string_format("%s.%02x%02x%02x",
                       av_fourcc2str(s->codecpar->codec_tag),
                       s->codecpar->extradata[1], s->codecpar->extradata[2],
                       s->codecpar->extradata[3]);
      } else if (s->codecpar->extradata_size > 12 &&
                 s->codecpar->extradata[0] == 0 &&
                 s->codecpar->extradata[1] == 0 &&
                 s->codecpar->extradata[2] == 0 &&
                 s->codecpar->extradata[3] == 1) {
        //raw SPS
        codecs += string_format(
            "%s.%02x%02x%02x", av_fourcc2str(s->codecpar->codec_tag),
            s->codecpar->extradata[10], s->codecpar->extradata[11],
            s->codecpar->extradata[12]);
      } else {
        codecs += string_format("%s.%02x%02x%02x",
                                av_fourcc2str(s->codecpar->codec_tag),
                                s->codecpar->profile, 0, s->codecpar->level);
      }
      break;
      case AV_CODEC_ID_HEVC:
      // HEVCDecoderConfigurationRecord
      if (s->codecpar->extradata_size > 4 && s->codecpar->extradata[0] == 1) {
        codecs += string_format("%s.%d.%d.L%d.B%d",
                                av_fourcc2str(s->codecpar->codec_tag),
                                s->codecpar->extradata[1] & 0x1F,
                                ((s->codecpar->extradata[1] >> 5) & 0x1) + 5,
                                s->codecpar->extradata[12],
                                (s->codecpar->extradata[11] & 0x1)
                                    ? 0
                                    : ((s->codecpar->extradata[5] >> 3) & 0x3));
      } else {
        codecs += string_format(
            "%s.%d.%d.L%d.B%d", av_fourcc2str(s->codecpar->codec_tag),
            s->codecpar->profile, ((s->codecpar->profile >> 5) & 0x1) + 5,
            s->codecpar->level, ((s->codecpar->level >> 1) & 0x7));
      }
        break;
      case AV_CODEC_ID_AV1:
        // AVCDecoderConfigurationRecord
        if (s->codecpar->extradata_size > 4 && s->codecpar->extradata[0] == 0x81) {
        codecs += string_format(
            "av01.%d.%02d%c.%02d", (s->codecpar->extradata[1] >> 5) & 0x7,
            s->codecpar->extradata[1] & 0x1F,
            (s->codecpar->extradata[2] >> 7) & 0x1 ? 'H' : 'M',
            (s->codecpar->extradata[2] >> 5) & 0x1
                ? 12
                : (((s->codecpar->extradata[2] >> 6) & 0x1) ? 10 : 8));
        } else {
        codecs +=
            string_format("av01.%d.%02d%c.%02d",
                          //  avcodec_get_name(s->codecpar->codec_id),
                          s->codecpar->profile, s->codecpar->level, 'M', 10);
        }
        break;
      case AV_CODEC_ID_ALAC:
      case AV_CODEC_ID_AAC:
        if (s->codecpar->codec_tag) {
          codecs += av_fourcc2str(s->codecpar->codec_tag);
          if (s->codecpar->extradata_size > 2) {
            codecs += ".";
            codecs += aot_str[s->codecpar->extradata[0] & 0x1F];
            codecs += string_format(".%d", (s->codecpar->extradata[1] - 0x80) >> 3);
          } else {
            codecs += ".40";
            // FF_PROFILE_UNKNOWN
            if (s->codecpar->profile != -99) {
              codecs += string_format(".%d", s->codecpar->profile);
            }
          }
        } else {
        codecs += string_format("%s", avcodec_get_name(s->codecpar->codec_id));
        }

        break;
      
      case AV_CODEC_ID_PCM_S16LE:
        codecs += string_format("%d", 0);
        break;
      case AV_CODEC_ID_PCM_ALAW:
        codecs += string_format("%d", 1);
        break;
      case AV_CODEC_ID_ADPCM_MS:
        codecs += string_format("%d", 2);
        break;
      case AV_CODEC_ID_PCM_F32LE:
        codecs += string_format("%d", 3);
        break;
      case AV_CODEC_ID_ADPCM_IMA_WAV:
        codecs += string_format("%d", 17);
        break;
      case AV_CODEC_ID_MP2:
      case AV_CODEC_ID_MP3:
        codecs += string_format("%d", 85);
        break;
      case AV_CODEC_ID_MPEG4:
        codecs += av_fourcc2str(s->codecpar->codec_tag);
        break;
      case AV_CODEC_ID_MJPEG:
        mime = "image/jpeg";
        break;
      case AV_CODEC_ID_JPEG2000:
        mime = "image/jp2";
        break;
      case AV_CODEC_ID_PNG:
        mime = "image/png";
        break;
      case AV_CODEC_ID_PPM:
        mime = "image/x-portable-pixmap";
        break;
      case AV_CODEC_ID_WEBP:
        mime = "image/webp";
        break;
      case AV_CODEC_ID_TIFF:
        mime = "image/tiff";
        break;
      case AV_CODEC_ID_GIF:
        mime = "image/gif";
        break;
      case AV_CODEC_ID_BMP:
        mime = "image/bmp";
        break;
      case AV_CODEC_ID_PAM:
        mime = "image/x-portable-anymap";
        break;
      case AV_CODEC_ID_XBM:
        mime = "image/xbm";
        break;
      case AV_CODEC_ID_XPM:
        mime = "image/x-xpixmap";
        break;
      case AV_CODEC_ID_JPEGLS:
        mime = "image/jpeg-ls";
        break;
      case AV_CODEC_ID_PIXLET:
        mime = "image/vnd.dxf";
        break;

      case AV_CODEC_ID_VP8:
      case AV_CODEC_ID_VP9:
      case AV_CODEC_ID_VORBIS:
      case AV_CODEC_ID_OPUS:
      case AV_CODEC_ID_FLV1:
        codecs += avcodec_get_name(s->codecpar->codec_id);
        break;
      default:
        codecs += avcodec_get_name(s->codecpar->codec_id);
        break;
      }
  }

  avformat_close_input(&fmt_ctx);

  if (0 == mime.compare(0, 5, "image")) {
    return mime;
  }

  if (format && format->mime_type) {
    mime += format->mime_type;
  } else if (format && !format->mime_type) {
    mime += string_format("%s/", av_get_media_type_string(type));
    if (format->name) {
      if (!strcmp(format->name, "mov")) {
        mime += "quicktime";
      } else {
        mime += format->name;
      }
    }
  } else if (!format) {
    mime += string_format("%s/", av_get_media_type_string(type));
    if (iformat->extensions) {
      if (iformat->mime_type) {
        mime += iformat->mime_type;
      } else if (iformat->name) {
        mime += iformat->name;
      }
    }
  }

  // printf("%s", mime);
  if (codecs.length() > 0 && (!mime.compare(0, 5, "video", 5) || !mime.compare(0, 5, "audio", 5))) {
      mime += "; ";

      if (codecs.find(',') != std::string::npos) {
        mime += "codecs=\"" + codecs + "\"";
      } else {
        mime += "codecs=" + codecs;
      }
  }

  return mime;
}

#ifdef NATIVE_CLI
int main(int argc, char **argv)
{
  std::string filename = argv[1];
  std::string mime = getmime(filename);
  std::cout << mime << std::endl;
  return 0;
}
#else
EMSCRIPTEN_BINDINGS(ffmime) {
  emscripten::function("getmime", &getmime);
}
#endif

