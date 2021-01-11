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



// Any logging of level 2 and above is directed to the Verbose level. All
// Android log output is tagged with the string "native".
//
// If the symbol ANDROID is not defined, all output goes to std::cerr.
// This allows code to be built on a different system for debug.
//
// Portions of this code are taken from the GLOG package.  This code is only a
// small subset of the GLOG functionality. Notable differences from GLOG
// behavior include lack of support for displaying unprintable characters and
// lack of stack trace information upon failure of the CHECK macros.
//
// CHECK macros are defined to test for conditions within code.  Any CHECK that
// fails will log the failure and terminate the application.
// e.g. CHECK_GE(3, 2) will pass while CHECK_GE(3, 4) will fail after logging
//      "Check failed 3 >= 4".
//
// The following CHECK macros are defined:
//
//   CHECK(condition)        - fails if condition is false and logs condition.
//   CHECK_NOTNULL(variable) - fails if the variable is NULL.
//
// The following binary check macros are also defined :
//
//   Macro                     Operator equivalent
//   --------------------      -------------------
//   CHECK_EQ(val1, val2)      val1 == val2
//   CHECK_NE(val1, val2)      val1 != val2
//   CHECK_GT(val1, val2)      val1 > val2
//   CHECK_GE(val1, val2)      val1 >= val2
//   CHECK_LT(val1, val2)      val1 < val2
//   CHECK_LE(val1, val2)      val1 <= val2
//
// Debug only versions of all of the check macros are also defined.  These
// macros generate no code in a release build, but avoid unused variable
// warnings / errors.
//
// To use the debug only versions, prepend a D to the normal check macros, e.g.
// DCHECK_EQ(a, b).

#ifndef EASY3D_UTIL_LOGGING_H
#define EASY3D_UTIL_LOGGING_H

#include <ctime>
#include <set>
#include <string>
#include <sstream>


namespace easy3d {

    // Log severity level constants.
    const int INFO = 0;
    const int WARNING = 1;
    const int ERROR = 2;
    const int FATAL = 3;
    const int QUIET = 4; // log messages written to sinks (e.g., a log file) only, not to stderr

}

// ------------------------- Glog compatibility ------------------------------

// \cond
namespace google {

    typedef int LogSeverity;
    const int INFO = easy3d::INFO;
    const int WARNING = easy3d::WARNING;
    const int ERROR = easy3d::ERROR;
    const int FATAL = easy3d::FATAL;
    const int QUIET = easy3d::QUIET;

    // Sink class used for integration with mock and test functions. If sinks are
    // added, all log output is also sent to each sink through the send function.
    // In this implementation, WaitTillSent() is called immediately after the send.
    // This implementation is not thread safe.
    class LogSink {
    public:
        virtual ~LogSink() {}

        virtual void send(google::LogSeverity severity,
                          int line_number,
                          const std::string &full_path,
                          const std::string &short_name,
                          const std::string &time,
                          const std::string &pid_tid,
                          const std::string &msg) = 0;

        virtual void WaitTillSent() { /* client code should implement this */ }
    };

}
// \endcond

// ---------------------------- Logger Class --------------------------------

// \cond
namespace easy3d {
    namespace logging {
        // We want the special COUNTER value available for LOG_EVERY_X()'ed messages
        enum PRIVATE_Counter { COUNTER };
    }
}
// \endcond

// \cond
namespace google {

    class LogStream : public std::ostringstream {
    public:
        LogStream(int ctr) : ctr_(ctr) {}
        int ctr() const { return ctr_; }

    private:
        LogStream(const LogStream &);
        LogStream &operator=(const LogStream &);
        int ctr_;  // Counter hack (for the LOG_EVERY_X() macro)
    };
}
// \endcond


// \cond
// Define global operator<< to declare using ::operator<<.
// This allows to output the COUNTER value. This is only valid if ostream is a LogStream.
// You must not use COUNTER with non-glog ostream
inline std::ostream& operator<<(std::ostream &os, const easy3d::logging::PRIVATE_Counter&) {
    auto log = dynamic_cast<google::LogStream*>(&os);
    if (log)
        os << log->ctr();
    return os;
}
// \endcond


