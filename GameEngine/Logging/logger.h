#pragma once

#include <map>
#include <boost/log/core.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/trivial.hpp>

namespace GameEngine
{
namespace Logging
{
    class Logger
    {
    public:

        enum LogLevel
        {
            kTrace,
            kDebug,
            kInfo,
            kWarning,
            kError,
            kFatal
        };

        // Disable all calls to the specified logging level
        static void disable_lvl(LogLevel lvl); 

        // Completely disable logging
        static void disable_all();

        // Enable all calls to the specified logging level
        static void enable_lvl(LogLevel lvl);

        // Reset the logger back to its original state
        // The original state is where
        static void enable_all();

        // Any log call with the level above or equal to the specified lvl will be logged
        static void set_log_level(const LogLevel& lvl);

        static LogLevel log_level()
        {
            return s_log_level;
        }

		template <typename T>
        static void trace(const std::string& message, const T& obj)
        {
            if (s_is_trace_enabled)
                BOOST_LOG_TRIVIAL(trace) << obj << message;
        }

		template <typename T>
        static void debug(const std::string& message, const T& obj)
        {
            if (s_is_debug_enabled)
                BOOST_LOG_TRIVIAL(debug) << obj << message;
        }

		template <typename T>
        static void info(const std::string& message, const T& obj)
        {
            if (s_is_info_enabled)
                BOOST_LOG_TRIVIAL(info) << obj << message;
        }

		template <typename T>
        static void warning(const std::string& message, const T& obj)
        {
            if (s_is_warning_enabled)
                BOOST_LOG_TRIVIAL(warning) << obj << message;
        }

		template <typename T>
        static void error(const std::string& message, const T& obj)
        {
            if (s_is_error_enabled)
                BOOST_LOG_TRIVIAL(error) << obj << message;
        }

		template <typename T>
        static void fatal(const std::string& message, const T& obj)
        {
            if (s_is_fatal_enabled)
                BOOST_LOG_TRIVIAL(fatal) << obj << message;
        }

    private:
        static LogLevel s_log_level;
        static bool s_is_trace_enabled;
        static bool s_is_debug_enabled;
        static bool s_is_info_enabled;
        static bool s_is_warning_enabled;
        static bool s_is_error_enabled;
        static bool s_is_fatal_enabled;

    public:
        static std::map<LogLevel, std::string> s_string_to_log_level_map;
        static std::map<std::string, LogLevel> s_level_to_string_map;
    };
}
}
