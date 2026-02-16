#ifndef NB_SRC_MEMORYTRACKER_HPP
#define NB_SRC_MEMORYTRACKER_HPP

// SafeMemoryOverride.cpp - БЕЗОПАСНОЕ переопределение
#ifdef _DEBUG

#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include <map>
#include <mutex>

// ============================================================================
// ТОЛЬКО ДЛЯ ПОЛЬЗОВАТЕЛЬСКИХ АЛЛОКАЦИЙ (не для STL!)
// ============================================================================

namespace SafeMemoryTrack {
    struct AllocationInfo {
        size_t size;
        const char* file;
        int line;
        bool isArray;
    };

    static std::map<void*, AllocationInfo> g_customAllocations;
    static std::mutex g_mutex;
    static size_t g_totalCustomAllocated = 0;
    static size_t g_totalCustomFreed = 0;
    static bool g_enabled = true;

    // Флаги для определения, кто вызывает new
    static thread_local bool g_insideStlAllocation = false;

    class StlAllocationGuard {
    public:
        StlAllocationGuard() { g_insideStlAllocation = true; }
        ~StlAllocationGuard() { g_insideStlAllocation = false; }
    };

    static void TrackCustomAlloc(void* ptr, size_t size, const char* file, int line, bool isArray) {
        if (!g_enabled || !ptr || g_insideStlAllocation) return;

        std::lock_guard<std::mutex> lock(g_mutex);
        g_customAllocations[ptr] = { size, file, line, isArray };
        g_totalCustomAllocated += size;
    }

    static void TrackCustomFree(void* ptr) {
        if (!ptr || g_insideStlAllocation) return;

        std::lock_guard<std::mutex> lock(g_mutex);
        auto it = g_customAllocations.find(ptr);
        if (it != g_customAllocations.end()) {
            g_totalCustomFreed += it->second.size;
            g_customAllocations.erase(it);
        }
    }

    static void ReportCustomLeaks() {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (g_customAllocations.empty()) {
            printf("\n[OK] No CUSTOM memory leaks detected\n");
            return;
        }

        printf("\n[CUSTOM MEMORY LEAKS]\n");
        for (const auto& [ptr, info] : g_customAllocations) {
            printf("  %p: %zu bytes", ptr, info.size);
            if (info.file) printf(" at %s:%d", info.file, info.line);
            printf("\n");
        }
    }

    class AutoReporter {
    public:
        ~AutoReporter() {
            g_enabled = false;
            ReportCustomLeaks();
        }
    } g_autoReporter;
}

// ============================================================================
// МАКРОСЫ вместо глобального переопределения (БЕЗОПАСНО!)
// ============================================================================

// Макросы для использования в ВАШЕМ коде
#define NB_NEW new(__FILE__, __LINE__)
#define NB_DELETE(p) do { delete p; p = nullptr; } while(0)

// Перегрузки ТОЛЬКО для версий с местом вызова
void* operator new(size_t size, const char* file, int line) {
    // Проверяем, не STL ли это
    SafeMemoryTrack::StlAllocationGuard stlGuard;

    void* ptr = malloc(size);
    SafeMemoryTrack::TrackCustomAlloc(ptr, size, file, line, false);
    return ptr;
}

void* operator new[](size_t size, const char* file, int line) {
    SafeMemoryTrack::StlAllocationGuard stlGuard;

    void* ptr = malloc(size);
    SafeMemoryTrack::TrackCustomAlloc(ptr, size, file, line, true);
    return ptr;
}

// Delete для размещенных версий
void operator delete(void* ptr, const char*, int) noexcept {
    SafeMemoryTrack::TrackCustomFree(ptr);
    free(ptr);
}

void operator delete[](void* ptr, const char*, int) noexcept {
    SafeMemoryTrack::TrackCustomFree(ptr);
    free(ptr);
}

// ============================================================================
// НЕ ПЕРЕОПРЕДЕЛЯЕМ стандартные new/delete - ОСТАВЛЯЕМ STL в покое!
// ============================================================================

extern "C" {
    __declspec(dllexport) void DumpMemoryLeaks() {
        SafeMemoryTrack::ReportCustomLeaks();
    }
}

#else
// Release версия
#define NB_NEW new
#define NB_DELETE(p) delete p
extern "C" {
    __declspec(dllexport) void DumpMemoryLeaks() {}
}
#endif // _DEBUG
#endif // NB_SRC_MEMORYTRACKER_HPP
