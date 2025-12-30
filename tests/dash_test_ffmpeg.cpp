#include "gtest/gtest.h"
#include "../src/DashRemuxer.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

struct DashTestParam {
    std::string filename;
    int video_tolerance;
    int audio_tolerance;
    double allowed_pts_diff;
};

class DashRemuxerFFmpegTest : public ::testing::TestWithParam<DashTestParam> {
protected:
    static void TearDownTestSuite() { cleanup_test_files(); }
};

int runFfmpegSafe(const std::string& inputPath, const std::string& targetExt, const std::string& ffOutputPath) {
    pid_t pid = fork();

    if (pid == 0) {
        int devNull = open("/dev/null", O_WRONLY);
        dup2(devNull, STDOUT_FILENO);
        dup2(devNull, STDERR_FILENO);
        close(devNull);

        std::string initSeg = "init_$RepresentationID$." + targetExt;
        std::string mediaSeg = "chunk_$RepresentationID$_$Number$." + targetExt;

        std::vector<const char*> args = {
            "ffmpeg", "-y", "-i", inputPath.c_str(),
            "-c", "copy", "-f", "dash",
            "-seg_duration", "4", "-window_size", "0",
            "-init_seg_name", initSeg.c_str(),
            "-media_seg_name", mediaSeg.c_str(),
            ffOutputPath.c_str(),
            nullptr
        };

        execvp("ffmpeg", const_cast<char* const*>(args.data()));
        _exit(1); // execvp が失敗した場合のみ到達
    } else if (pid > 0) { // 親プロセス
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
    return -1;
}

TEST_P(DashRemuxerFFmpegTest, CompareWithFFmpegCommand) {
    const auto& param = GetParam();
    const char* srcdir = std::getenv("TEST_SRCDIR");
    const char* workspace = std::getenv("TEST_WORKSPACE");
    const std::string inputPath = std::string(srcdir) + "/" + workspace + "/tests/test_data/" + param.filename;

    const std::string myOutputPath = "my_ffcomp_" + param.filename + ".mpd";
    const std::string ffOutputPath = "real_ff_" + param.filename + ".mpd";

    DashRemuxer remuxer;
    remuxer.process(inputPath, myOutputPath);
    auto myData = get_stream_data(myOutputPath);

    std::string format = param.filename.substr(param.filename.find_last_of('.') + 1);
    std::string targetExt = (format == "mov" || format == "mkv") ? "mp4" : format;

    ASSERT_EQ(runFfmpegSafe(inputPath, targetExt, ffOutputPath), 0);
    auto ffData = get_stream_data(ffOutputPath);

    verify_stream(ffData.video, myData.video, "Comparison Video", 0, param.allowed_pts_diff);
    verify_stream(ffData.audio, myData.audio, "Comparison Audio", 0, param.allowed_pts_diff);
}

INSTANTIATE_TEST_SUITE_P(FFmpegComparison, DashRemuxerFFmpegTest, ::testing::Values(
    DashTestParam{"big-buck-bunny_trailer_h264.mov", 0, 0, 0},
    DashTestParam{"big-buck-bunny_trailer_vp8.webm", 0, 0, 0},
    DashTestParam{"Tears of Steel in 4k - Official Blender Foundation release [OHOpb2fS-cM] (mp4a).mkv", 0, 0, 0.001},
    DashTestParam{"Tears of Steel in 4k - Official Blender Foundation release [OHOpb2fS-cM] (opus).webm", 0, 0, 0},
    DashTestParam{"Tears of Steel in 4k - Official Blender Foundation release [OHOpb2fS-cM] (av1).webm", 0, 0, 0},
    DashTestParam{"No Copyright, Copyright Free Videos, sunset, beach, sea, waves [Eoo4HzILB-M].mp4", 0, 0, 0}
));