// \cond
namespace google {

    // Class created for each use of the logging macros.
    // The logger acts as a stream and routes the final stream contents to the
    // Android logcat output at the proper filter level.  If ANDROID is not
    // defined, output is directed to std::cerr.  This class should not
    // be directly instantiated in code, rather it should be invoked through the
    // use of the log macros LG, LOG, or VLOG.
    class MessageLogger {
    public:
        MessageLogger(const char *file, int line, google::LogSeverity severity);
        MessageLogger(const char *file, int line, google::LogSeverity severity, int ctr);

        // Output the contents of the stream to the proper channel on destruction.
        ~MessageLogger();

        // Return the stream associated with the logger object.
        LogStream &stream() { return stream_; }

    private:

        static bool terminal_supports_colors();

    private:
        void LogToSinks() const;

        void WaitForSinks();

        std::string simple_name(const std::string &full_path);

        // Avoid using cerr from this module since we may get called during
        // exit code, and cerr may be partially or fully destroyed by then.
        static void write_to_stderr(google::LogSeverity severity, const char *message, size_t len);

    private:
        std::string full_path_;
        std::string short_name_;
        int line_;
        std::string time_str_;
        std::string pid_tid_str_;
        std::string message_;
        LogStream stream_;
        int severity_;
    };

    // ---------------------- Logging Macro definitions --------------------------

    // This class is used to explicitly ignore values in the conditional
    // logging macros.  This avoids compiler warnings like "value computed
    // is not used" and "statement has no effect".
    class LoggerVoidify {
    public:
        LoggerVoidify() {}

        // This has to be an operator with a precedence lower than << but
        // higher than ?:
        void operator&(const std::ostream &s) { (void) s; }
    };
}


using google::MessageLogger;
using google::LoggerVoidify;

// \endcond

// Log only if condition is met.  Otherwise evaluates to void.
#define LOG_IF(severity, condition) \
    !(condition) ? (void) 0 : LoggerVoidify() & \
      MessageLogger(__FILE__, __LINE__, severity).stream()

// Log only if condition is NOT met.  Otherwise evaluates to void.
#define LOG_IF_FALSE(severity, condition) LOG_IF(severity, !(condition))

// LG is a convenient shortcut for LOG(INFO). Its use is in new
// google3 code is discouraged and the following shortcut exists for
// backward compatibility with existing code.
#ifdef MAX_LOG_LEVEL
#  define LOG(n)  LOG_IF(n, n <= MAX_LOG_LEVEL)
#  define VLOG(n) LOG_IF(n, n <= MAX_LOG_LEVEL)
#  define LG      LOG_IF(INFO, INFO <= MAX_LOG_LEVEL)
#  define VLOG_IF(n, condition) LOG_IF(n, (n <= MAX_LOG_LEVEL) && condition)
#else
#  define LOG(n)  MessageLogger(__FILE__, __LINE__, n).stream()    // NOLINT
#  define VLOG(n) MessageLogger(__FILE__, __LINE__, n).stream()    // NOLINT
#  define LG      MessageLogger(__FILE__, __LINE__, INFO).stream() // NOLINT
#  define VLOG_IF(n, condition) LOG_IF(n, condition)
#endif

// Currently, VLOG is always on for levels below MAX_LOG_LEVEL.
#ifndef MAX_LOG_LEVEL
#  define VLOG_IS_ON(x) (1)
#else
#  define VLOG_IS_ON(x) (x <= MAX_LOG_LEVEL)
#endif

#ifndef NDEBUG
#  define DLOG LOG
#else
#  define DLOG(severity) true ? (void) 0 : LoggerVoidify() & \
      MessageLogger((char *)__FILE__, __LINE__, severity).stream()
#endif


// \cond

// Use macro expansion to create, for each use of LOG_EVERY_N(), static
// variables with the __LINE__ expansion as part of the variable name.
#define LOG_EVERY_N_VARNAME(base, line) LOG_EVERY_N_VARNAME_CONCAT(base, line)
#define LOG_EVERY_N_VARNAME_CONCAT(base, line) base ## line

