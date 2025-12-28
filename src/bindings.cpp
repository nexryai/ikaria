#include <emscripten/bind.h>
#include "DashRemuxer.hpp"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(dash_remuxer_module) {
    class_<DashRemuxer>("DashRemuxer")
        .constructor<>()
        .function("process", &DashRemuxer::process);
}
