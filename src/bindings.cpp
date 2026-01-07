#include "DashRemuxer.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(dash_remuxer_module) {
    class_<DashRemuxer>("DashRemuxer")
        .constructor<>()
        .function("process", &DashRemuxer::process, async());
}
#endif