#define LOG_OCCURRENCES LOG_EVERY_N_VARNAME(occurrences_, __LINE__)
#define LOG_OCCURRENCES_MOD_N LOG_EVERY_N_VARNAME(occurrences_mod_n_, __LINE__)

#define SOME_KIND_OF_LOG_EVERY_N(severity, n) \
  static int LOG_OCCURRENCES = 0, LOG_OCCURRENCES_MOD_N = 0; \
  ++LOG_OCCURRENCES; \
  if (++LOG_OCCURRENCES_MOD_N > n) LOG_OCCURRENCES_MOD_N -= n; \
  if (LOG_OCCURRENCES_MOD_N == 1) \
    MessageLogger( \
        __FILE__, __LINE__, severity, LOG_OCCURRENCES).stream()

#define SOME_KIND_OF_LOG_IF_EVERY_N(severity, condition, n) \
  static int LOG_OCCURRENCES = 0, LOG_OCCURRENCES_MOD_N = 0; \
  ++LOG_OCCURRENCES; \
  if (condition && \
      ((LOG_OCCURRENCES_MOD_N=(LOG_OCCURRENCES_MOD_N + 1) % n) == (1 % n))) \
    MessageLogger( \
        __FILE__, __LINE__, severity, LOG_OCCURRENCES).stream()

#define SOME_KIND_OF_LOG_FIRST_N(severity, n) \
  static int LOG_OCCURRENCES = 0; \
  if (LOG_OCCURRENCES <= n) \
    ++LOG_OCCURRENCES; \
  if (LOG_OCCURRENCES <= n) \
    MessageLogger( \
        __FILE__, __LINE__, severity, LOG_OCCURRENCES).stream()

// \endcond


#define LOG_EVERY_N(severity, n)                SOME_KIND_OF_LOG_EVERY_N(severity, (n))
#define LOG_FIRST_N(severity, n)                SOME_KIND_OF_LOG_FIRST_N(severity, (n))
#define LOG_IF_EVERY_N(severity, condition, n)  SOME_KIND_OF_LOG_IF_EVERY_N(severity, (condition), (n))


#ifndef NDEBUG
#define DLOG_IF(severity, condition)            LOG_IF(severity, condition)
#define DLOG_FIRST_N(severity, n)               LOG_FIRST_N(severity, (n))
#define DLOG_EVERY_N(severity, n)               LOG_EVERY_N(severity, n)
#define DLOG_IF_EVERY_N(severity, condition, n) LOG_IF_EVERY_N(severity, condition, n)
#else
#define DLOG_IF(severity, condition) \
  static_cast<void>(0),              \
  (true || !(condition)) ? (void) 0 : LoggerVoidify() & MessageLogger((char *)__FILE__, __LINE__, severity).stream()

#define DLOG_EVERY_N(severity, n) \
  static_cast<void>(0),           \
  true ? (void) 0 : LoggerVoidify() & MessageLogger((char *)__FILE__, __LINE__, severity).stream()

#define DLOG_FIRST_N(severity, n) \
  static_cast<void>(0),           \
  true ? (void) 0 : LoggerVoidify() & MessageLogger((char *)__FILE__, __LINE__, severity).stream()

#define DLOG_IF_EVERY_N(severity, condition, n) \
  static_cast<void>(0),                         \
  (true || !(condition))? (void) 0 : LoggerVoidify() & MessageLogger((char *)__FILE__, __LINE__, severity).stream()

#endif


#define LOG_ASSERT(condition)  \
  LOG_IF(FATAL, !(condition)) << "Assert failed: " #condition

#ifndef NDEBUG
#define DLOG_ASSERT(condition) LOG_ASSERT(condition)
#else
#define DLOG_ASSERT(condition) static_cast<void>(0), true ? (void) 0 : LOG_ASSERT(condition)
#endif


// cond


// Log a message and terminate.
template<class T>
void LogMessageFatal(const char *file, int line, const T &message) {
    MessageLogger(file, line, easy3d::FATAL).stream() << message;
}

// \endcond


// ---------------------------- CHECK macros ---------------------------------

// Check for a given boolean condition.
#define CHECK(condition) LOG_IF_FALSE(FATAL, condition) \
        << "Check failed: " #condition " "

