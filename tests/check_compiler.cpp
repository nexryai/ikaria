#include <gtest/gtest.h>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>

TEST(CompilerCheck, IsGCC) {
#ifdef defined(__GNUC__)
    FAIL() << "GCC is not allowed for testing.";
#else
    // Emscripten uses clang, so we need to use the same compiler for unit tests.
    SUCCEED();
#endif
}

TEST(CompilerCheck, IsValidCompiler) {
    std::cout << "[info] Compiler Version String: " << __VERSION__ << std::endl;
#ifdef __clang__
    std::string compiler = __VERSION__;
    std::transform(compiler.begin(), compiler.end(), compiler.begin(), ::tolower);

    EXPECT_EQ(compiler.find("clang") == std::string::npos, false);

    EXPECT_GE(__clang_major__, 18) << "Clang version is too old. Required: 18+, Found: " << __clang_major__;
#else
    FAIL() << "This compiler is not allowed.";
#endif
}
