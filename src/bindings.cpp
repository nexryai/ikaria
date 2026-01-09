#include "DashRemuxer.hpp"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#include <emscripten/wasmfs.h>

using namespace emscripten;

int remuxToDash(std::string inputPath, std::string outputPath) {
    DashRemuxer remuxer;
    return remuxer.process(inputPath, outputPath, true);
}

EMSCRIPTEN_BINDINGS(dash_remuxer_module) {
    function("remuxToDash", &remuxToDash, async());
}

int main() {
    backend_t opfs = wasmfs_create_opfs_backend();
    std::cout << "[ikaria] binding.cpp: wasmfs_create_opfs_backend() --> OK" << std::endl;
    wasmfs_create_directory("/opfs", 0755, opfs);
    std::cout << "[ikaria] binding.cpp: wasmfs_create_directory() --> OK" << std::endl;
    return 0;
}
#endif
