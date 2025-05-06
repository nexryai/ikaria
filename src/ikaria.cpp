#include <stdexcept>
#include <string>
#include <vector>
#include <inttypes.h>
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/bprint.h>
#include <libavutil/imgutils.h>
};

const std::string c_avformat_version() {
    return AV_STRINGIFY(LIBAVFORMAT_VERSION);
}

const std::string c_avcodec_version() {
    return AV_STRINGIFY(LIBAVCODEC_VERSION);
}

const std::string c_avutil_version() {
    return AV_STRINGIFY(LIBAVUTIL_VERSION);
}

typedef struct Tag {
  std::string key;
  std::string value;
} Tag;

typedef struct Stream {
  int id;
  float start_time;
  float duration;
  int codec_type;
  std::string codec_name;
  std::string format;
  float bit_rate;
  std::string profile;
  int level;
  int width;
  int height;
  int sample_rate;
  int frame_size;
  std::vector<Tag> tags;
} Stream;

typedef struct Chapter {
  int id;
  std::string time_base;
  float start;
  float end;
  std::vector<Tag> tags;
} Chapter;

typedef struct Frame {
  int frame_number;
  char pict_type;
  int pts;
  int dts;
  int pos;
  int pkt_size;
} Frame;

typedef struct FileInfoResponse {
  std::string name;
  float bit_rate;
  float duration;
  std::string url;
  int nb_streams;
  int flags;
  std::vector<Stream> streams;
  int nb_chapters;
  std::vector<Chapter> chapters;
} FileInfoResponse;

typedef struct FramesResponse {
  std::vector<Frame> frames;
  int nb_frames;
  int gop_size;
  float duration;
  double time_base;
  double avg_frame_rate;
} FramesResponse;

typedef struct Keyframe {
  double pts_time;
  std::string pts_time_string;
} Keyframe;

typedef struct VideoInfoResponse {
    double duration;
    int width;
    int height;
    std::string videoCodec;
    std::string audioCodec;
    std::vector<Keyframe> keyframes;
} VideoInfoResponse;