#ifndef NDEBUG
// Debug only version of CHECK
#  define DCHECK(condition) LOG_IF_FALSE(FATAL, condition) \
          << "Check failed: " #condition " "
#else
// Optimized version - generates no code.
#  define DCHECK(condition) if (false) LOG_IF_FALSE(FATAL, condition) \
          << "Check failed: " #condition " "
#endif  // NDEBUG

// ------------------------- CHECK_OP macros ---------------------------------

// Generic binary operator check macro. This should not be directly invoked,
// instead use the binary comparison macros defined below.
#define CHECK_OP(val1, val2, op) LOG_IF_FALSE(FATAL, ((val1) op (val2))) \
  << "Check failed: " #val1 " " #op " " #val2 " "

// Check_op macro definitions
#define CHECK_EQ(val1, val2) CHECK_OP(val1, val2, ==)
#define CHECK_NE(val1, val2) CHECK_OP(val1, val2, !=)
#define CHECK_LE(val1, val2) CHECK_OP(val1, val2, <=)
#define CHECK_LT(val1, val2) CHECK_OP(val1, val2, <)
#define CHECK_GE(val1, val2) CHECK_OP(val1, val2, >=)
#define CHECK_GT(val1, val2) CHECK_OP(val1, val2, >)


#define CHECK_NEAR(val1, val2, margin)           \
  do {                                           \
    CHECK_LE((val1), (val2)+(margin));           \
    CHECK_GE((val1), (val2)-(margin));           \
  } while (0)

#ifndef NDEBUG
// Debug only versions of CHECK_OP macros.
#  define DCHECK_EQ(val1, val2) CHECK_OP(val1, val2, ==)
#  define DCHECK_NE(val1, val2) CHECK_OP(val1, val2, !=)
#  define DCHECK_LE(val1, val2) CHECK_OP(val1, val2, <=)
#  define DCHECK_LT(val1, val2) CHECK_OP(val1, val2, <)
#  define DCHECK_GE(val1, val2) CHECK_OP(val1, val2, >=)
#  define DCHECK_GT(val1, val2) CHECK_OP(val1, val2, >)
#  define DCHECK_NEAR(val1, val2, margin) CHECK_NEAR(val1, val2, margin)
#else
// These versions generate no code in optimized mode.
#  define DCHECK_EQ(val1, val2) if (false) CHECK_OP(val1, val2, ==)
#  define DCHECK_NE(val1, val2) if (false) CHECK_OP(val1, val2, !=)
#  define DCHECK_LE(val1, val2) if (false) CHECK_OP(val1, val2, <=)
#  define DCHECK_LT(val1, val2) if (false) CHECK_OP(val1, val2, <)
#  define DCHECK_GE(val1, val2) if (false) CHECK_OP(val1, val2, >=)
#  define DCHECK_GT(val1, val2) if (false) CHECK_OP(val1, val2, >)
#  define DCHECK_NEAR(val1, val2, margin) if (false) CHECK_NEAR(val1, val2, margin)
#endif  // NDEBUG

// ---------------------------CHECK_NOTNULL macros ---------------------------

// \cond


// Helpers for CHECK_NOTNULL(). Two are necessary to support both raw pointers
// and smart pointers.
template<typename T>
T &CheckNotNullCommon(const char *file, int line, const char *names, T &t) {
    if (t == NULL) {
        LogMessageFatal(file, line, std::string(names));
    }
    return t;
}

template<typename T>
T *CheckNotNull(const char *file, int line, const char *names, T *t) {
    return CheckNotNullCommon(file, line, names, t);
}

template<typename T>
T &CheckNotNull(const char *file, int line, const char *names, T &t) {
    return CheckNotNullCommon(file, line, names, t);
}

// \endcond


// Check that a pointer is not null.
#define CHECK_NOTNULL(val) \
  CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

#ifndef NDEBUG
// Debug only version of CHECK_NOTNULL
#define DCHECK_NOTNULL(val) \
  CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))
#else
// Optimized version - generates no code.
#define DCHECK_NOTNULL(val) if (false)\
  CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))
