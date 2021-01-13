#ifdef USE_MINI_GLOG

/// Liangliang: This file comes from Google GLOG, but most parts of it have been rewritten.
/// So now it is a quite light logging module.

// Copyright (c) 1999, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.

#include <easy3d/util/logging.h>
#include <easy3d/util/stack_tracer.h>
#include <easy3d/util/file_system.h>
#include <3rd_party/backward/backward.hpp>

#include <algorithm>
#include <thread>
#include <iomanip>
#include <cstring>
#include <mutex>


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <process.h>
#else
#include <unistd.h>
#endif


// \cond
namespace google {

    static std::string global_severity_labels[5] = {
            "INFO    ",
            "WARNING ",
            "ERROR   ",
            "FATAL   ",
            "QUIET   "
    };


    // see https://gist.github.com/vratiu/9780109
    static const char *global_severity_color_code[5] = {
            "",     // default -> INFO
            "3",    // yellow  -> WARNING
            "5",    // purple  -> ERROR
            "1",    // red     -> FATAL
            ""      // default -> QUIET
    };


    // This is the set of log sinks. This must be in a separate library to ensure
    // that there is only one instance of this across the entire program.
    static std::set<google::LogSink *> global_log_sinks;

    static google::LogSeverity global_log_severity_threshold;


    MessageLogger::MessageLogger(const char *file, int line, int severity)
            : full_path_(file), line_(line), stream_(0), severity_(severity) {
    }

    MessageLogger::MessageLogger(const char *file, int line, int severity, int ctr)
            : full_path_(file), line_(line), stream_(ctr), severity_(severity) {
    }

    // Output the contents of the stream to the proper channel on destruction.
    MessageLogger::~MessageLogger() {
        static std::mutex mutex;
        const std::lock_guard<std::mutex> lock(mutex);

        if (severity_ < global_log_severity_threshold)
            return;

        short_name_ = simple_name(std::string(full_path_));

        // get a precise timestamp as a string
        const auto now = std::chrono::system_clock::now();
        const auto now_as_time_t = std::chrono::system_clock::to_time_t(now);
        const auto now_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        std::stringstream time_stream;
        time_stream << std::put_time(std::localtime(&now_as_time_t), "%Y%m%d %T")
                    << '.' << std::setfill('0') << std::setw(3) << now_in_ms.count();
        time_str_ = time_stream.str();

        std::ostringstream pid_tid_stream;
        pid_tid_stream << std::setfill(' ') << std::setw(5)
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
                       << _getpid() << ':'
#else
                       << getpid() << ':'
#endif
                       << std::this_thread::get_id();
        pid_tid_str_ = pid_tid_stream.str();

        // remove all trailing ending line breaks
        message_ = stream_.str();
        while (message_.back() == '\n' && !message_.empty()) {
            message_.erase(message_.end() - 1);
        }

        if (severity_ == FATAL) {
            // If fatal error occurred, stop the program.
            // After logging the error (before the program crashes), notify StackTracer not to log again.
            if (!backward::SignalHandling::failure_has_been_recored) {
                message_ += "\n" + ::easy3d::logging::stacktrace_failure_header()
                            + "\n" + ::easy3d::StackTracer::back_trace_string(32, 5);
            }
        }

        std::ostringstream record;
        record << global_severity_labels[severity_][0] << ' ' << time_str_ << ' ' << pid_tid_str_
               << ' ' << short_name_ << ':' << line_ << "] " << message_ << std::endl;

        if (severity_ != QUIET)
            write_to_stderr(severity_, record.str().c_str(), record.str().size());

        LogToSinks();

        // release the lock so that signal handlers can use the logging facility.
        mutex.unlock();
        WaitForSinks();

        if (severity_ == FATAL) {
            // Now log has been written. Notify StackTracer not to log again.
            if (!backward::SignalHandling::failure_has_been_recored) {
                backward::SignalHandling::failure_has_been_recored = true;
                abort();
            }
        }
    }


    void MessageLogger::LogToSinks() const {
//        time_t rawtime;
//        time(&rawtime);
//
//        struct tm timeinfo;
//#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
//        // On Windows, use secure localtime_s not localtime.
//        localtime_s(&timeinfo, &rawtime);
//#else
//        // On non-Windows systems, use threadsafe localtime_r not localtime.
//        localtime_r(&rawtime, &timeinfo);
//#endif

        static std::mutex mutex;
        const std::lock_guard<std::mutex> lock(mutex);
        std::set<google::LogSink *>::iterator iter;
        // Send the log message to all sinks.
        for (iter = google::global_log_sinks.begin();
             iter != google::global_log_sinks.end(); ++iter) {
            (*iter)->send(severity_, line_, full_path_, short_name_, time_str_, pid_tid_str_, message_);
        }
    }