FileInfoResponse get_file_info(const std::string filename) {
    // printf("File: %s\n", filename.c_str());
    av_log_set_level(AV_LOG_QUIET); // No logging output for libav.

    FILE *file = fopen(filename.c_str(), "rb");
    if (!file) {
      printf("cannot open file\n");
    }
    fclose(file);

    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (!pFormatContext) {
      printf("ERROR: could not allocate memory for Format Context\n");
    }

    // Open the file and read header.
    int ret;
    if ((ret = avformat_open_input(&pFormatContext, filename.c_str(), NULL, NULL)) < 0) {
        printf("ERROR: %s\n", av_err2str(ret));
    }

    // Get stream info from format.
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
      printf("ERROR: could not get stream info\n");
    }

    // Initialize response struct with format data.
    FileInfoResponse r = {
      .name = pFormatContext->iformat->name,
      .bit_rate = (float)pFormatContext->bit_rate,
      .duration = (float)pFormatContext->duration,
      .url = pFormatContext->url,
      .nb_streams = (int)pFormatContext->nb_streams,
      .flags = pFormatContext->flags,
      .nb_chapters = (int)pFormatContext->nb_chapters
    };

    // Loop through the streams.
    for (int i = 0; i < pFormatContext->nb_streams; i++) {
      AVCodecParameters *pLocalCodecParameters = NULL;
      pLocalCodecParameters = pFormatContext->streams[i]->codecpar;

      // Convert to char byte array.
      uint32_t n = pLocalCodecParameters->codec_tag;
      char fourcc[5];
      for (int j = 0; j < 4; ++j) {
        fourcc[j] = (n >> (j * 8) & 0xFF);
      }
      fourcc[4] = 0x00; // NULL terminator.

      Stream stream = {
        .id = (int)pFormatContext->streams[i]->id,
        .start_time = (float)pFormatContext->streams[i]->start_time,
        .duration = (float)pFormatContext->streams[i]->duration,
        .codec_type = (int)pLocalCodecParameters->codec_type,
        .codec_name = fourcc,
        .format = av_get_pix_fmt_name((AVPixelFormat)pLocalCodecParameters->format),
        .bit_rate = (float)pLocalCodecParameters->bit_rate,
        .profile = avcodec_profile_name(pLocalCodecParameters->codec_id, pLocalCodecParameters->profile),
        .level = (int)pLocalCodecParameters->level,
        .width = (int)pLocalCodecParameters->width,
        .height = (int)pLocalCodecParameters->height,
        .sample_rate = (int)pLocalCodecParameters->sample_rate,
        .frame_size = (int)pLocalCodecParameters->frame_size,
      };

      // Add tags to stream.
      const AVDictionaryEntry *tag = NULL;
      while ((tag = av_dict_get(pFormatContext->streams[i]->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        Tag t = {
          .key = tag->key,
          .value = tag->value,
        };
        stream.tags.push_back(t);
      }

      r.streams.push_back(stream);
      free(fourcc);
    }

    // Loop through the chapters (if any).
    for (int i = 0; i < pFormatContext->nb_chapters; i++) {
      AVChapter *chapter = pFormatContext->chapters[i];

      // Format timebase string to buf.
      AVBPrint buf;
      av_bprint_init(&buf, 0, AV_BPRINT_SIZE_AUTOMATIC);
      av_bprintf(&buf, "%d%s%d", chapter->time_base.num, (char *)"/", chapter->time_base.den);

      Chapter c = {
        .id = (int)chapter->id,
        .time_base = buf.str,
        .start = (float)chapter->start,
        .end = (float)chapter->end,
      };

      // Add tags to chapter.
      const AVDictionaryEntry *tag = NULL;
      while ((tag = av_dict_get(chapter->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        Tag t = {
          .key = tag->key,
          .value = tag->value,
        };
        c.tags.push_back(t);
      }

      r.chapters.push_back(c);
    }

    avformat_close_input(&pFormatContext);
    return r;
}

double getDurationFromPTS(AVFormatContext* fmt_ctx) {
    double max_pts_time = 0.0;
    AVPacket* pkt = av_packet_alloc();
    if (pkt == nullptr) {
        printf("ERROR: could not allocate packet\n");
        throw std::runtime_error("パケットの割り当てに失敗しました");
    }

    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        AVStream* stream = fmt_ctx -> streams[pkt -> stream_index];
        if (pkt -> pts != AV_NOPTS_VALUE) {
            double pts_time = pkt -> pts * av_q2d(stream->time_base);
            max_pts_time = std::max(max_pts_time, pts_time);
        }

        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);

    // シークで巻き戻す
    av_seek_frame(fmt_ctx, -1, 0, AVSEEK_FLAG_BACKWARD);
    return max_pts_time;
}

VideoInfoResponse getVideoInfo(const std::string filename) {
    avformat_network_init();
    AVFormatContext* fmt_ctx = nullptr;

    int err;
    if ((err = avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL)) < 0) {
        printf("failed to open file: %s\n", av_err2str(err));
        throw std::runtime_error("ファイルを開けませんでした");
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        avformat_close_input(&fmt_ctx);
        printf("ERROR: could not find stream information\n");
        throw std::runtime_error("ストリーム情報を取得できませんでした");
    }


    VideoInfoResponse info;
    info.duration = (fmt_ctx->duration != AV_NOPTS_VALUE)
                    ? (double)fmt_ctx->duration / AV_TIME_BASE
                    : getDurationFromPTS(fmt_ctx);

    for (unsigned i = 0; i < fmt_ctx->nb_streams; ++i) {
        AVStream* stream = fmt_ctx->streams[i];
        AVCodecParameters* codecpar = stream->codecpar;

        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            info.videoCodec = avcodec_get_name(codecpar->codec_id);
            info.width = codecpar->width;
            info.height = codecpar->height;

            // キーフレーム取得
            AVPacket* pkt = av_packet_alloc();
            if (pkt == nullptr) {
                printf("ERROR: could not allocate packet\n");
                throw std::runtime_error("パケットの割り当てに失敗しました");
            }

            while (av_read_frame(fmt_ctx, pkt) >= 0) {
                if (pkt -> stream_index == static_cast<int>(i)) {
                    if (pkt -> flags & AV_PKT_FLAG_KEY && pkt -> pts != AV_NOPTS_VALUE) {
                        const double pts_time = pkt -> pts * av_q2d(stream->time_base);
                        // printf("Key frame found at pts_time: %f\n", pts_time);
                        
                        // JavaScript側で扱いやすいようにstringに変換
                        // ffprobeのCLIの出力と同じ、"0.000000" 形式にする
                        static char buffer[64];
                        snprintf(buffer, sizeof(buffer), "%.6f", pts_time);
                        std::string pts_time_string(buffer);
                        
                        info.keyframes.push_back({
                            .pts_time = pts_time,
                            .pts_time_string = pts_time_string
                        });
                    }
                }
                av_packet_unref(pkt);
            }

            av_packet_free(&pkt);

            av_seek_frame(fmt_ctx, -1, 0, AVSEEK_FLAG_BACKWARD);
        } else if (codecpar -> codec_type == AVMEDIA_TYPE_AUDIO) {
            info.audioCodec = avcodec_get_name(codecpar -> codec_id);
        }
    }

    avformat_close_input(&fmt_ctx);
    return info;
}

