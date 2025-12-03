extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
}

#include <emscripten/bind.h>
#include <emscripten/wasmfs.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace emscripten;

namespace fs = std::filesystem;

int create_dir(const std::string path) {
    fs::path p(path);
    fs::path dir = p.parent_path();

    std::error_code err;

    if (fs::create_directories(dir, err)) {
        std::cout << "Directory created: " << dir << std::endl;
    } else {
        if (err) {
            std::cerr << "Failed to create directory: " << err.message() << std::endl;
            return -1;
        } else {
            std::cout << "Directory already exists: " << dir << std::endl;
        }
    }

    return 0;
}

void check_error(int ret, const std::string& msg) {
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
        std::cerr << "Error: " << msg << " (" << errbuf << ")" << std::endl;
        throw std::runtime_error(msg);
    }
}

int init_opfs() {
    backend_t opfs = wasmfs_create_opfs_backend();
    std::cout << "created OPFS backend\n";

    auto err = wasmfs_create_directory("/opfs", 0755, opfs);
    std::cout << "mounted OPFS root directory with error code " << err << "\n";

    return err;
}

void remuxToDash(std::string inputPath, std::string outputPath) {
    const std::string outputPathOPFS = "/opfs" + outputPath;

    AVFormatContext* ifmt_ctx = nullptr;
    AVFormatContext* ofmt_ctx = nullptr;
    AVDictionary* opts = nullptr;
    int ret;

    if ((ret = init_opfs()) < 0) {
        throw std::runtime_error("Failed to initialize OPFS");
    }

    if ((ret = create_dir(outputPathOPFS) < 0)) {
        throw std::runtime_error("Failed to create dir");
    }

    if ((ret = avformat_open_input(&ifmt_ctx, inputPath.c_str(), 0, 0)) < 0) {
        check_error(ret, "Could not open input file");
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        avformat_close_input(&ifmt_ctx);
        check_error(ret, "Failed to retrieve input stream information");
    }

    avformat_alloc_output_context2(&ofmt_ctx, nullptr, "dash", outputPathOPFS.c_str());
    if (!ofmt_ctx) {
        avformat_close_input(&ifmt_ctx);
        throw std::runtime_error("Could not create output context");
    }

    for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream* in_stream = ifmt_ctx->streams[i];
        AVStream* out_stream = avformat_new_stream(ofmt_ctx, nullptr);
        if (!out_stream) {
            throw std::runtime_error("Failed allocating output stream");
        }

        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        check_error(ret, "Failed to copy codec parameters");

        out_stream->codecpar->codec_tag = 0;
    }

    // -window_size 0, -seg_duration 4, -init_seg_name, -media_seg_name
    av_dict_set(&opts, "window_size", "0", 0);
    av_dict_set(&opts, "seg_duration", "4", 0);
    av_dict_set(&opts, "init_seg_name", "init_$RepresentationID$.webm", 0);
    av_dict_set(&opts, "media_seg_name", "chunk_$RepresentationID$_$Number$.webm", 0);
    // av_dict_set(&opts, "format", "webm", 0);

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, outputPath.c_str(), AVIO_FLAG_WRITE);
        check_error(ret, "Could not open output file");
    }

    // ヘッダー書き込み
    ret = avformat_write_header(ofmt_ctx, &opts);
    check_error(ret, "Error occurred when writing header");

    // Remux処理
    AVPacket pkt;
    while (true) {
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) break; // EOF or error

        AVStream* in_stream = ifmt_ctx->streams[pkt.stream_index];
        AVStream* out_stream = ofmt_ctx->streams[pkt.stream_index];

        // Rescale
        av_packet_rescale_ts(&pkt, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        av_packet_unref(&pkt);
        if (ret < 0) {
            std::cerr << "Error muxing packet" << std::endl;
            break;
        }
    }

    // トレーラー書き込み
    av_write_trailer(ofmt_ctx);

    // Clean up
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);

    avformat_free_context(ofmt_ctx);
    avformat_close_input(&ifmt_ctx);
    if (opts) av_dict_free(&opts);

    std::cout << "Conversion completed successfully." << std::endl;
}

// Emscripten Binding
EMSCRIPTEN_BINDINGS(my_module) {
    function("remuxToDash", &remuxToDash);
}
