#ifndef EASY3D_UTIL_LOGGING_H
#define EASY3D_UTIL_LOGGING_H

#include <easy3d/util/easylogging++.h>

namespace easy3d {

    /// \brief The logging mechanism.
    /// \namespace easy3d::logging
    namespace logging {

        /**
         * @brief Initializes the logging module.
         * @param verbose \c ture to log messages at a the INFO level to stderr.
         *      \c false only log at WARNING, ERROR, FATAL levels.
         * @param log_file A string specifying the full path to the log file.
         *      If valid, the log messages will be written to this file in addition to logging to stderr.
         *      No log file will be created if \p log_file is empty.
         *      Passing "default" allows to creat a log file with a title "ApplicationName.log" in a
         *      directory "logs" next to the executable file.
         * @note This initialization is optional. If not called, log messages will be written to stderr only.
         */
        void initialize(bool verbose = false, const std::string &log_file = "default");


        /// Base class for a logger, i.e., to log messages to whatever
        /// Users should subclass Logger and override send() to do whatever they want.
        /// \class Logger easy3d/util/logging.h
        class Logger : public el::LogDispatchCallback {
        public:
            Logger();

            /// writes the log message \p msg (and may also other given information).
            virtual void send(el::Level level, const std::string& msg) = 0;

        protected:
            void handle(const el::LogDispatchData* data) noexcept override {
                send(data->logMessage()->level(), data->logMessage()->message());
            }
        };

    };



// to have the same syntax as glog
#define LOG_FIRST_N LOG_N_TIMES

// to have LOG_IF_EVERY_N
#define LOG_IF_EVERY_N(n, condition, LEVEL)  if (condition) \
    CLOG_EVERY_N(n, LEVEL, ELPP_CURR_FILE_LOGGER_ID)

// for logging the counter number
#define COUNTER     ELPP_COUNTER->hitCounts()

}



#endif  // EASY3D_UTIL_LOGGING_H
