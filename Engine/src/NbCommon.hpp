#ifndef SRC_NB_COMMON_HPP
#define SRC_NB_COMMON_HPP

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

/// todo find compiler defines
// that mb incoorect
//
#if defined(__clang__)


#elif defined(__GNUC__) || defined(__GNUG__)


#elif defined(_MSC_VER)
//
#endif

#endif