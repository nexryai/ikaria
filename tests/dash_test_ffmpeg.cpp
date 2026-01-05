#include "gtest/gtest.h"
#include "../src/DashRemuxer.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <filesystem>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

struct DashTestParam {
    std::string url;
    int video_tolerance;
    int audio_tolerance;
    double allowed_pts_diff;
};

class DashRemuxerFFmpegTest : public ::testing::TestWithParam<DashTestParam> {
protected:
    static void TearDownTestSuite() { cleanup_test_files(); }
};

int runFfmpegSafe(const std::string& url, const std::string& targetExt, const std::string& ffOutputPath) {
    const char* ffmpeg_env = std::getenv("FFMPEG_PATH");
    if (!ffmpeg_env) {
        std::cerr << "Error: FFMPEG_PATH environment variable not set" << std::endl;
        return -1;
    }
    std::string ffmpeg_path = std::string(ffmpeg_env);

    pid_t pid = fork();

    if (pid == 0) {
        int devNull = open("/dev/null", O_WRONLY);
        dup2(devNull, STDOUT_FILENO);
        dup2(devNull, STDERR_FILENO);
        close(devNull);

        std::string initSeg = "real_ffmpeg_init_$RepresentationID$." + targetExt;
        std::string mediaSeg = "real_ffmpeg_chunk_$RepresentationID$_$Number$." + targetExt;

        std::vector<const char*> args = {
            ffmpeg_path.c_str(), "-y", "-i", url.c_str(),
            "-c", "copy", "-f", "dash",
            "-seg_duration", "4", "-window_size", "0",
            "-init_seg_name", initSeg.c_str(),
            "-media_seg_name", mediaSeg.c_str(),
            ffOutputPath.c_str(),
            nullptr
        };

        execvp(ffmpeg_path.c_str(), const_cast<char* const*>(args.data()));
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

    const std::string inputLocalPath = download_test_data(param.url);
    const size_t last_slash_idx = param.url.find_last_of('/');
    const std::string raw_filename = param.url.substr(last_slash_idx + 1);
    const std::string myOutputPath = raw_filename + ".mpd";
    const std::string ffOutputPath = "real_ffmpeg_out.mpd";

    DashRemuxer remuxer;
    remuxer.process(inputLocalPath, myOutputPath);
    auto myData = get_stream_data(myOutputPath);

    std::string format = param.url.substr(param.url.find_last_of('.') + 1);
    std::string targetExt = (format == "mov" || format == "mkv") ? "mp4" : format;

    ASSERT_EQ(runFfmpegSafe(param.url, targetExt, ffOutputPath), 0);

    // Check both mpd files are created successfully
    ASSERT_TRUE(std::filesystem::exists(myOutputPath)) << "Ikaria MPD file was not created: " << myOutputPath;
    ASSERT_TRUE(std::filesystem::exists(ffOutputPath)) << "ffmpeg MPD file was not created: " << myOutputPath;

    if (myOutputPath == ffOutputPath) {
        throw std::runtime_error("Invalid filename");
    }

    // Check initial segment existence
    std::string ffInitSeg = "real_ffmpeg_init_0." + targetExt;
    ASSERT_TRUE(std::filesystem::exists(ffInitSeg));

    std::string myInitSeg = "init_0." + targetExt;
    ASSERT_TRUE(std::filesystem::exists(myInitSeg));

    auto ffData = get_stream_data(ffOutputPath);

    verify_stream(ffData.video, myData.video, "Comparison Video", 0, param.allowed_pts_diff);
    verify_stream(ffData.audio, myData.audio, "Comparison Audio", 0, param.allowed_pts_diff);
}

INSTANTIATE_TEST_SUITE_P(FFmpegComparison, DashRemuxerFFmpegTest, ::testing::Values(
    DashTestParam{"https://itdr2.nexryai.me/big-buck-bunny_trailer_h264.mov", 0, 0, 0},
    DashTestParam{"https://itdr2.nexryai.me/big-buck-bunny_trailer_vp8.webm", 0, 0, 0},
    DashTestParam{"https://itdr2.nexryai.me/Tears of Steel in 4k - Official Blender Foundation release [OHOpb2fS-cM] (mp4a).mkv", 0, 0, 0.001},
    DashTestParam{"https://itdr2.nexryai.me/Tears of Steel in 4k - Official Blender Foundation release [OHOpb2fS-cM] (opus).webm", 0, 0, 0},
    DashTestParam{"https://itdr2.nexryai.me/Tears of Steel in 4k - Official Blender Foundation release [OHOpb2fS-cM] (av1).webm", 0, 0, 0},
    DashTestParam{"https://itdr2.nexryai.me/No Copyright, Copyright Free Videos, sunset, beach, sea, waves [Eoo4HzILB-M].mp4", 0, 0, 0}
));
