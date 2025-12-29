#include "gtest/gtest.h"
#include "../src/DashRemuxer.hpp"
#include <cstdlib>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <set>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/crc.h>
}

struct PacketInfo {
    uint32_t crc;
    double pts_sec;
};

struct StreamData {
    std::vector<PacketInfo> video;
    std::vector<PacketInfo> audio;
};

struct TestAVDeleter {
    void operator()(AVFormatContext* p) const { if (p) { if (p->iformat) avformat_close_input(&p); else avformat_free_context(p); } }
    void operator()(AVPacket* p) const { if (p) av_packet_free(&p); }
};

using FormatContextPtr = std::unique_ptr<AVFormatContext, TestAVDeleter>;
using PacketPtr = std::unique_ptr<AVPacket, TestAVDeleter>;

// パケット情報（CRCとPTS）を取得する
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

struct DashTestParam {
    std::string filename;
    int video_tolerance;
    int audio_tolerance;
};

class DashRemuxerTest : public ::testing::TestWithParam<DashTestParam> {
protected:
    // 終了時にクリーンアップを実行する
    static void TearDownTestSuite() {
        namespace fs = std::filesystem;
        const std::set<std::string> targets = {".webm", ".mov", ".mp4", ".mpd"};

        try {
            for (const auto& entry : fs::directory_iterator(".")) {
                if (entry.is_regular_file() && targets.count(entry.path().extension().string())) {
                    fs::remove(entry.path());
                }
            }
        } catch (...) {
            // クリーンアップ中のエラーはテスト結果に影響させない
        }
    }
};

TEST_P(DashRemuxerTest, FlexibleRemuxTest) {
    const auto& param = GetParam();

    const char* srcdir = std::getenv("TEST_SRCDIR");
    const char* workspace = std::getenv("TEST_WORKSPACE");
    ASSERT_TRUE(srcdir && workspace) << "Bazel環境変数が取得できません。";

    std::string inputPath = std::string(srcdir) + "/" + workspace + "/tests/test_data/" + param.filename;
    std::string outputPath = "output_" + param.filename + ".mpd";

    auto original = get_stream_data(inputPath);
    ASSERT_FALSE(original.video.empty()) << "入力ビデオが空です。";

    DashRemuxer remuxer;
    ASSERT_NO_THROW(remuxer.process(inputPath, outputPath));

    auto remuxed = get_stream_data(outputPath);

    auto verify_stream = [](const std::vector<PacketInfo>& orig, const std::vector<PacketInfo>& remux, const std::string& label, int tolerance) {
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

            // 0.1秒までのズレなら許容
            EXPECT_LT(pts_diff, 0.1) << label << ": The difference in PTS is too large (difference: " << pts_diff << "s)";

            std::cout << "[Info] " << label << " PTS DIFF: " << pts_diff << "s (Original: " << last_pts_orig << ", Remuxed: " << last_pts_remux << ")" << std::endl;
        }
    };

    verify_stream(original.video, remuxed.video, "Video", param.video_tolerance);
    verify_stream(original.audio, remuxed.audio, "Audio", param.audio_tolerance);
}

INSTANTIATE_TEST_SUITE_P(
    FileVariations,
    DashRemuxerTest,
    ::testing::Values(
        DashTestParam{"20250916211744.webm", 5, 10},
        DashTestParam{"big-buck-bunny_trailer.webm", 5, 10}
    ),
    [](const ::testing::TestParamInfo<DashRemuxerTest::ParamType>& info) {
        std::string name = info.param.filename;
        std::replace_if(name.begin(), name.end(), [](char c) { return !std::isalnum(c); }, '_');
        return name;
    }
);