    void MessageLogger::WaitForSinks() {
        static std::mutex mutex;
        const std::lock_guard<std::mutex> lock(mutex);

        std::set<google::LogSink *>::iterator iter;
        // Call WaitTillSent() for all sinks.
        for (iter = google::global_log_sinks.begin();
             iter != google::global_log_sinks.end(); ++iter) {
            (*iter)->WaitTillSent();
        }
    }


    std::string MessageLogger::simple_name(const std::string &file_name) {
        static const char *const PATH_SEPARATORS = "/\\";
        std::string::size_type slash = file_name.find_last_of(PATH_SEPARATORS);
        if (slash == std::string::npos)
            return file_name;
        else
            return std::string(file_name.begin() + slash + 1, file_name.end());
    }


    // Returns true iff terminal supports using colors in output.
    bool MessageLogger::terminal_supports_colors() {
        // for now I assume colors are always supported.
        return true;

        static bool term_supports_color = false;
        static bool already_known = false;
        if (already_known)
            return term_supports_color;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
        // on Windows TERM variable is usually not set, but the console does
      // support colors.
      term_supports_color = true;
#else
        // On non-Windows platforms, we rely on the TERM variable.
        const char *const term = getenv("TERM");
        if (term != NULL && term[0] != '\0') {
            term_supports_color =
                    !strcmp(term, "xterm") ||
                    !strcmp(term, "xterm-color") ||
                    !strcmp(term, "xterm-256color") ||
                    !strcmp(term, "screen-256color") ||
                    !strcmp(term, "konsole") ||
                    !strcmp(term, "konsole-16color") ||
                    !strcmp(term, "konsole-256color") ||
                    !strcmp(term, "screen") ||
                    !strcmp(term, "linux") ||
                    !strcmp(term, "cygwin");
        }
#endif
        already_known = true;
        return term_supports_color;
    }


    void MessageLogger::write_to_stderr(google::LogSeverity severity, const char *message, size_t len) {
        const char *code = terminal_supports_colors() ? global_severity_color_code[severity] : global_severity_color_code[0];

        // Avoid using cerr from this module since we may get called during
        // exit code, and cerr may be partially or fully destroyed by then.
        if (global_severity_color_code[0] == code) {
            fwrite(message, len, 1, stderr);
            return;
        }

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
        const HANDLE stderr_handle = GetStdHandle(STD_ERROR_HANDLE);

      // Gets the current text color.
      CONSOLE_SCREEN_BUFFER_INFO buffer_info;
      GetConsoleScreenBufferInfo(stderr_handle, &buffer_info);
      const WORD old_color_attrs = buffer_info.wAttributes;

      // We need to flush the stream buffers into the console before each
      // SetConsoleTextAttribute call lest it affect the text that is already
      // printed but has not yet reached the console.
      fflush(stderr);
      SetConsoleTextAttribute(stderr_handle,
                              GetColorAttribute(color) | FOREGROUND_INTENSITY);
      fwrite(message, len, 1, stderr);
      fflush(stderr);
      // Restores the text color.
      SetConsoleTextAttribute(stderr_handle, old_color_attrs);
#else
        fprintf(stderr, "\033[0;3%sm", code);
        fwrite(message, len, 1, stderr);
        fprintf(stderr, "\033[m");  // Resets the terminal to default.
#endif
    }

}  // namespace google

// \endcond

#endif

#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stack_tracer.h>


INITIALIZE_EASYLOGGINGPP


namespace easy3d
{

    namespace logging
    {

        std::string log_file_name = "";


        std::string stacktrace_failure_header() {
            auto compose_header = []() -> std::string {
                std::stringstream stream;
                stream << "================================================================================="
                       << "\nEasy3D has encountered a fatal error and has to abort. ";

                if (!log_file_name.empty()) { // a log file exists
                    stream << "The error has been recorded \n"
                           << "in the log file [" + easy3d::file_system::absolute_path(log_file_name) + "].";
                }

                stream << "\nPlease report this issue with the complete log, a description of how to reproduce";
                stream << "\nthe issue, and possibly your data to Liangliang Nan (liangliang.nan@gmail.com).";
                stream << "\n=================================================================================";
                stream << "\n*** Check failure stack trace (most recent call first): ***";
                return stream.str();
            };
            static std::string header = compose_header();
            return header;
        }