#endif  // NDEBUG


// ---------------------------TRACE macros ---------------------------
#define __FILENAME__ \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define DEXEC(fn)                                                      \
{                                                                      \
    DLOG(INFO) << "[EXEC " << #fn << " START]";                        \
    std::chrono::steady_clock::time_point begin =                      \
      std::chrono::steady_clock::now();                                \
    fn();                                                                \
    std::chrono::steady_clock::time_point end =                        \
      std::chrono::steady_clock::now();                                \
    DLOG(INFO) << "[EXEC " << #fn << " FINISHED in "                   \
             << std::chrono::duration_cast<std::chrono::microseconds>  \
               (end - begin).count() << " ms]";                        \
  }
// DEXEC(fn)
//
// Usage:
// DEXEC(foo());
// -- output --
// foo.cpp: 123 [EXEC foo() START]
// foo.cpp: 123 [EXEC foo() FINISHED in 456 ms]

#define DTRACE  DLOG(INFO) << "of [" << __func__ << "]";
// Usage:
// void foo() {
//   DTRACE
// }
// -- output --
// foo.cpp: 123 of [void foo(void)]


// --------------------------- LOG STL containers ---------------------------

#include <deque>
#include <list>
#include <map>
#include <ostream>
#include <set>
#include <utility>
#include <vector>


// \cond

// Forward declare these two, and define them after all the container streams
// operators so that we can recurse from pair -> container -> container -> pair
// properly.
template<class First, class Second>
std::ostream &operator<<(std::ostream &out, const std::pair<First, Second> &p);

namespace google {

    template<class Iter>
    void PrintSequence(std::ostream &out, Iter begin, Iter end);

}

#define OUTPUT_TWO_ARG_CONTAINER(Sequence) \
template<class T1, class T2> \
inline std::ostream& operator<<(std::ostream& out, \
                                const Sequence<T1, T2>& seq) { \
  google::PrintSequence(out, seq.begin(), seq.end()); \
  return out; \
}

OUTPUT_TWO_ARG_CONTAINER(std::vector)

OUTPUT_TWO_ARG_CONTAINER(std::deque)

OUTPUT_TWO_ARG_CONTAINER(std::list)

#undef OUTPUT_TWO_ARG_CONTAINER

#define OUTPUT_THREE_ARG_CONTAINER(Sequence) \
template<class T1, class T2, class T3> \
inline std::ostream& operator<<(std::ostream& out, \
                                const Sequence<T1, T2, T3>& seq) { \
  google::PrintSequence(out, seq.begin(), seq.end()); \
  return out; \
}

OUTPUT_THREE_ARG_CONTAINER(std::set)

OUTPUT_THREE_ARG_CONTAINER(std::multiset)

#undef OUTPUT_THREE_ARG_CONTAINER

#define OUTPUT_FOUR_ARG_CONTAINER(Sequence) \
template<class T1, class T2, class T3, class T4> \
inline std::ostream& operator<<(std::ostream& out, \
                                const Sequence<T1, T2, T3, T4>& seq) { \
  google::PrintSequence(out, seq.begin(), seq.end()); \
  return out; \
}

OUTPUT_FOUR_ARG_CONTAINER(std::map)

OUTPUT_FOUR_ARG_CONTAINER(std::multimap)

#undef OUTPUT_FOUR_ARG_CONTAINER

#define OUTPUT_FIVE_ARG_CONTAINER(Sequence) \
template<class T1, class T2, class T3, class T4, class T5> \
inline std::ostream& operator<<(std::ostream& out, \
                                const Sequence<T1, T2, T3, T4, T5>& seq) { \
  google::PrintSequence(out, seq.begin(), seq.end()); \
  return out; \
}

#undef OUTPUT_FIVE_ARG_CONTAINER

template<class First, class Second>
inline std::ostream &operator<<(std::ostream &out,
                                const std::pair<First, Second> &p) {
    out << '(' << p.first << ", " << p.second << ')';
    return out;
}

namespace google {

