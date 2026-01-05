#include "utils.hpp"
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <ostream>
#include <stdexcept>
#include <filesystem>
#include <set>
#include "gtest/gtest.h"
#include <curl/curl.h>

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

std::string download_test_data(const std::string& url) {
    static bool curl_initialized = false;
    if (!curl_initialized) {
        curl_global_init(CURL_GLOBAL_ALL);
        curl_initialized = true;
    }

    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialize CURL");

    size_t last_slash_idx = url.find_last_of('/');
    std::string base_url = url.substr(0, last_slash_idx + 1);
    std::string raw_filename = url.substr(last_slash_idx + 1);

    char* encoded_filename_ptr = curl_easy_escape(curl, raw_filename.c_str(), static_cast<int>(raw_filename.length()));
    const std::string encoded_filename = encoded_filename_ptr;
    curl_free(encoded_filename_ptr);

    std::string final_url = base_url + encoded_filename;

    const std::string local_filename = "tmp_" + raw_filename;
    std::ofstream ofs(local_filename, std::ios::binary);

    curl_easy_setopt(curl, CURLOPT_URL, final_url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](void* ptr, size_t size, size_t nmemb, void* stream) -> size_t {
        static_cast<std::ostream*>(stream)->write(static_cast<char*>(ptr), size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ofs);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        ofs.close();
        std::filesystem::remove(local_filename);
        throw std::runtime_error("Download failed for URL: " + final_url + " | Error: " + curl_easy_strerror(res));
    } else {
        std::cout << "File downloaded! : " << local_filename << std::endl;
    }

    return local_filename;
}
