#pragma once
#include <string>
#include <vector>
#include <memory>

extern "C" {
#include <libavformat/avformat.h>
}

struct PacketInfo {
    uint32_t crc;
    double pts_sec;
};

struct StreamData {
    std::vector<PacketInfo> video;
    std::vector<PacketInfo> audio;
};

struct AVDeleter {
    void operator()(AVFormatContext* p) const { if (p) { if (p->iformat) avformat_close_input(&p); else avformat_free_context(p); } }
    void operator()(AVPacket* p) const { if (p) av_packet_free(&p); }
};

using FormatContextPtr = std::unique_ptr<AVFormatContext, AVDeleter>;
using PacketPtr = std::unique_ptr<AVPacket, AVDeleter>;

StreamData get_stream_data(const std::string& path);
void verify_stream(const std::vector<PacketInfo>& orig, const std::vector<PacketInfo>& remux, const std::string& label, int tolerance, const double allowed_pts_diff, const std::string& format = "");
void cleanup_test_files();

std::string download_test_data(const std::string& url);