    template<class Iter>
    inline void PrintSequence(std::ostream &out, Iter begin, Iter end) {
        // Output at most 100 elements -- appropriate if used for logging.
        for (int i = 0; begin != end && i < 100; ++i, ++begin) {
            if (i > 0) out << ' ';
            out << *begin;
        }
        if (begin != end) {
            out << " ...";
        }
    }

}

// Note that this is technically undefined behavior! We are adding things into
// the std namespace for a reason though -- we are providing new operations on
// types which are themselves defined with this namespace. Without this, these
// operator overloads cannot be found via ADL. If these definitions are not
// found via ADL, they must be #included before they're used, which requires
// this header to be included before apparently independent other headers.
//
// For example, base/logging.h defines various template functions to implement
// CHECK_EQ(x, y) and stream x and y into the log in the event the check fails.
// It does so via the function template MakeCheckOpValueString:
//   template<class T>
//   void MakeCheckOpValueString(strstream* ss, const T& v) {
//     (*ss) << v;
//   }
// Because 'glog/logging.h' is included before 'glog/stl_logging.h',
// subsequent CHECK_EQ(v1, v2) for vector<...> typed variable v1 and v2 can only
// find these operator definitions via ADL.
//
// Even this solution has problems -- it may pull unintended operators into the
// namespace as well, allowing them to also be found via ADL, and creating code
// that only works with a particular order of includes. Long term, we need to
// move all of the *definitions* into namespace std, bet we need to ensure no
// one references them first. This lets us take that step. We cannot define them
// in both because that would create ambiguous overloads when both are found.
namespace std { using ::operator<<; }

// \endcond

// End of ------ LOG STL containers ---------------------------


// Another great logging library:
//  https://github.com/amrayn/easyloggingpp

namespace easy3d {

    class StackTracer;

    /// \brief The logging mechanism.
    /// \namespace easy3d::logging
    namespace logging {

        /**
         * @brief Initializes the logging module.
         * @param threshold Log messages at a level >= this flag are automatically sent to standard error
         *        (stderr) in addition to log files. The value can be one of INFO, WARNING, and ERROR.
         * @param log_file If a valid file path is provided (i.e., the file can be created or opened),
         *      the log messages will be written to this file in addition to logging to stderr.
         *      No log file will be created if \p log_file is empty.
         *      Passing "default" allows to creat a log file with a title "ApplicationName.log" in a
         *      directory "logs" next to the executable file.
         * @note This initialization is optional. If not called, log messages will be written to stderr only.
         */
        void initialize(int threshold = INFO, const std::string &log_file = "default");


        /// Changes the log severity threshold.
        /// Only log messages at levels >= min_severity will be logged
        void set_severity_threshold(int min_severity);

        /// header string for recording stack trace.
        /// This is used when we log a FATAL message, or when the program crashes.
        std::string stacktrace_header();

        /// Base class for a logger, i.e., to log messages to whatever
        /// Users should subclass Logger and override send() to do whatever they want.
        /// \class Logger easy3d/util/logger.h
        class Logger : public google::LogSink {
        public:
            Logger();

            /// writes the log message \p msg (and may also other given information).
            virtual void send(google::LogSeverity severity,
                      int line_number,
                      const std::string &file_full_path,
                      const std::string &file_short_name,
                      const std::string &time,
                      const std::string &pid_tid,
                      const std::string &msg) override = 0;
        };

        /// Add a logger
        void add_logger(Logger *logger);

        /// Removes a logger
        void remove_logger(Logger *logger);

        /// A file logger logs all log message into a file.
        class FileLogger : public Logger {
        public:
            FileLogger(const std::string &file_name, std::ios::openmode mode = std::ios::app);

            /// returns the log file name.
            /// returns and empty string if the stream doesn't exist.
            static const std::string log_file_name();

            /// writes the log message \p msg (and may also other given information) into the log file.
            void send(google::LogSeverity severity,
                      int line_number,
                      const std::string &file_full_path,
                      const std::string &file_short_name,
                      const std::string &time,
                      const std::string &pid_tid,
                      const std::string &msg) override;

            /// appends a message
            void append(const std::string& msg);

        private:
            static std::ofstream* output_;
            static std::string log_file_name_;
        };

    };

}


#endif  // EASY3D_UTIL_LOGGING_H