void trimingWebM(const std::string inputFilePath, const std::string outputFilePath, const std::string start_time, const std::string end_time) {
    AVFormatContext* ctx = nullptr;
    
    int err;
    if ((err = avformat_open_input(&ctx, inputFilePath.c_str(), nullptr, nullptr)) < 0) {
        printf("failed to open file: %s\n", av_err2str(err));
        throw std::runtime_error("ファイルを開けませんでした");
    }

    if (avformat_find_stream_info(ctx, nullptr) < 0) {
        avformat_close_input(&ctx);
        printf("ERROR: could not find stream information\n");
        throw std::runtime_error("ストリーム情報を取得できませんでした");
    }

    // 出力ファイルの初期化
    AVFormatContext* out_ctx = nullptr;
    if ((err = avformat_alloc_output_context2(&out_ctx, nullptr, nullptr, outputFilePath.c_str())) < 0) {
        printf("ERROR: could not allocate output context\n");
        throw std::runtime_error("出力ファイルの初期化に失敗しました");
    }

    // ストリームのコピー
    printf("Copying streams parameters...\n");
    for (unsigned i = 0; i < ctx->nb_streams; ++i) {
        AVStream* in_stream = ctx->streams[i];
        AVStream* out_stream = avformat_new_stream(out_ctx, nullptr);
        if (!out_stream) {
            printf("ERROR: could not allocate stream\n");
            throw std::runtime_error("ストリームのコピーに失敗しました");
        }

        printf("Stream %d: codec_type=%d, codec_name=%s\n", i, in_stream->codecpar->codec_type, avcodec_get_name(in_stream->codecpar->codec_id));

        avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        out_stream->time_base = in_stream->time_base;
    }

    // 出力ファイルを開く
    if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
        if ((err = avio_open(&out_ctx->pb, outputFilePath.c_str(), AVIO_FLAG_WRITE)) < 0) {
            printf("failed to open output file: %s\n", av_err2str(err));
            throw std::runtime_error("出力ファイルを開けませんでした");
        }
    }

    // ヘッダを書き込む
    if ((err = avformat_write_header(out_ctx, nullptr)) < 0) {
        printf("ERROR: could not write header\n");
        throw std::runtime_error("ヘッダの書き込みに失敗しました");
    }

    // ストリームのトリミング
    // 秒からPTSに変換
    //int64_t start_time_pts = std::stod(start_time) * AV_TIME_BASE;
    //int64_t end_time_pts = std::stod(end_time) * AV_TIME_BASE;
    
    // printf("Trimming from %s to %s (pts: %lld to %lld)\n", start_time.c_str(), end_time.c_str(), start_time_pts, end_time_pts);

    for (unsigned i = 0; i < ctx->nb_streams; ++i) {
        AVStream* in_stream = ctx->streams[i];
        AVStream* out_stream = out_ctx->streams[i];

        // const double pts_time = pkt.pts * av_q2d(stream->time_base); で生成したPTSが渡されるので復元
        const int64_t start_time_pts = std::stod(start_time) / av_q2d(in_stream->time_base);
        const int64_t end_time_pts = std::stod(end_time) / av_q2d(in_stream->time_base);
        printf("Trimming from %s to %s (pts: %lld to %lld)\n", start_time.c_str(), end_time.c_str(), start_time_pts, end_time_pts);

        printf("Trimming Stream %d: codec_type=%d, codec_name=%s\n", i, in_stream->codecpar->codec_type, avcodec_get_name(in_stream->codecpar->codec_id));

        int copied_pkts = 0;

        // ストリームの開始時間を設定
        if (av_seek_frame(ctx, i, start_time_pts, AVSEEK_FLAG_BACKWARD) < 0) {
            printf("ERROR: could not seek to start time\n");
            throw std::runtime_error("開始時間のシークに失敗しました");
        }

        // パケットの読み込みと書き込み
        AVPacket pkt;
        while (av_read_frame(ctx, &pkt) >= 0) {
            if (pkt.stream_index == static_cast<int>(i)) {
                if (pkt.pts >= start_time_pts && pkt.pts <= end_time_pts) {
                    // パケットのPTS/DTSを更新
                    pkt.pts = av_rescale_q(pkt.pts, in_stream->time_base, out_stream->time_base);
                    pkt.dts = av_rescale_q(pkt.dts, in_stream->time_base, out_stream->time_base);
                    pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
                    pkt.pos = -1;

                    // パケットを書き込む
                    if ((err = av_interleaved_write_frame(out_ctx, &pkt)) < 0) {
                        printf("ERROR: could not write frame\n");
                        throw std::runtime_error("フレームの書き込みに失敗しました");
                    }

                    copied_pkts++;
                }
                
                // 終了時間を超えたらループを抜ける
                if (pkt.pts > end_time_pts) {
                    break;
                }

                av_packet_unref(&pkt);
            }
        }

        printf("Copied %d packets from stream %d\n", copied_pkts, i);
    }

    // ヘッダを書き込む
    printf("Writing trailer...\n");
    if ((err = av_write_trailer(out_ctx)) < 0) {
        printf("ERROR: could not write trailer\n");
        throw std::runtime_error("トレーラーの書き込みに失敗しました");
    }

    // 出力ファイルを閉じる
    if (out_ctx->pb) {
        avio_closep(&out_ctx->pb);
    }

    avformat_free_context(out_ctx);
    avformat_close_input(&ctx);

    return;
}

