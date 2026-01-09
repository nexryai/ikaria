#include "DashRemuxer.hpp"
#include <iostream>
#include <ostream>

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#include <emscripten/wasmfs.h>

using namespace emscripten;

int remuxToDash(std::string inputPath, std::string outputPath) {
    DashRemuxer remuxer;
    return remuxer.process(inputPath, outputPath, true);
}

int main(const int argc, const char** argv) {
    if(argc > 4) {
        std::cerr << "Invalid args" << std::endl;
        return -1;
    }

    const std::string command = argv[1];
    const std::string inputPath = argv[2];
    const std::string outputPath = argv[3];

    std::cout << "inputPath: " << inputPath << std::endl;
    std::cout << "outputPath: " << outputPath << std::endl;

    backend_t opfs = wasmfs_create_opfs_backend();
    std::cout << "[ikaria] binding.cpp: wasmfs_create_opfs_backend() --> OK" << std::endl;
    wasmfs_create_directory("/opfs", 0755, opfs);
    std::cout << "[ikaria] binding.cpp: wasmfs_create_directory() --> OK" << std::endl;

    int result = 0;
    if (command == "remuxToDash") {
        result = remuxToDash(inputPath, outputPath);
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        result = -1;
    }

    return result;
}
#endif
