/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stack_tracer.h>


INITIALIZE_EASYLOGGINGPP


namespace easy3d
{

    // cond
    namespace logging
    {

        // \cond
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


        std::string crash_reason(int sig) {
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


        void crash_sandler(int sig) {
            std::stringstream ss;
            ss << crash_reason(sig) << "\n"
               << stacktrace_failure_header() << "\n"
//               << el::base::debug::StackTrace();
                << StackTracer::back_trace_string(32, 4); // more reliable and readable than el

            LOG(FATAL) << ss.str();

            // FOLLOWING LINE IS ABSOLUTELY NEEDED AT THE END IN ORDER TO ABORT APPLICATION
            el::Helpers::crashAbort(sig);
        }

        // \endcond

        void initialize(bool info_to_stderr, const std::string &log_file, int verbosity_threshold)
        {
            std::string full_path_log_file = log_file;
            if (log_file == "default") {
                const std::string app_path = file_system::executable();
                std::string log_path = app_path;
#ifdef __APPLE__
                // macOS may put the executable file in an application bundle, e.g., "PolyFit.app/Contents/MacOS/PolyFit"
                std::string::size_type pos = log_path.find(".app");
                if (pos != std::string::npos)
                    log_path = log_path.substr(0, pos);
#endif
                log_path = file_system::parent_directory(log_path);
                full_path_log_file = log_path + "/" + file_system::base_name(app_path) + ".log";
            }

            std::string log_file_failure_msg("");
            if (!full_path_log_file.empty()) {
                std::ofstream output(full_path_log_file.c_str(), std::ios::app);
                if (!output.is_open()) {
                    if (log_file != "default" && log_file != "") // error only if requested
                        log_file_failure_msg = "failed to create log file: " + full_path_log_file;

                    // now let's try the current working directory
                    full_path_log_file = file_system::current_working_directory() + "/" + file_system::simple_name(full_path_log_file);
                    output.open(full_path_log_file);
                    if (!output.is_open()) { // if still failed, try the home directory
                        full_path_log_file = file_system::home_directory() + "/" + file_system::simple_name(full_path_log_file);
                        output.open(full_path_log_file);
                    }
                }
                if (output.is_open()) {
                    auto size_bytes = static_cast<std::size_t>(output.tellp());
                    if (size_bytes > 0)
                        output << "\n\n";
                    output << "================================================================= program started ...\n";
                    output.close();
                    log_file_name = full_path_log_file;
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

            defaultConf.set(el::Level::Info, el::ConfigurationType::ToStandardOutput, info_to_stderr ? "true" : "false");

            // If you wish to have a configuration for existing and future loggers, you can use the following.
            // This is useful when you are working on fairly large scale, or using a third-party library that is
            // already using Easylogging++. Any newly created logger will use default configurations. If you wish
            // to configure existing loggers as well, you can set second argument to true (it defaults to false).
            el::Loggers::setDefaultConfigurations(defaultConf, true);

            el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
            el::Loggers::addFlag(el::LoggingFlag::LogDetailedCrashReason);

            // default logger uses default configurations
            el::Loggers::reconfigureLogger("default", defaultConf);

            el::Helpers::setCrashHandler(crash_sandler);

            // allow all levels of verbose messages to be logged into the log file (but not shown on UI).
            el::Loggers::setVerboseLevel(verbosity_threshold);
            VLOG(1) << "executable path: " << file_system::executable_directory();
            VLOG(1) << "current working dir: " << file_system::current_working_directory();

            if (!log_file_failure_msg.empty())
                VLOG(1) << log_file_failure_msg;
        }


        std::string log_file() {
            return log_file_name;
        }


        static Logger* logger = nullptr;

        class Dispatcher : public el::LogDispatchCallback {
        protected:
            void handle(const el::LogDispatchData* data) noexcept override {
                // remove all trailing ending line breaks
                std::string msg = data->logMessage()->message();
                while (msg.back() == '\n' && !msg.empty()) {
                    msg.erase(msg.end() - 1);
                }
                if (logger)
                    logger->send(data->logMessage()->level(), msg);
            }
        };

        Logger::Logger() {
            logger = this;
            ELPP->installLogDispatchCallback<Dispatcher>("Easy3D-Logger");
        }

    }
    // \endcond
}
