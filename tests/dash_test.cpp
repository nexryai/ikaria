#include "gtest/gtest.h"
#include "../src/DashRemuxer.hpp"
#include "utils.hpp"

struct DashTestParam {
    std::string filename;
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
    const char* srcdir = std::getenv("TEST_SRCDIR");
    const char* workspace = std::getenv("TEST_WORKSPACE");
    ASSERT_TRUE(srcdir && workspace);

    const std::string inputPath = std::string(srcdir) + "/" + workspace + "/tests/test_data/" + param.filename;
    const std::string outputPath = "output_" + param.filename + ".mpd";

    auto original = get_stream_data(inputPath);
    ASSERT_FALSE(original.video.empty());

    DashRemuxer remuxer;
    ASSERT_NO_THROW(remuxer.process(inputPath, outputPath));

    auto remuxed = get_stream_data(outputPath);
    verify_stream(original.video, remuxed.video, "Video", param.video_tolerance, param.allowed_pts_diff, param.filename.substr(param.filename.find_last_of('.') + 1));
    verify_stream(original.audio, remuxed.audio, "Audio", param.audio_tolerance, param.allowed_pts_diff);
}

INSTANTIATE_TEST_SUITE_P(FileVariations, DashRemuxerTest, ::testing::Values(
    DashTestParam{"big-buck-bunny_trailer_h264.mov", 5, 30, 0.8},
    DashTestParam{"big-buck-bunny_trailer_vp8.webm", 0, 4, 0.1},
    DashTestParam{"Tears of Steel in 4k - Official Blender Foundation release [OHOpb2fS-cM] (mp4a).mkv", 0, 0, 1e-5},
    DashTestParam{"Tears of Steel in 4k - Official Blender Foundation release [OHOpb2fS-cM] (opus).webm", 0, 2, 0.05},
    DashTestParam{"Tears of Steel in 4k - Official Blender Foundation release [OHOpb2fS-cM] (av1).webm", 0, 2, 0.1},
    DashTestParam{"No Copyright, Copyright Free Videos, sunset, beach, sea, waves [Eoo4HzILB-M].mp4", 0, 0, 0}
));
