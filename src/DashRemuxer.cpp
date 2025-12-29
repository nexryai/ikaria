#include "DashRemuxer.hpp"
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
}

#include <filesystem>
#include <memory>
#include <stdexcept>

#ifdef __EMSCRIPTEN__
#include <emscripten/wasmfs.h>
#endif

namespace fs = std::filesystem;

struct AVDeleter {
    void operator()(AVFormatContext* p) const { if (p) { if (p->iformat) avformat_close_input(&p); else avformat_free_context(p); } }
    void operator()(AVDictionary* p) const { if (p) av_dict_free(&p); }
    void operator()(AVPacket* p) const { if (p) av_packet_free(&p); }
};

using FormatContextPtr = std::unique_ptr<AVFormatContext, AVDeleter>;
using DictionaryPtr = std::unique_ptr<AVDictionary, AVDeleter>;
using PacketPtr = std::unique_ptr<AVPacket, AVDeleter>;

void DashRemuxer::ensure_directory(const std::string& filepath) {
    fs::path p(filepath);
    if (p.has_parent_path()) fs::create_directories(p.parent_path());
}

void DashRemuxer::init_opfs() {
    static bool initialized = false;
    if (!initialized) {
        #ifdef __EMSCRIPTEN__
        backend_t opfs = wasmfs_create_opfs_backend();
        wasmfs_create_directory("/opfs", 0755, opfs);
        initialized = true;
        #endif
    }
}

void DashRemuxer::process(std::string inputPath, std::string outputPath, bool use_opfs) {
    const std::string format = inputPath.substr(inputPath.find_last_of('.') + 1);
    const std::string targetFormat = format == "mov" || format == "mkv" ? "mp4" : format;

    std::string finalOutputPath = outputPath;
    if (use_opfs) {
        init_opfs();
        finalOutputPath = "/opfs" + outputPath;
    }

    ensure_directory(finalOutputPath);

    AVFormatContext* ifmt_raw = nullptr;
    int ret = avformat_open_input(&ifmt_raw, inputPath.c_str(), nullptr, nullptr);
    if (ret < 0) {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));

        std::string msg = "Could not open input: " + inputPath + " (Error: " + errbuf + ", Code: " + std::to_string(ret) + ")";
        throw std::runtime_error(msg);
    }

    FormatContextPtr ifmt_ctx(ifmt_raw);

    if (avformat_find_stream_info(ifmt_ctx.get(), nullptr) < 0) {
        throw std::runtime_error("Failed to retrieve stream info");
    }

    AVFormatContext* ofmt_raw = nullptr;
    avformat_alloc_output_context2(&ofmt_raw, nullptr, "dash", finalOutputPath.c_str());
    if (!ofmt_raw) {
        throw std::runtime_error("Could not create output context");
    }

    FormatContextPtr ofmt_ctx(ofmt_raw);

    for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *in_s = ifmt_ctx->streams[i], *out_s = avformat_new_stream(ofmt_ctx.get(), nullptr);
        avcodec_parameters_copy(out_s->codecpar, in_s->codecpar);
        out_s->codecpar->codec_tag = 0;
    }

    AVDictionary* opts_raw = nullptr;
    av_dict_set(&opts_raw, "window_size", "0", 0);
    av_dict_set(&opts_raw, "seg_duration", "4", 0);
    av_dict_set(&opts_raw, "init_seg_name", ("init_$RepresentationID$." + targetFormat).c_str(), 0);
    av_dict_set(&opts_raw, "media_seg_name", ("chunk_$RepresentationID$_$Number$." + targetFormat).c_str(), 0);
    DictionaryPtr opts(opts_raw);

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx->pb, finalOutputPath.c_str(), AVIO_FLAG_WRITE) < 0) {
            throw std::runtime_error("Could not open output file");
        }
    }

    AVDictionary* tmp_opts = opts.release();
    if (avformat_write_header(ofmt_ctx.get(), &tmp_opts) < 0) {
        av_dict_free(&tmp_opts);
        throw std::runtime_error("Error writing header");
    }

    PacketPtr pkt(av_packet_alloc());
    while (av_read_frame(ifmt_ctx.get(), pkt.get()) >= 0) {
        AVStream *in_s = ifmt_ctx->streams[pkt->stream_index], *out_s = ofmt_ctx->streams[pkt->stream_index];
        av_packet_rescale_ts(pkt.get(), in_s->time_base, out_s->time_base);
        pkt->pos = -1;
        av_interleaved_write_frame(ofmt_ctx.get(), pkt.get());
        av_packet_unref(pkt.get());
    }

    av_write_trailer(ofmt_ctx.get());
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&ofmt_ctx->pb);
    }
}