        static std::string crashReason(int sig) {
            std::stringstream ss;
            bool foundReason = false;
            for (int i = 0; i < el::base::consts::kCrashSignalsCount; ++i) {
                if (el::base::consts::kCrashSignals[i].numb == sig) {
                    ss << "Application has crashed due to [" << el::base::consts::kCrashSignals[i].name << "] signal";
                    if (ELPP->hasFlag(el::LoggingFlag::LogDetailedCrashReason)) {
                        ss << std::endl <<
                           "    " << el::base::consts::kCrashSignals[i].brief << std::endl <<
                           "    " << el::base::consts::kCrashSignals[i].detail;
                    }
                    foundReason = true;
                }
            }
            if (!foundReason) {
                ss << "Application has crashed due to unknown signal [" << sig << "]";
            }
            return ss.str();
        }

        void myCrashHandler(int sig) {
#if 0
            el::Helpers::logCrashReason(sig, true);
#else
            std::stringstream ss;
            ss << crashReason(sig) << "\n"
               << stacktrace_failure_header() << "\n"
//               << el::base::debug::StackTrace();
                << StackTracer::back_trace_string(32, 5); // more reliable and readable than el

            LOG(FATAL) << ss.str();
#endif
            // FOLLOWING LINE IS ABSOLUTELY NEEDED AT THE END IN ORDER TO ABORT APPLICATION
            el::Helpers::crashAbort(sig);
        }



        void initialize(bool verbose, const std::string &log_file)
        {
            std::string full_path = log_file;
            if (log_file == "default") {
                const std::string app_path = file_system::executable();
                std::string log_path = app_path;
#ifdef __APPLE__
                // macOS may put the executable file in an application bundle, e.g., "PolyFit.app/Contents/MacOS/PolyFit"
                std::string::size_type pos = log_path.find(".app");
                if (pos != std::string::npos)
                    log_path = log_path.substr(0, pos);
#endif
                log_path = file_system::parent_directory(log_path) + "/logs";

                if (!file_system::is_directory(log_path))
                    file_system::create_directory(log_path);
                full_path = log_path + "/" + file_system::base_name(app_path) + ".log";
            }

            if (!full_path.empty()) {
                std::ofstream output(full_path.c_str(), std::ios::app);
                if (output.is_open()) {
                    auto size_bytes = static_cast<std::size_t>(output.tellp());
                    if (size_bytes > 0)
                        output << "\n\n";
                    output << "================================================================= program started ...\n";
                    output.close();
                    log_file_name = full_path;
                }
            }

//            // configures existing logger - everything in global.conf
//            el::Loggers::configureFromGlobal("global.conf");

            el::Configurations defaultConf;
            defaultConf.setToDefault();
            // Values are always std::string
            defaultConf.setGlobally(el::ConfigurationType::Format, "%levshort %datetime{%d/%M/%Y %h:%m:%s.%g} %fbase:%line] %msg");

            if (!log_file_name.empty()) {
                defaultConf.setGlobally(el::ConfigurationType::Filename, log_file_name);
                defaultConf.setGlobally(el::ConfigurationType::MaxLogFileSize, std::string("1048576")); // 1024 * 1024 = 1M
            }
            else
                defaultConf.setGlobally(el::ConfigurationType::ToFile, std::string("false"));

            defaultConf.set(el::Level::Info, el::ConfigurationType::ToStandardOutput, verbose ? "true" : "false");

            // If you wish to have a configuration for existing and future loggers, you can use the following.
            // This is useful when you are working on fairly large scale, or using a third-party library that is
            // already using Easylogging++. Any newly created logger will use default configurations. If you wish
            // to configure existing loggers as well, you can set second argument to true (it defaults to false).
            el::Loggers::setDefaultConfigurations(defaultConf, false);

            el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
            el::Loggers::addFlag(el::LoggingFlag::LogDetailedCrashReason);

            // default logger uses default configurations
            el::Loggers::reconfigureLogger("default", defaultConf);

            el::Helpers::setCrashHandler(myCrashHandler);

            LOG(INFO) << "executable path: " << file_system::executable_directory();
            LOG(INFO) << "current working dir: " << file_system::current_working_directory();
        }


        Logger::Logger() {
            el::base::elStorage->installLogDispatchCallback(this);
        }

    }

}
