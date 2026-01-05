#include "gtest/gtest.h"
#include <filesystem>
#include "../src/DashRemuxer.hpp"
#include "utils.hpp"

struct DashTestParam {
    std::string url;
    int video_tolerance;
    int audio_tolerance;
    double allowed_pts_diff;
};

class DashRemuxerTest : public ::testing::TestWithParam<DashTestParam> {
protected:
    static void TearDownTestSuite() { cleanup_test_files(); }
};

TEST_P(DashRemuxerTest, FlexibleRemuxTest) {
    const auto& param = GetParam();

    const std::string inputPath = download_test_data(param.url);
    const std::string outputPath = "output_" + std::filesystem::path(inputPath).filename().string() + ".mpd";

    auto original = get_stream_data(inputPath);
    ASSERT_FALSE(original.video.empty());

    DashRemuxer remuxer;
    ASSERT_NO_THROW(remuxer.process(inputPath, outputPath));

    auto remuxed = get_stream_data(outputPath);
    verify_stream(original.video, remuxed.video, "Video", param.video_tolerance, param.allowed_pts_diff, param.url.substr(param.url.find_last_of('.') + 1));
    verify_stream(original.audio, remuxed.audio, "Audio", param.audio_tolerance, param.allowed_pts_diff);
}

INSTANTIATE_TEST_SUITE_P(FileVariations, DashRemuxerTest, ::testing::Values(
    DashTestParam{"https://itdr2.nexryai.me/big-buck-bunny_trailer_h264.mov", 5, 30, 0.8},
    DashTestParam{"https://itdr2.nexryai.me/big-buck-bunny_trailer_vp8.webm", 0, 4, 0.1},
    DashTestParam{"https://itdr2.nexryai.me/Tears of Steel in 4k - Official Blender Foundation release [OHOpb2fS-cM] (mp4a).mkv", 0, 0, 1e-5},
    DashTestParam{"https://itdr2.nexryai.me/Tears of Steel in 4k - Official Blender Foundation release [OHOpb2fS-cM] (opus).webm", 0, 2, 0.05},
    DashTestParam{"https://itdr2.nexryai.me/Tears of Steel in 4k - Official Blender Foundation release [OHOpb2fS-cM] (av1).webm", 0, 2, 0.1},
    DashTestParam{"https://itdr2.nexryai.me/No Copyright, Copyright Free Videos, sunset, beach, sea, waves [Eoo4HzILB-M].mp4", 0, 0, 0}
));
