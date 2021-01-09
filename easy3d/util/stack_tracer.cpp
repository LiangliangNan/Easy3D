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

#include <easy3d/util/stack_tracer.h>

#include <iomanip>
#include <string>

#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include <3rd_party/backward/backward.hpp>


namespace easy3d {


    StackTracer::StackTracer() {
        static backward::SignalHandling sh;

        // Setup the failure function.
        // This function writes the stack trace record \p st as a FATAL error into the log file and stderr.
        // This failure function will be called when the program encountered an error and has to abort.

        // st was captured in backward's SignalHandling method
        sh.signal_log_func = [](backward::StackTrace* st, int skip, const char* comment) ->void {
            if (backward::SignalHandling::failure_has_been_recored)
                return;

            backward::StackTrace& stack = *st;
            backward::TraceResolver resolver;
            resolver.load_stacktrace(stack);

            std::vector<StackTracer::StackEntry> record;
            for (size_t trace_idx = skip; trace_idx < stack.size(); ++trace_idx) {
                const auto trace = resolver.resolve(stack[trace_idx]);
                record.emplace_back(
                        file_system::base_name(trace.object_filename), // get the short name
                        trace.object_function
                );
            }

            std::stringstream stream;
            stream << "\n================================================================================="
                   << "\nEasy3D has encountered a fatal error and has to abort. ";

            const std::string log_file = logging::FileLogClient::log_file_name();
            if (!log_file.empty()) { // a log file exists
                stream << "The error has been recorded \n"
                       << "in the log file [" + file_system::absolute_path(log_file) + "].";
            }

            stream << "\nPlease report this issue with the complete log, a description of how to reproduce";
            stream << "\nthe issue, and possibly your data to Liangliang Nan (liangliang.nan@gmail.com).";
            stream << "\n=================================================================================";
            stream << "\nStack trace (most recent call first):";
            stream << "\n" << StackTracer::to_string(record);

            backward::SignalHandling::failure_has_been_recored = true;

            // log this failure as a fatal error
            LOG(FATAL) << (comment ? std::string(comment) : "A fatal error has occurred") << stream.str();
        };
    }


    StackTracer::~StackTracer() {
    }


    std::vector<StackTracer::StackEntry> StackTracer::back_trace(int amount, int skip) {
        backward::StackTrace st;
        st.load_here(amount + skip);

        backward::TraceResolver tr;
        tr.load_stacktrace(st);

        std::vector<StackTracer::StackEntry> record;
        for (size_t i = skip; i < st.size() && i < (skip + amount); ++i) {
            const auto trace = tr.resolve(st[i]);
            record.emplace_back(
                    file_system::base_name(trace.object_filename), // get the short name
                    trace.object_function
            );
        }
        return record;
    }


    std::string StackTracer::back_trace_string(int amount, int skip) {
        return to_string(back_trace(amount, skip + 1));
    }


    std::string StackTracer::to_string(const std::vector<StackEntry>& record) {
        std::stringstream stream;
        for (std::size_t i=0; i<record.size(); ++i) {
            stream << "\t" << std::setw(5) << std::left << std::setfill(' ') << i
                   << "\t" << std::setw(20) << std::setfill(' ') << record[i].object_name
                   << "\t" << record[i].function_name << "\n";
        }
        return stream.str();
    }

}
