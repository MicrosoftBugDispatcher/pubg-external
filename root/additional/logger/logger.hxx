#pragma once
#include <chrono>
#include <ctime>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <windows.h>

namespace logger {

    void console(const char* title);

    // Log levels
    enum class level {
        info,
        success,
        warning,
        error,
        debug
    };

    // Internal state
    namespace internal {
        inline std::mutex log_mutex;
        inline std::ofstream log_file;
        inline bool file_logging_enabled = false;
        inline bool console_allocated = false;
    }

    // Initialize file logging
    inline void init_file_logging(const char* filename = "nirvana.log") {
        std::lock_guard<std::mutex> lock(internal::log_mutex);
        internal::log_file.open(filename, std::ios::out | std::ios::app);
        if (internal::log_file.is_open()) {
            internal::file_logging_enabled = true;
        }
    }

    // Close file logging
    inline void shutdown_file_logging() {
        std::lock_guard<std::mutex> lock(internal::log_mutex);
        if (internal::log_file.is_open()) {
            internal::log_file.close();
        }
        internal::file_logging_enabled = false;
    }

    // Allocate console
    inline void init_console(const char* title = "nirvana - logger") {
        if (!internal::console_allocated) {
            AllocConsole();
            FILE* f;
            freopen_s(&f, "CONOUT$", "w", stdout);
            freopen_s(&f, "CONOUT$", "w", stderr);
            SetConsoleTitleA(title);
            internal::console_allocated = true;
        }
    }

    // Color codes for different log levels (copied from rage)
    namespace colors {
        constexpr const char* timestamp = "\x1b[38;2;120;150;150m";  // Greyish cyan
        constexpr const char* tag = "\x1b[38;2;160;50;50m";        // Red
        constexpr const char* info = "\x1b[34m";                    // Blue
        constexpr const char* success = "\x1b[32m";                 // Green
        constexpr const char* warning = "\x1b[33m";                 // Yellow
        constexpr const char* error = "\x1b[31m";                   // Red
        constexpr const char* debug = "\x1b[36m";                   // Cyan
        constexpr const char* reset = "\x1b[0m";                    // Reset
    }

    inline const char* get_level_color( level lvl ) {
        switch ( lvl ) {
            case level::info:    return colors::info;
            case level::success: return colors::success;
            case level::warning: return colors::warning;
            case level::error:   return colors::error;
            case level::debug:   return colors::debug;
            default:             return colors::info;
        }
    }

    inline const char* get_level_prefix( level lvl ) {
        switch ( lvl ) {
            case level::info:    return "info";
            case level::success: return "success";
            case level::warning: return "warn";
            case level::error:   return "failed";
            case level::debug:   return "debug";
            default:             return "log";
        }
    }

    template<typename... Args>
    inline void log( level lvl, const char* format, Args... args ) {
        std::lock_guard<std::mutex> lock(internal::log_mutex);
        
        // Get current time
        auto now = std::chrono::system_clock::now( );
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        std::time_t time = std::chrono::system_clock::to_time_t( now );
        tm local_tm;
        localtime_s( &local_tm, &time );

        // Print timestamp (HH:MM:SS.mmm format like temp logger)
        printf( "%s%02d:%02d:%02d.%03lld%s  ",
            colors::timestamp,
            local_tm.tm_hour,
            local_tm.tm_min,
            local_tm.tm_sec,
            ms.count(),
            colors::reset );

        // Print level
        printf( "%s%s%s  ", get_level_color( lvl ), get_level_prefix( lvl ), colors::reset );

        // Print message in grey/white (no color)
        printf( format, args... );
        printf( "\n" );

        // File output (no ANSI codes)
        if (internal::file_logging_enabled && internal::log_file.is_open()) {
            internal::log_file 
                << local_tm.tm_hour << ":"
                << local_tm.tm_min << ":"
                << local_tm.tm_sec << "."
                << ms.count()
                << "  [" 
                << get_level_prefix( lvl ) 
                << "]  ";
            
            // Format message for file
            char message_buffer[1024];
            snprintf(message_buffer, sizeof(message_buffer), format, args...);
            internal::log_file << message_buffer << std::endl;
            internal::log_file.flush();
        }
    }

    // Convenience functions
    template<typename... Args>
    inline void info( const char* format, Args... args ) {
        log( level::info, format, args... );
    }

    template<typename... Args>
    inline void success( const char* format, Args... args ) {
        log( level::success, format, args... );
    }

    template<typename... Args>
    inline void warning( const char* format, Args... args ) {
        log( level::warning, format, args... );
    }

    template<typename... Args>
    inline void error( const char* format, Args... args ) {
        log( level::error, format, args... );
    }

    template<typename... Args>
    inline void debug( const char* format, Args... args ) {
        log( level::debug, format, args... );
    }
}
