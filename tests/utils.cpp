#include "utils.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <set>
#include "gtest/gtest.h"
extern "C" {
#include <libavutil/crc.h>
#include <libavutil/avutil.h>
}

StreamData get_stream_data(const std::string& path) {
    StreamData results;
    AVFormatContext* fmt_raw = nullptr;
    if (avformat_open_input(&fmt_raw, path.c_str(), nullptr, nullptr) < 0) return results;
    FormatContextPtr fmt_ctx(fmt_raw);
    if (avformat_find_stream_info(fmt_ctx.get(), nullptr) < 0) return results;

    const AVCRC* crc_table = av_crc_get_table(AV_CRC_32_IEEE);
    PacketPtr pkt(av_packet_alloc());

    while (av_read_frame(fmt_ctx.get(), pkt.get()) >= 0) {
        AVStream* stream = fmt_ctx->streams[pkt->stream_index];
        AVMediaType type = stream->codecpar->codec_type;

        PacketInfo info;
        info.crc = av_crc(crc_table, 0, pkt->data, pkt->size);
        info.pts_sec = pkt->pts * av_q2d(stream->time_base);

        if (type == AVMEDIA_TYPE_VIDEO) {
            results.video.push_back(info);
        } else if (type == AVMEDIA_TYPE_AUDIO) {
            results.audio.push_back(info);
        }
        av_packet_unref(pkt.get());
    }
    return results;
}

void verify_stream(const std::vector<PacketInfo>& orig, const std::vector<PacketInfo>& remux, const std::string& label, int tolerance, const double allowed_pts_diff,const std::string& format) {
    // パケット数の確認（わずかな欠損は許容）
    int diff = static_cast<int>(orig.size()) - static_cast<int>(remux.size());
    std::cout << "[info] " << label << " PKTS DIFF: " << diff << std::endl;
    EXPECT_GE(diff, 0) << label << ": There are more packets after remuxing.";
    EXPECT_LE(diff, tolerance) << label << ": Too many packets lost (number of lost packets: " << diff << ")";

    // 先頭からパケットのCRCが一致するかを確認（末尾が合わないのは許容）
    size_t common_count = std::min(orig.size(), remux.size());
    for (size_t i = 0; i < common_count; ++i) {
        EXPECT_EQ(orig[i].crc, remux[i].crc) << label << ": CRC mismatch at INDEX " << i;
    }

    // 再生時間が維持されているか
    if (!orig.empty() && !remux.empty()) {
        double last_pts_orig = orig.back().pts_sec;
        double last_pts_remux = remux.back().pts_sec;
        double pts_diff = std::abs(last_pts_orig - last_pts_remux);

        // わずかなズレなら許容（movの場合mp4への変換を挟むので大きくなる？）
        EXPECT_LE(pts_diff, allowed_pts_diff) << label << ": The difference in PTS is too large (difference: " << pts_diff << "s)";

        std::cout << "[Info] " << label << " PTS DIFF: " << pts_diff << "s (Original: " << last_pts_orig << ", Remuxed: " << last_pts_remux << ")" << std::endl;
    }
}

void cleanup_test_files() {
    namespace fs = std::filesystem;
    const std::set<std::string> targets = {".webm", ".mov", ".mp4", ".mpd", ".m4s"};
    try {
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_regular_file() && targets.count(entry.path().extension().string())) {
                fs::remove(entry.path());
            }
        }
    } catch (...) {}
}