EMSCRIPTEN_BINDINGS(constants) {
    function("AVFORMAT_VERSION", &c_avformat_version);
    function("AVCODEC_VERSION", &c_avcodec_version);
    function("AVUTIL_VERSION", &c_avutil_version);
}

EMSCRIPTEN_BINDINGS(structs) {
  emscripten::value_object<Tag>("Tag")
  .field("key", &Tag::key)
  .field("value", &Tag::value)
  ;
  register_vector<Tag>("Tag");

  emscripten::value_object<Stream>("Stream")
  .field("id", &Stream::id)
  .field("start_time", &Stream::start_time)
  .field("duration", &Stream::duration)
  .field("codec_type", &Stream::codec_type)
  .field("codec_name", &Stream::codec_name)
  .field("format", &Stream::format)
  .field("bit_rate", &Stream::bit_rate)
  .field("profile", &Stream::profile)
  .field("level", &Stream::level)
  .field("width", &Stream::width)
  .field("height", &Stream::height)
  .field("sample_rate", &Stream::sample_rate)
  .field("frame_size", &Stream::frame_size)
  .field("tags", &Stream::tags)
  ;
  register_vector<Stream>("Stream");

  emscripten::value_object<Chapter>("Chapter")
  .field("id", &Chapter::id)
  .field("time_base", &Chapter::time_base)
  .field("start", &Chapter::start)
  .field("end", &Chapter::end)
  .field("tags", &Chapter::tags)
  ;
  register_vector<Chapter>("Chapter");

  emscripten::value_object<Frame>("Frame")
  .field("frame_number", &Frame::frame_number)
  .field("pict_type", &Frame::pict_type)
  .field("pts", &Frame::pts)
  .field("dts", &Frame::dts)
  .field("pos", &Frame::pos)
  .field("pkt_size", &Frame::pkt_size);
  register_vector<Frame>("Frame");

  emscripten::value_object<FileInfoResponse>("FileInfoResponse")
  .field("name", &FileInfoResponse::name)
  .field("duration", &FileInfoResponse::duration)
  .field("bit_rate", &FileInfoResponse::bit_rate)
  .field("url", &FileInfoResponse::url)
  .field("nb_streams", &FileInfoResponse::nb_streams)
  .field("flags", &FileInfoResponse::flags)
  .field("streams", &FileInfoResponse::streams)
  .field("nb_chapters", &FileInfoResponse::nb_chapters)
  .field("chapters", &FileInfoResponse::chapters)
  ;
  function("get_file_info", &get_file_info);
  
  emscripten::value_object<Keyframe>("Keyframe")
  .field("ptsTime", &Keyframe::pts_time)
  .field("ptsTimeString", &Keyframe::pts_time_string)
  ;
  register_vector<Keyframe>("Keyframe");

  emscripten::value_object<VideoInfoResponse>("VideoInfoResponse")
  .field("duration", &VideoInfoResponse::duration)
  .field("width", &VideoInfoResponse::width)
  .field("height", &VideoInfoResponse::height)
  .field("videoCodec", &VideoInfoResponse::videoCodec)
  .field("audioCodec", &VideoInfoResponse::audioCodec)
  .field("keyframes", &VideoInfoResponse::keyframes)
  ;

  function("getVideoInfo", &getVideoInfo);

  function("trimingWebM", &trimingWebM);
}