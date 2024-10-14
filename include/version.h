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
    static constexpr const version_t program_version = { 0, 1, 0 };

#if defined(__clang__)
    static constexpr const char *program_compiler = "Clang";
    static constexpr const version_t program_compiler_version = {
        __clang_major__, __clang_minor__, __clang_patchlevel__
    };
#elif defined(__GNUC__)
    static constexpr const char *program_compiler = "GCC";
    static constexpr const version_t program_compiler_version = {
        __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
    };
#elif defined(_MSC_VER)
    static constexpr const char *program_compiler = "Microsoft C/C++ Compiler";
    static constexpr const version_t program_compiler_version = {
        _MSC_VER / 100,
        _MSC_VER % 100,
#if defined(_MSC_FULL_VER)
        _MSC_FULL_VER % 100000
#else
        0
#endif
    };
#else
    static constexpr const char *program_compiler = "Unknown Compiler";
    static constexpr const version_t program_compiler_version = { 0, 0, 0 };
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
