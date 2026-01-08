#include "DashRemuxer.hpp"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#include <emscripten/wasmfs.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(dash_remuxer_module) {
    class_<DashRemuxer>("DashRemuxer")
        .constructor<>()
        .function("process", &DashRemuxer::process, async());
}

int main() {
    backend_t opfs = wasmfs_create_opfs_backend();
    std::cout << "[ikaria] binding.cpp: wasmfs_create_opfs_backend() --> OK" << std::endl;
    wasmfs_create_directory("/opfs", 0755, opfs);
    std::cout << "[ikaria] binding.cpp: wasmfs_create_directory() --> OK" << std::endl;
}

#endif
