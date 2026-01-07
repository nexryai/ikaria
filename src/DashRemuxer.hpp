#pragma once
#include <string>

class DashRemuxer {
public:
    DashRemuxer() = default;
    int process(std::string inputPath, std::string outputPath, bool use_opfs = false);

private:
    void init_opfs();
    void ensure_directory(const std::string& filepath);
};
