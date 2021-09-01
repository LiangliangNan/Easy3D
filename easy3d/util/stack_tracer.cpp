/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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
 ********************************************************************/

#include <easy3d/util/stack_tracer.h>

#include <iomanip>
#include <string>

#include <easy3d/util/file_system.h>
#include <3rd_party/backward/backward.hpp>


#define USE_BACKWARD



    namespace easy3d {

#ifdef USE_BACKWARD

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

#endif

    std::string StackTracer::back_trace_string(int amount, int skip) {
        return to_string(back_trace(amount, skip + 1));
    }


    std::string StackTracer::to_string(const std::vector<StackEntry> &record) {
        std::stringstream stream;
        for (std::size_t i = 0; i < record.size(); ++i) {
            if (!record[i].function_name.empty()) {  // skip the record with unknown function_name
                stream << "\t" << std::setw(3) << std::left << std::setfill(' ') << i
                       << "\t" << std::setw(20) << std::setfill(' ') << record[i].object_name
                       << "\t" << record[i].function_name << "\n";
            }
        }
        return stream.str();
    }
}


#ifndef USE_BACKWARD

#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>

#include <easy3d/util/logging.h>

    namespace easy3d {

        // This implementation is independent of backward.
        // Tested on macOS, should work on Linux, but not Windows.
        std::vector<StackTracer::StackEntry> StackTracer::back_trace(int amount, int skip) {
            std::vector<StackTracer::StackEntry> frames;

            void *callstack[128];
            const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
            int nFrames = backtrace(callstack, nMaxFrames);
            if (amount == 0) {
                return frames;
            } else if (amount > 0) {
                // limit the number of frames
                nFrames = (skip + amount) < nFrames ? skip + amount : nFrames;
            }
            char **symbols = backtrace_symbols(callstack, nFrames);
            for (int i = skip; i < nFrames; i++) {
                // this is the raw data
                // printf("%s\n", symbols[i]);
                Dl_info info;
                if (dladdr(callstack[i], &info)) {
                    int status(0);
                    char *demangled = abi::__cxa_demangle(info.dli_sname, nullptr, 0, &status);
                    const char *name;
                    if (status == 0)
                        name = demangled ? demangled : "(no demangled name)";
                    else
                        name = info.dli_sname ? info.dli_sname : "(no dli_sname)";
                    // the "- skip" makes sure the most recent trace has an index of 0.
                    frames.emplace_back(file_system::base_name(info.dli_fname), name);
                    if (demangled)
                        free(demangled);
                } else {
                    // the "- skip" makes sure the most recent trace has an index of 0.
                    frames.emplace_back("[unknown]", "[unknown]");
                }
            }
            if (symbols)
                free(symbols);
            if (nFrames == nMaxFrames) {
                // the "- skip" makes sure the most recent trace has an index of 0.
                frames.emplace_back("record too long [truncated]", "");
            }
            return frames;
        }
    }

#endif


//    // For Windows
//    int GetStackTrace(void** result, int max_depth, int skip_count) {
//        if (max_depth > 64) {
//            max_depth = 64;
//        }
//        skip_count++;  // we want to skip the current frame as well
//        // This API is thread-safe (moreover it walks only the current thread).
//        return CaptureStackBackTrace(skip_count, max_depth, result, NULL);
//    }
//
//
//    StackTracer::StackTracer() {
//        struct sigaction sa;
//        sa.sa_sigaction = (void (*)(int, siginfo_t *, void *))signal_handler;
//        sigemptyset(&sa.sa_mask);
//        sa.sa_flags = SA_RESTART | SA_SIGINFO;
//        if (sigaction(SIGSEGV, &sa, nullptr) < 0) {
//            perror("DeathHandler - sigaction(SIGSEGV)");
//        }
//        if (sigaction(SIGABRT, &sa, nullptr) < 0) {
//            perror("DeathHandler - sigaction(SIGABBRT)");
//        }
//        if (sigaction(SIGFPE, &sa, nullptr) < 0) {
//            perror("DeathHandler - sigaction(SIGFPE)");
//        }
//    }
//
//
//    StackTracer::~StackTracer() {
//        // Disable alternative signal handler stack
//        stack_t altstack;
//        altstack.ss_sp = nullptr;
//        altstack.ss_size = 0;
//        altstack.ss_flags = SS_DISABLE;
//        sigaltstack(&altstack, nullptr);
//
//        struct sigaction sa;
//
//        sigaction(SIGSEGV, nullptr, &sa);
//        sa.sa_handler = SIG_DFL;
//        sigaction(SIGSEGV, &sa, nullptr);
//
//        sigaction(SIGABRT, nullptr, &sa);
//        sa.sa_handler = SIG_DFL;
//        sigaction(SIGABRT, &sa, nullptr);
//
//        sigaction(SIGFPE, nullptr, &sa);
//        sa.sa_handler = SIG_DFL;
//        sigaction(SIGFPE, &sa, nullptr);
//    }
//
//
//    void StackTracer::signal_handler(int sig, void *info, void *secret) {
//        // Stop all other running threads by forking
//        pid_t forkedPid = fork();
//        if (forkedPid != 0) {
//            int status;
//            // Freeze the original process, until it's child prints the stack trace
//            kill(getpid(), SIGSTOP);
//            // Wait for the child without blocking and exit as soon as possible,
//            // so that no zombies are left.
//            waitpid(forkedPid, &status, WNOHANG);
//
//#ifdef QUICK_EXIT
//            if (quick_exit_) {
//              ::quick_exit(EXIT_FAILURE);
//            }
//#endif
//            exit(EXIT_FAILURE);
//        }
//
//        std::string log = "==========================================\n";
//        switch (sig) {
//            case SIGSEGV:
//                log = "Fatal error: segmentation fault";
//                break;
//            case SIGABRT:
//                log = "Fatal error: aborted";
//                break;
//            case SIGFPE:
//                log = "Fatal error: floating point exception";
//                break;
//            default:
//                log = "Fatal error with signal: " + std::to_string(sig);
//                break;
//        }
//
//        // skip the last function call, i.e., back_trace() itself.
//        LOG(INFO) << log << "\n" << back_trace(1).c_str();
//
//        // Resume the parent process
//        kill(getppid(), SIGCONT);
//
//        // This is called in the child process
//        _Exit(EXIT_SUCCESS);
//    }