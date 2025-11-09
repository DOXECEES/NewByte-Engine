#ifndef NBCOMMON_NBCORE_HPP
#define NBCOMMON_NBCORE_HPP


#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define NB_NODISCARD [[nodiscard]]
#else
#define NB_NODISCARD
#endif

#if __has_cpp_attribute(noreturn)
#define NB_NORETURN [[noreturn]]
#else
#define NB_NORETURN
#endif

#if __has_cpp_attribute(deprecated)
#define NB_DEPRECATED [[deprecated]]
#else
#define NB_DEPRECATED
#endif

#if __has_cpp_attribute(fallthrough)
#define NB_FALLTHROUGH [[fallthrough]]
#else
#define NB_FALLTHROUGH
#endif

#if __has_cpp_attribute(maybe_unused)
#define NB_MAYBE_UNUSED [[maybe_unused]]
#else
#define NB_MAYBE_UNUSED
#endif

#if __has_cpp_attribute(likely)
#define NB_LIKELY [[likely]]
#else
#define NB_LIKELY
#endif

#if __has_cpp_attribute(unlikely)
#define NB_UNLIKELY [[unlikely]]
#else
#define NB_UNLIKELY
#endif

#else
#define NB_NODISCARD
#define NB_NORETURN
#define NB_DEPRECATED
#define NB_FALLTHROUGH
#define NB_MAYBE_UNUSED
#define NB_LIKELY
#define NB_UNLIKELY
#endif


#if defined(__clang__)
#define NB_FORCEINLINE __attribute__((always_inline))

#elif defined(__GNUC__) || defined(__GNUG__)
#define NB_FORCEINLINE __attribute__((always_inline))

#elif defined(_MSC_VER)
#define NB_FORCEINLINE __forceinline
#endif

#define NB_DEBUG _DEBUG

#ifndef NB_DEBUG
#define NB_ASSERT(condition, message) ((void)0)
#else
#include <iostream>
#define NB_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "ASSERT FAILED: " << (message) << "\n" \
                      << "File: " << __FILE__ << "\n" \
                      << "Line: " << __LINE__ << "\n" \
                      << "Function: " << __func__ << "\n" \
                      << "Condition: " << #condition << std::endl; \
            std::abort(); \
        } \
    } while (false)
#endif

#define NB_NON_COPYABLE(ClassName) \
ClassName(const ClassName& other) noexcept = delete; \
ClassName& operator=(const ClassName& other) noexcept = delete; \

#define NB_NON_MOVABLE(ClassName) \
ClassName(ClassName&& other) noexcept = delete; \
ClassName& operator=(ClassName&& other) noexcept = delete;\

#define NB_NON_COPYMOVABLE(ClassName) \
NB_NON_COPYABLE(ClassName) \
NB_NON_MOVABLE(ClassName) \

#define NB_MOVE_ONLY(ClassName) \
NB_NON_COPYABLE(ClassName) \
ClassName(ClassName&& other) noexcept = default; \
ClassName& operator=(ClassName&& other) noexcept = default; \

#define NB_COPY_ONLY(ClassName) \
NB_NON_MOVABLE(ClassName) \
ClassName(const ClassName& other) noexcept = default; \
ClassName& operator=(const ClassName& other) noexcept = default; \

#endif