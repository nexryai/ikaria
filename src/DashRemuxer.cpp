#include "runtime.hpp"
#include <iostream>
#include "DashRemuxer.hpp"
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
}

#include <filesystem>

#ifdef __EMSCRIPTEN__
#include <emscripten/wasmfs.h>
#endif

namespace fs = std::filesystem;

void DashRemuxer::ensure_directory(const std::string& filepath) {
    fs::path p(filepath);
    if (p.has_parent_path()) fs::create_directories(p.parent_path());
}

void DashRemuxer::init_opfs() {
    static bool initialized = false;
    if (!initialized) {
        #ifdef __EMSCRIPTEN__
        backend_t opfs = wasmfs_create_opfs_backend();
        std::cout << "[ikaria] wasmfs_create_opfs_backend() --> OK" << std::endl;
        wasmfs_create_directory("/opfs", 0755, opfs);
        std::cout << "[ikaria] wasmfs_create_directory() --> OK" << std::endl;
        initialized = true;
        #endif
    }
}

int DashRemuxer::process(std::string inputPath, std::string outputPath, bool use_opfs) {
    std::cout << "[ikaria] called process" << std::endl;

    const std::string format = inputPath.substr(inputPath.find_last_of('.') + 1);
    const std::string targetFormat = format == "mov" || format == "mkv" ? "mp4" : format;

    std::string finalOutputPath = outputPath;
    if (use_opfs) {
        init_opfs();
        finalOutputPath = "/opfs" + outputPath;
    }

    ensure_directory(finalOutputPath);
    std::cout << "[ikaria] ensure_directory() --> OK" << std::endl;

    AVFormatContext* ifmt_raw = nullptr;
    int ret = avformat_open_input(&ifmt_raw, inputPath.c_str(), nullptr, nullptr);
    if (ret < 0) {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        std::cerr << "[ikaria] Could not open input: " << inputPath << " (Error: " << errbuf << ", Code: " << ret << ")" << std::endl;
        return -1;
    }

    FormatContextPtr ifmt_ctx(ifmt_raw);

    if (avformat_find_stream_info(ifmt_ctx.get(), nullptr) < 0) {
        std::cerr << "[ikaria] Failed to retrieve stream info" << std::endl;
        return -1;
    }

    AVFormatContext* ofmt_raw = nullptr;
    avformat_alloc_output_context2(&ofmt_raw, nullptr, "dash", finalOutputPath.c_str());
    if (!ofmt_raw) {
        std::cerr << "[ikaria] Could not create output context" << std::endl;
        return -1;
    }

    FormatContextPtr ofmt_ctx(ofmt_raw);

    for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *in_s = ifmt_ctx->streams[i];
        AVStream *out_s = avformat_new_stream(ofmt_ctx.get(), nullptr);
        if (!out_s) {
            std::cerr << "[ikaria] Failed to allocate output stream" << std::endl;
            return -1;
        }
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
            std::cerr << "[ikaria] Could not open output file" << std::endl;
            return -1;
        }
    }

    AVDictionary* tmp_opts = opts.release();
    if (avformat_write_header(ofmt_ctx.get(), &tmp_opts) < 0) {
        av_dict_free(&tmp_opts);
        std::cerr << "[ikaria] Error writing header" << std::endl;
        return -1;
    }

    std::cout << "[ikaria] avformat_write_header() --> OK" << std::endl;

    while (true) {
        PacketPtr pkt(av_packet_alloc());
        if (!pkt) {
            std::cerr << "[ikaria] Could not allocate packet" << std::endl;
            return -1;
        }

        if (av_read_frame(ifmt_ctx.get(), pkt.get()) < 0) {
            break;
        }

        AVStream *in_s = ifmt_ctx->streams[pkt->stream_index];
        AVStream *out_s = ofmt_ctx->streams[pkt->stream_index];

        av_packet_rescale_ts(pkt.get(), in_s->time_base, out_s->time_base);
        pkt->pos = -1;

        if (av_interleaved_write_frame(ofmt_ctx.get(), pkt.get()) < 0) {
            std::cerr << "[ikaria] Error while muxing packet" << std::endl;
            return -1;
        }
    }

    av_write_trailer(ofmt_ctx.get());

    return 0;
}
