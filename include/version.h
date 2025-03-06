#if !defined(EXPRPARSER_VERSION_HEADER)
#define EXPRPARSER_VERSION_HEADER

#include <cstdint>          // std::uint16_t
#include <iosfwd>           // std::ostream

namespace expr {
    struct version_t {
        std::uint16_t major;
        std::uint16_t minor;
        std::uint16_t revision;
    };

    static constexpr const char *program_name = "exprparser";
    static constexpr const version_t program_version = {
        .major = 0,
        .minor = 3,
        .revision = 0
    };

#if defined(__INTEL_COMPILER)
    static constexpr const char *program_compiler = "ICC";
    static constexpr const version_t program_compiler_version = {
#if __INTEL_COMPILER > 2000
        .major = __INTEL_COMPILER,
#else
        .major = __INTEL_COMPILER / 100,
#endif
        .minor = __INTEL_COMPILER_UPDATE,
        .revision = 0
    };
#elif defined(__clang__)
#if defined(__apple_build_version__)
    static constexpr const char *program_compiler = "Apple Clang";
#else
    static constexpr const char *program_compiler = "LLVM Clang";
#endif
    static constexpr const version_t program_compiler_version = {
        .major = __clang_major__,
        .minor = __clang_minor__,
        .revision = __clang_patchlevel__
    };
#elif defined(__GNUC__)
    static constexpr const char *program_compiler = "GCC";
    static constexpr const version_t program_compiler_version = {
        .major = __GNUC__,
        .minor = __GNUC_MINOR__,
        .revision = __GNUC_PATCHLEVEL__
    };
#elif defined(_MSC_VER)
    static constexpr const char *program_compiler = "Microsoft C/C++ Compiler";
    static constexpr const version_t program_compiler_version = {
        .major = _MSC_VER / 100,
        .minor = _MSC_VER % 100,
#if defined(_MSC_FULL_VER)
        .revision = _MSC_FULL_VER % 100000
#else
        .revision = 0
#endif
    };
#else
    static constexpr const char *program_compiler = "Unknown Compiler";
    static constexpr const version_t program_compiler_version = {
        .major = 0,
        .minor = 0,
        .revision = 0
    };
#endif

#if defined(_WIN32)
    static constexpr const char *program_platform = "Windows";
#elif defined(__linux__)
    static constexpr const char *program_platform = "Linux";
#elif defined(__APPLE__)
    static constexpr const char *program_platform = "Apple";
#else
    static constexpr const char *program_platform = "Unknown";
#endif

}

std::ostream& operator<<(std::ostream& stream, const expr::version_t& version);

#endif
