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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <thread>
#include <iomanip>
#include <cstring>


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <process.h>
#else
#include <unistd.h>
#endif


namespace google {

    // This is the set of log sinks. This must be in a separate library to ensure
    // that there is only one instance of this across the entire program.
    static std::set<google::LogSink *> log_sinks_global;

    // Note: the Log sink functions are not thread safe.
    void AddLogSink(LogSink *sink) {
        // TODO(settinger): Add locks for thread safety.
        log_sinks_global.insert(sink);
    }

    void RemoveLogSink(LogSink *sink) {
        log_sinks_global.erase(sink);
    }

    logging_fail_func_t g_logging_fail_func = nullptr;

    void InstallFailureFunction(void (*fail_func)()) {
        g_logging_fail_func = (logging_fail_func_t)fail_func;
    }


    const std::string MessageLogger::severity_labels[4] = {
            "INFO    ",
            "WARNING ",
            "ERROR   ",
            "FATAL   "
    };


    // see https://gist.github.com/vratiu/9780109
    const char *MessageLogger::severity_color_code[4] = {
            "",     // default -> INFO
            "3",    // yellow  -> WARNING
            "5",    // purple  -> ERROR
            "1"     // red     -> FATAL
    };

    MessageLogger::MessageLogger(const char *file, int line, int severity)
            : full_path_(file), line_(line), severity_(severity) {
    }

    MessageLogger::MessageLogger(const char *file, int line, int severity, int ctr)
            : full_path_(file), line_(line), severity_(severity) {
    }

    // Output the contents of the stream to the proper channel on destruction.
    MessageLogger::~MessageLogger() {
        short_name_ = simple_name(std::string(full_path_));

        // get a precise timestamp as a string
        const auto now = std::chrono::system_clock::now();
        const auto now_as_time_t = std::chrono::system_clock::to_time_t(now);
        const auto now_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        std::stringstream time_stream;
        time_stream << std::put_time(std::localtime(&now_as_time_t), "%Y%m%d:%T")
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

        // remove all the ending line breaks
        message_ = stream_.str();
        while (message_.back() == '\n' && !message_.empty()) {
            message_.erase(message_.end() - 1);
        }

        if (severity_ == FATAL) {
            message_ += ("\n================================================================================="
                         "\nEasy3D encountered a problem."
                         "\nPlease report this issue with the complete log, a description of how to reproduce"
                         "\nthe issue, and possibly your data to Liangliang Nan (liangliang.nan@gmail.com)."
                         "\n================================================================================="
                         "\nStack trace (most recent call first):\n"
                         + easy3d::StackTracer::back_trace(2));
        }

        std::ostringstream record;
        record << severity_labels[severity_][0] << ' ' << time_str_ << ' ' << pid_tid_str_ << ' ' << short_name_ << ':'
               << line_ << "] " << message_ << std::endl;
        write_to_stderr(severity_, record.str().c_str(), record.str().size());

        LogToSinks();
        WaitForSinks();

        // If fatal error occurred, stop the program.
        if (severity_ == FATAL) {
            if (g_logging_fail_func)
                g_logging_fail_func();
            abort();
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

        std::set<google::LogSink *>::iterator iter;
        // Send the log message to all sinks.
        for (iter = google::log_sinks_global.begin();
             iter != google::log_sinks_global.end(); ++iter) {
            (*iter)->send(severity_, line_, full_path_, short_name_, time_str_, pid_tid_str_, message_);
        }
    }


    void MessageLogger::WaitForSinks() {
        // TODO(settinger): Add locks for thread safety.
        std::set<google::LogSink *>::iterator iter;

        // Call WaitTillSent() for all sinks.
        for (iter = google::log_sinks_global.begin();
             iter != google::log_sinks_global.end(); ++iter) {
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
        const char *code = terminal_supports_colors() ? severity_color_code[severity] : severity_color_code[0];

        // Avoid using cerr from this module since we may get called during
        // exit code, and cerr may be partially or fully destroyed by then.
        if (severity_color_code[0] == code) {
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




#include <easy3d/util/file_system.h>


namespace easy3d
{

    namespace logging
    {

        void initialize(int severity_threshold, const std::string& log_file)
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
                LOG_IF(ERROR, !file_system::is_directory(log_path)) << "could not create log directory: " << log_path;

                full_path = log_path + "/" + file_system::base_name(app_path) + ".log";
            }

            if (!full_path.empty()) {
                static FileLogClient client(full_path);
            }

            DLOG(INFO) << "logger initialized";
            DLOG(INFO) << "executable path: " << file_system::executable_directory();
            DLOG(INFO) << "current working dir: " << file_system::current_working_directory();
        }



        LogClient::LogClient() {
            google::AddLogSink(this);
        }


        FileLogClient::FileLogClient(const std::string &file_name) {
            static std::ofstream output(file_name.c_str(), std::ios::app);
            if (output.is_open()) {
                output_ = &output;
                google::AddLogSink(this);
            }
            else {
                output_ = nullptr;
                LOG(WARNING) << "failed to creat file logger";
            }
        }

        void FileLogClient::send(google::LogSeverity severity, int line_number, const std::string &full_path,
                                 const std::string &short_name, const std::string &time,
                                 const std::string &pid_tid, const std::string &msg) {
            if (output_)
                (*output_) << MessageLogger::severity_labels[severity][0] << ' ' << time << ' ' << pid_tid << ' '
                           << short_name << ':' << line_number << "] " << msg << std::endl;
        }

    }

}
