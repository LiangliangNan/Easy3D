//
//  Portable File Dialogs
//
//  Copyright © 2018—2019 Sam Hocevar <sam@hocevar.net>
//
//  This library is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#pragma once

#if _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>
#include <future>
#else
#ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 2 // for popen()
#endif
#include <cstdio>   // for popen()
#include <cstdlib>  // for std::getenv()
#include <fcntl.h>  // for fcntl()
#include <unistd.h> // for read()
#endif

#include <string>
#include <memory>
#include <iostream>
#include <map>
#include <regex>
#include <thread>
#include <chrono>

namespace pfd
{

enum class button
{
    cancel = -1,
    ok,
    yes,
    no,
    abort,
    retry,
    ignore,
};

enum class choice
{
    ok = 0,
    ok_cancel,
    yes_no,
    yes_no_cancel,
    retry_cancel,
    abort_retry_ignore,
};

enum class icon
{
    info = 0,
    warning,
    error,
    question,
};

// Process wait timeout, in milliseconds
static int const default_wait_timeout = 20;

// The settings class, only exposing to the user a way to set verbose mode
// and to force a rescan of installed desktop helpers (zenity, kdialog…).
class settings
{
public:
    static void verbose(bool value)
    {
        settings().flags(flag::is_verbose) = value;
    }

    static void rescan()
    {
        settings(true);
    }

protected:
    enum class flag
    {
        is_scanned = 0,
        is_verbose,

        has_zenity,
        has_matedialog,
        has_qarma,
        has_kdialog,
        is_vista,

        max_flag,
    };

    explicit settings(bool resync = false)
    {
        flags(flag::is_scanned) &= !resync;
    }

    inline bool is_osascript() const
    {
#if __APPLE__
        return true;
#else
        return false;
#endif
    }

    inline bool is_zenity() const
    {
        return flags(flag::has_zenity) ||
               flags(flag::has_matedialog) ||
               flags(flag::has_qarma);
    }

    inline bool is_kdialog() const
    {
        return flags(flag::has_kdialog);
    }

    // Static array of flags for internal state
    bool const &flags(flag in_flag) const
    {
        static bool flags[size_t(flag::max_flag)];
        return flags[size_t(in_flag)];
    }

    // Non-const getter for the static array of flags
    bool &flags(flag in_flag)
    {
        return const_cast<bool &>(static_cast<const settings *>(this)->flags(in_flag));
    }
};

// Forward declarations for our API
class notify;
class message;

// Internal classes, not to be used by client applications
namespace internal
{

#if _WIN32
static inline std::wstring str2wstr(std::string const &str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring ret(len, '\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), (LPWSTR)ret.data(), (int)ret.size());
    return ret;
}

static inline std::string wstr2str(std::wstring const &str)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0, nullptr, nullptr);
    std::string ret(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), (LPSTR)ret.data(), (int)ret.size(), nullptr, nullptr);
    return ret;
}

static inline bool is_vista()
{
    OSVERSIONINFOEXW osvi;
    memset(&osvi, 0, sizeof(osvi));
    DWORDLONG const mask = VerSetConditionMask(
            VerSetConditionMask(
                    VerSetConditionMask(
                            0, VER_MAJORVERSION, VER_GREATER_EQUAL),
                    VER_MINORVERSION, VER_GREATER_EQUAL),
            VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_VISTA);
    osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_VISTA);
    osvi.wServicePackMajor = 0;

    return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, mask) != FALSE;
}
#endif

// This is necessary until C++20 which will have std::string::ends_with() etc.
static inline bool ends_with(std::string const &str, std::string const &suffix)
{
    return suffix.size() <= str.size() &&
        str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static inline bool starts_with(std::string const &str, std::string const &prefix)
{
    return prefix.size() <= str.size() &&
        str.compare(0, prefix.size(), prefix) == 0;
}

class executor
{
    friend class dialog;

public:
    // High level function to get the result of a command
    std::string result(int *exit_code = nullptr)
    {
        stop();
        if (exit_code)
            *exit_code = m_exit_code;
        return m_stdout;
    }

#if _WIN32
    void start(std::function<std::string(int *)> const &fun)
    {
        stop();
        m_future = std::async(fun, &m_exit_code);
        m_running = true;
    }
#endif

    void start(std::string const &command)
    {
        stop();
        m_stdout.clear();
        m_exit_code = -1;

#if _WIN32
        STARTUPINFOW si;

        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        std::wstring wcommand = str2wstr(command);
        if (!CreateProcessW(nullptr, (LPWSTR)wcommand.c_str(), nullptr, nullptr,
                            FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &m_pi))
            return; /* TODO: GetLastError()? */
        WaitForInputIdle(m_pi.hProcess, INFINITE);
#elif __EMSCRIPTEN__ || __NX__
        // FIXME: do something
#else
        m_stream = popen((command + " 2>/dev/null").c_str(), "r");
        if (!m_stream)
            return;
        m_fd = fileno(m_stream);
        fcntl(m_fd, F_SETFL, O_NONBLOCK);
#endif
        m_running = true;
    }

    ~executor()
    {
        stop();
    }

protected:
    bool ready(int timeout = default_wait_timeout)
    {
        if (!m_running)
            return true;

#if _WIN32
        if (m_future.valid())
        {
            auto status = m_future.wait_for(std::chrono::milliseconds(timeout));
            if (status != std::future_status::ready)
                return false;

            m_stdout = m_future.get();
        }
        else
        {
            if (WaitForSingleObject(m_pi.hProcess, timeout) == WAIT_TIMEOUT)
                return false;

            DWORD ret;
            GetExitCodeProcess(m_pi.hProcess, &ret);
            m_exit_code = (int)ret;
            CloseHandle(m_pi.hThread);
            CloseHandle(m_pi.hProcess);
        }
#elif __EMSCRIPTEN__ || __NX__
        // FIXME: do something
#else
        char buf[BUFSIZ];
        ssize_t received = read(m_fd, buf, BUFSIZ - 1);
        if (received == -1 && errno == EAGAIN)
        {
            // FIXME: this happens almost always at first iteration
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
            return false;
        }
        if (received > 0)
        {
            m_stdout += std::string(buf, received);
            return false;
        }
        m_exit_code = pclose(m_stream);
#endif

        m_running = false;
        return true;
    }

    void stop()
    {
        // Loop until the user closes the dialog
        while (!ready())
        {
#if _WIN32
            // On Windows, we need to run the message pump. If the async
            // thread uses a Windows API dialog, it may be attached to the
            // main thread and waiting for messages that only we can dispatch.
            MSG msg;
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
#endif
        }
    }

private:
    bool m_running = false;
    std::string m_stdout;
    int m_exit_code = -1;
#if _WIN32
    std::future<std::string> m_future;
    PROCESS_INFORMATION m_pi;
#else
    FILE *m_stream = nullptr;
    int m_fd = -1;
#endif
};

class platform
{
protected:
#if _WIN32
    // Helper class around LoadLibraryA() and GetProcAddress() with some safety
    class dll
    {
    public:
        dll(std::string const &name)
          : handle(::LoadLibraryA(name.c_str()))
        {}

        ~dll()
        {
            if (handle)
                ::FreeLibrary(handle);
        }

        template<typename T> class proc
        {
        public:
            proc(dll const &lib, std::string const &sym)
              : m_proc(reinterpret_cast<T *>(::GetProcAddress(lib.handle, sym.c_str())))
            {}

            operator bool() const
            {
                return m_proc != nullptr;
            }

            operator T *() const
            {
                return m_proc;
            }

        private:
            T *m_proc;
        };

    private:
        HMODULE handle;
    };

    // Helper class around CreateActCtx() and ActivateActCtx()
    class new_style_context
    {
    public:
        new_style_context()
        {
            // Only create one activation context for the whole app lifetime.
            static HANDLE hctx = create();

            if (hctx != INVALID_HANDLE_VALUE)
                ActivateActCtx(hctx, &m_cookie);
        }

        ~new_style_context()
        {
            DeactivateActCtx(0, m_cookie);
        }

    private:
        HANDLE create()
        {
            // This “hack” seems to be necessary for this code to work on windows XP.
            // Without it, dialogs do not show and close immediately. GetError()
            // returns 0 so I don’t know what causes this. I was not able to reproduce
            // this behavior on Windows 7 and 10 but just in case, let it be here for
            // those versions too.
            // This hack is not required if other dialogs are used (they load comdlg32
            // automatically), only if message boxes are used.
            dll comdlg32("comdlg32.dll");

            // Using approach as shown here: https://stackoverflow.com/a/10444161
            UINT len = ::GetSystemDirectoryA(nullptr, 0);
            std::string sys_dir(len, '\0');
            ::GetSystemDirectoryA(&sys_dir[0], len);

            ACTCTXA act_ctx =
            {
                // Do not set flag ACTCTX_FLAG_SET_PROCESS_DEFAULT, since it causes a
                // crash with error “default context is already set”.
                sizeof(act_ctx),
                ACTCTX_FLAG_RESOURCE_NAME_VALID | ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID,
                "shell32.dll", 0, 0, sys_dir.c_str(), (LPCSTR)124,
            };

            return ::CreateActCtxA(&act_ctx);
        }

        ULONG_PTR m_cookie = 0;
    };
#endif
};

class dialog : protected settings, protected platform
{
    friend class pfd::notify;
    friend class pfd::message;

public:
    bool ready(int timeout = default_wait_timeout)
    {
        return m_async->ready(timeout);
    }

protected:
    explicit dialog()
      : m_async(std::make_shared<executor>())
    {
        if (!flags(flag::is_scanned))
        {
#if _WIN32
            flags(flag::is_vista) = is_vista();
#elif !__APPLE__
            flags(flag::has_zenity) = check_program("zenity");
            flags(flag::has_matedialog) = check_program("matedialog");
            flags(flag::has_qarma) = check_program("qarma");
            flags(flag::has_kdialog) = check_program("kdialog");

            // If multiple helpers are available, try to default to the best one
            if (flags(flag::has_zenity) && flags(flag::has_kdialog))
            {
                auto desktop_name = std::getenv("XDG_SESSION_DESKTOP");
                if (desktop_name && desktop_name == std::string("gnome"))
                    flags(flag::has_kdialog) = false;
                else if (desktop_name && desktop_name == std::string("KDE"))
                    flags(flag::has_zenity) = false;
            }
#endif
            flags(flag::is_scanned) = true;
        }
    }

    std::string desktop_helper() const
    {
#if __APPLE__
        return "osascript";
#else
        return flags(flag::has_zenity) ? "zenity"
             : flags(flag::has_matedialog) ? "matedialog"
             : flags(flag::has_qarma) ? "qarma"
             : flags(flag::has_kdialog) ? "kdialog"
             : "echo";
#endif
    }

    std::string buttons_to_name(choice choice) const
    {
        switch (choice)
        {
            case choice::ok_cancel: return "okcancel";
            case choice::yes_no: return "yesno";
            case choice::yes_no_cancel: return "yesnocancel";
            case choice::retry_cancel: return "retrycancel";
            case choice::abort_retry_ignore: return "abortretryignore";
            /* case choice::ok: */ default: return "ok";
        }
    }

    std::string get_icon_name(icon icon) const
    {
        switch (icon)
        {
            case icon::warning: return "warning";
            case icon::error: return "error";
            case icon::question: return "question";
            // Zenity wants "information" but WinForms wants "info"
            /* case icon::info: */ default:
#if _WIN32
                return "info";
#else
                return "information";
#endif
        }
    }

    // Properly quote a string for Powershell: replace ' or " with '' or ""
    // FIXME: we should probably get rid of newlines!
    // FIXME: the \" sequence seems unsafe, too!
    std::string powershell_quote(std::string const &str) const
    {
        return "'" + std::regex_replace(str, std::regex("['\"]"), "$&$&") + "'";
    }

    // Properly quote a string for osascript: replace ' with '\'' and \ or " with \\ or \"
    std::string osascript_quote(std::string const &str) const
    {
        return "\"" + std::regex_replace(std::regex_replace(str,
                          std::regex("[\\\\\"]"), "\\$&"), std::regex("'"), "'\\''") + "\"";
    }

    // Properly quote a string for the shell: just replace ' with '\''
    std::string shell_quote(std::string const &str) const
    {
        return "'" + std::regex_replace(str, std::regex("'"), "'\\''") + "'";
    }

    // Check whether a program is present using “which”.
    bool check_program(std::string const &program)
    {
#if _WIN32
        (void)program;
        return false;
#else
        int exit_code = -1;
        m_async->start("which " + program + " 2>/dev/null");
        m_async->result(&exit_code);
        return exit_code == 0;
#endif
    }

protected:
    // Keep handle to executing command
    std::shared_ptr<executor> m_async;
};

class file_dialog : public dialog
{
protected:
    enum type
    {
        open,
        save,
        folder,
    };

    file_dialog(type in_type,
                std::string const &title,
                std::string const &default_path = "",
                std::vector<std::string> filters = {},
                bool allow_multiselect = false,
                bool confirm_overwrite = true)
    {
#if _WIN32
        std::string filter_list;
        std::regex whitespace("  *");
        for (size_t i = 0; i + 1 < filters.size(); i += 2)
        {
            filter_list += filters[i] + '\0';
            filter_list += std::regex_replace(filters[i + 1], whitespace, ";") + '\0';
        }
        filter_list += '\0';

        m_async->start([this, in_type, title, default_path, filter_list,
                        allow_multiselect, confirm_overwrite](int *exit_code) -> std::string
        {
            (void)exit_code;
            m_wtitle = internal::str2wstr(title);
            m_wdefault_path = internal::str2wstr(default_path);
            auto wfilter_list = internal::str2wstr(filter_list);

            // Folder selection uses a different method
            if (in_type == type::folder)
            {
                dll ole32("ole32.dll");

                auto status = dll::proc<HRESULT WINAPI (LPVOID, DWORD)>(ole32, "CoInitializeEx")
                                  (nullptr, COINIT_APARTMENTTHREADED);
                if (flags(flag::is_vista))
                {
                    // On Vista and higher we should be able to use IFileDialog for folder selection
                    IFileDialog *ifd;
                    HRESULT hr = dll::proc<HRESULT WINAPI (REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID *)>(ole32, "CoCreateInstance")
                                     (CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&ifd));

                    // In case CoCreateInstance fails (which it should not), try legacy approach
                    if (SUCCEEDED(hr))
                        return select_folder_vista(ifd);
                }

                BROWSEINFOW bi;
                memset(&bi, 0, sizeof(bi));

                bi.lpfn = &bffcallback;
                bi.lParam = (LPARAM)this;

                if (flags(flag::is_vista))
                {
                    // This hangs on Windows XP, as reported here:
                    // https://github.com/samhocevar/portable-file-dialogs/pull/21
                    if (status == S_OK)
                        bi.ulFlags |= BIF_NEWDIALOGSTYLE;
                    bi.ulFlags |= BIF_EDITBOX;
                    bi.ulFlags |= BIF_STATUSTEXT;
                }

                auto *list = SHBrowseForFolderW(&bi);
                std::string ret;
                if (list)
                {
                    auto buffer = new wchar_t[MAX_PATH];
                    SHGetPathFromIDListW(list, buffer);
                    dll::proc<void WINAPI (LPVOID)>(ole32, "CoTaskMemFree")(list);
                    ret = internal::wstr2str(buffer);
                    delete[] buffer;
                }
                if (status == S_OK)
                    dll::proc<void WINAPI ()>(ole32, "CoUninitialize")();
                return ret;
            }

            OPENFILENAMEW ofn;
            memset(&ofn, 0, sizeof(ofn));
            ofn.lStructSize = sizeof(OPENFILENAMEW);
            ofn.hwndOwner = GetForegroundWindow();

            ofn.lpstrFilter = wfilter_list.c_str();

            auto woutput = std::wstring(MAX_PATH * 256, L'\0');
            ofn.lpstrFile = (LPWSTR)woutput.data();
            ofn.nMaxFile = (DWORD)woutput.size();
            if (!m_wdefault_path.empty())
            {
                // If a directory was provided, use it as the initial directory. If
                // a valid path was provided, use it as the initial file. Otherwise,
                // let the Windows API decide.
                auto path_attr = GetFileAttributesW(m_wdefault_path.c_str());
                if (path_attr != INVALID_FILE_ATTRIBUTES && (path_attr & FILE_ATTRIBUTE_DIRECTORY))
                    ofn.lpstrInitialDir = m_wdefault_path.c_str();
                else if (m_wdefault_path.size() <= woutput.size())
                    lstrcpyW(ofn.lpstrFile, m_wdefault_path.c_str());
                else
                {
                    ofn.lpstrFileTitle = (LPWSTR)m_wdefault_path.data();
                    ofn.nMaxFileTitle = (DWORD)m_wdefault_path.size();
                }
            }
            ofn.lpstrTitle = m_wtitle.c_str();
            ofn.Flags = OFN_NOCHANGEDIR | OFN_EXPLORER;

            dll comdlg32("comdlg32.dll");

            if (in_type == type::save)
            {
                if (!confirm_overwrite)
					ofn.Flags |= OFN_OVERWRITEPROMPT;

                // using set context to apply new visual style (required for windows XP)
                new_style_context ctx;

                dll::proc<BOOL WINAPI (LPOPENFILENAMEW)> get_save_file_name(comdlg32, "GetSaveFileNameW");
                if (get_save_file_name(&ofn) == 0)
                    return "";
                return internal::wstr2str(woutput.c_str());
            }

            if (allow_multiselect)
                ofn.Flags |= OFN_ALLOWMULTISELECT;
            ofn.Flags |= OFN_PATHMUSTEXIST;

            // using set context to apply new visual style (required for windows XP)
            new_style_context ctx;

            dll::proc<BOOL WINAPI (LPOPENFILENAMEW)> get_open_file_name(comdlg32, "GetOpenFileNameW");
            if (get_open_file_name(&ofn) == 0)
                return "";

            std::string prefix;
            for (wchar_t const *p = woutput.c_str(); *p; )
            {
                auto filename = internal::wstr2str(p);
                p += filename.size();
                // In multiselect mode, we advance p one step more and
                // check for another filename. If there is one and the
                // prefix is empty, it means we just read the prefix.
                if (allow_multiselect && *++p && prefix.empty())
                {
                    prefix = filename + "/";
                    continue;
                }

                m_vector_result.push_back(prefix + filename);
            }

            return "";
        });
#else
        auto command = desktop_helper();

        if (is_osascript())
        {
            command += " -e 'set ret to choose";
            switch (in_type)
            {
                case type::save:
                    command += " file name";
                    break;
                case type::open: default:
                    command += " file";
                    if (allow_multiselect)
                        command += " with multiple selections allowed";
                    break;
                case type::folder:
                    command += " folder";
                    break;
            }

#if 0

            if (default_path.size())
                command += " default location " + osascript_quote(default_path);

#else       // Liangliang: make it to have a similar behavior as Windows.
            // If a directory was provided, use it as the initial directory. If
            // a valid path was provided, use it as the initial file. Otherwise,
            // let macOS decide.

            static const char* const PATH_SEPARATORS = "/\\";
            std::string name = default_path; // file name without path
            std::string::size_type slash = default_path.find_last_of(PATH_SEPARATORS);
            if (slash != std::string::npos)
                name = std::string(default_path.begin() + slash + 1, default_path.end());

            if (default_path.size()) {
                if (in_type == type::save)
                    command += " default name " + osascript_quote(name);
                else
                    command += " default location " + osascript_quote(default_path);
            }
#endif

            command += " with prompt " + osascript_quote(title);

            if (in_type == type::open)
            {
                // Concatenate all user-provided filter patterns
                std::string patterns;
                for (size_t i = 0; i < filters.size() / 2; ++i)
                    patterns += " " + filters[2 * i + 1];

                // Split the pattern list to check whether "*" is in there; if it
                // is, we have to disable filters because there is no mechanism in
                // OS X for the user to override the filter.
                std::regex sep("\\s+");
                std::string filter_list;
                bool has_filter = true;
                std::sregex_token_iterator iter(patterns.begin(), patterns.end(), sep, -1);
                std::sregex_token_iterator end;
                for ( ; iter != end; ++iter)
                {
                    auto pat = iter->str();
#if 0 // Liangliang: no need to disable the filters if there is "*.*" or "*"
                    if (pat == "*" || pat == "*.*")
                        has_filter = false;
                    else
#endif
                        if (internal::starts_with(pat, "*."))
                        filter_list += (filter_list.size() == 0 ? "" : ",") +
                                       osascript_quote(pat.substr(2, pat.size() - 2));
                }
                if (has_filter && filter_list.size() > 0)
                    command += " of type {" + filter_list + "}";
            }

            if (in_type == type::open && allow_multiselect)
            {
                command += "\nset s to \"\"";
                command += "\nrepeat with i in ret";
                command += "\n  set s to s & (POSIX path of i) & \"\\n\"";
                command += "\nend repeat";
                command += "\ncopy s to stdout'";
            }
            else
            {
                command += "\nPOSIX path of ret'";
            }
        }
        else if (is_zenity())
        {
            command += " --file-selection --filename=" + shell_quote(default_path)
                     + " --title " + shell_quote(title)
                     + " --separator='\n'";

            for (size_t i = 0; i < filters.size() / 2; ++i)
                command += " --file-filter " + shell_quote(filters[2 * i] + "|" + filters[2 * i + 1]);

            if (in_type == type::save)
                command += " --save";
            if (in_type == type::folder)
                command += " --directory";
            if (confirm_overwrite)
                command += " --confirm-overwrite";
            if (allow_multiselect)
                command += " --multiple";
        }
        else if (is_kdialog())
        {
            switch (in_type)
            {
                case type::save: command += " --getsavefilename"; break;
                case type::open: command += " --getopenfilename"; break;
                case type::folder: command += " --getexistingdirectory"; break;
            }
            command += " " + shell_quote(default_path);

            std::string filter;
            for (size_t i = 0; i < filters.size() / 2; ++i)
                filter += (i == 0 ? "" : " | ") + filters[2 * i] + "(" + filters[2 * i + 1] + ")";
            command += " " + shell_quote(filter);

            command += " --title " + shell_quote(title);
        }

        if (flags(flag::is_verbose))
            std::cerr << "pfd: " << command << std::endl;

        m_async->start(command);
#endif
    }

protected:
    std::string string_result()
    {
#if _WIN32
        return m_async->result();
#else
        // Strip the newline character
        auto ret = m_async->result();
        return ret.back() == '\n' ? ret.substr(0, ret.size() - 1) : ret;
#endif
    }

    std::vector<std::string> vector_result()
    {
#if _WIN32
        m_async->result();
        return m_vector_result;
#else
        std::vector<std::string> ret;
        auto result = m_async->result();
        for (;;)
        {
            // Split result along newline characters
            auto i = result.find('\n');
            if (i == 0 || i == std::string::npos)
                break;
            ret.push_back(result.substr(0, i));
            result = result.substr(i + 1, result.size());
        }
        return ret;
#endif
    }

#if _WIN32
    // Use a static function to pass as BFFCALLBACK for legacy folder select
    static int CALLBACK bffcallback(HWND hwnd, UINT uMsg, LPARAM, LPARAM pData)
    {
        auto inst = (file_dialog *)pData;
        switch (uMsg)
        {
            case BFFM_INITIALIZED:
                SendMessage(hwnd, BFFM_SETSELECTIONW, TRUE, (LPARAM)inst->m_wdefault_path.c_str());
                break;
        }
        return 0;
    }

    std::string select_folder_vista(IFileDialog *ifd)
    {
        std::string result;

        IShellItem *folder;

        // Load library at runtime so app doesn't link it at load time (which will fail on windows XP)
        dll shell32("shell32.dll");
        dll::proc<HRESULT WINAPI (PCWSTR, IBindCtx*, REFIID, void**)>
            create_item(shell32, "SHCreateItemFromParsingName");

        if (!create_item)
            return "";

        auto hr = create_item(m_wdefault_path.c_str(),
                              nullptr,
                              IID_PPV_ARGS(&folder));

        // Set default folder if found. This only sets the default folder. If
        // Windows has any info about the most recently selected folder, it
        // will display it instead. Generally, calling SetFolder() to set the
        // current directory “is not a good or expected user experience and
        // should therefore be avoided”:
        // https://docs.microsoft.com/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifiledialog-setfolder
        if (SUCCEEDED(hr))
        {
            ifd->SetDefaultFolder(folder);
            folder->Release();
        }

        // Set the dialog title and option to select folders
        ifd->SetOptions(FOS_PICKFOLDERS);
        ifd->SetTitle(m_wtitle.c_str());

        hr = ifd->Show(GetForegroundWindow());
        if (SUCCEEDED(hr))
        {
            IShellItem* item;
            hr = ifd->GetResult(&item);
            if (SUCCEEDED(hr))
            {
                wchar_t* wselected = nullptr;
                item->GetDisplayName(SIGDN_FILESYSPATH, &wselected);
                item->Release();

                if (wselected)
                {
                    result = internal::wstr2str(std::wstring(wselected));
                    dll ole32("ole32.dll");
                    dll::proc<void WINAPI (LPVOID)>(ole32, "CoTaskMemFree")(wselected);
                }
            }
        }

        ifd->Release();

        return result;
    }

    std::wstring m_wtitle;
    std::wstring m_wdefault_path;

    std::vector<std::string> m_vector_result;
#endif
};

} // namespace internal

class notify : public internal::dialog
{
public:
    notify(std::string const &title,
           std::string const &message,
           icon icon = icon::info)
    {
        if (icon == icon::question) // Not supported by notifications
            icon = icon::info;

#if _WIN32
        // Use a static shared pointer for notify_icon so that we can delete
        // it whenever we need to display a new one, and we can also wait
        // until the program has finished running.
        struct notify_icon_data : public NOTIFYICONDATAW
        {
            ~notify_icon_data() { Shell_NotifyIconW(NIM_DELETE, this); }
        };

        static std::shared_ptr<notify_icon_data> nid;

        // Release the previous notification icon, if any, and allocate a new
        // one. Note that std::make_shared() does value initialization, so there
        // is no need to memset the structure.
        nid = nullptr;
        nid = std::make_shared<notify_icon_data>();

        // For XP support
        nid->cbSize = NOTIFYICONDATAW_V2_SIZE;
        nid->hWnd = nullptr;
        nid->uID = 0;

        // Flag Description:
        // - NIF_ICON    The hIcon member is valid.
        // - NIF_MESSAGE The uCallbackMessage member is valid.
        // - NIF_TIP     The szTip member is valid.
        // - NIF_STATE   The dwState and dwStateMask members are valid.
        // - NIF_INFO    Use a balloon ToolTip instead of a standard ToolTip. The szInfo, uTimeout, szInfoTitle, and dwInfoFlags members are valid.
        // - NIF_GUID    Reserved.
        nid->uFlags = NIF_MESSAGE | NIF_ICON | NIF_INFO;

        // Flag Description
        // - NIIF_ERROR     An error icon.
        // - NIIF_INFO      An information icon.
        // - NIIF_NONE      No icon.
        // - NIIF_WARNING   A warning icon.
        // - NIIF_ICON_MASK Version 6.0. Reserved.
        // - NIIF_NOSOUND   Version 6.0. Do not play the associated sound. Applies only to balloon ToolTips
        switch (icon)
        {
            case icon::warning: nid->dwInfoFlags = NIIF_WARNING; break;
            case icon::error: nid->dwInfoFlags = NIIF_ERROR; break;
            /* case icon::info: */ default: nid->dwInfoFlags = NIIF_INFO; break;
        }

        ENUMRESNAMEPROC icon_enum_callback = [](HMODULE, LPCTSTR, LPTSTR lpName, LONG_PTR lParam) -> BOOL
        {
            ((NOTIFYICONDATAW *)lParam)->hIcon = ::LoadIcon(GetModuleHandle(nullptr), lpName);
            return false;
        };

        nid->hIcon = ::LoadIcon(nullptr, IDI_APPLICATION);
        ::EnumResourceNames(nullptr, RT_GROUP_ICON, icon_enum_callback, (LONG_PTR)nid.get());

        nid->uTimeout = 5000;

        // FIXME check buffer length
        lstrcpyW(nid->szInfoTitle, internal::str2wstr(title).c_str());
        lstrcpyW(nid->szInfo, internal::str2wstr(message).c_str());

        // Display the new icon
        Shell_NotifyIconW(NIM_ADD, nid.get());
#else
        auto command = desktop_helper();

        if (is_osascript())
        {
            command += " -e 'display notification " + osascript_quote(message) +
                       "     with title " + osascript_quote(title) + "'";
        }
        else if (is_zenity())
        {
            command += " --notification"
                       " --window-icon " + get_icon_name(icon) +
                       " --text " + shell_quote(title + "\n" + message);
        }
        else if (is_kdialog())
        {
            command += " --icon " + get_icon_name(icon) +
                       " --title " + shell_quote(title) +
                       " --passivepopup " + shell_quote(message) +
                       " 5";
        }

        if (flags(flag::is_verbose))
            std::cerr << "pfd: " << command << std::endl;

        m_async->start(command);
#endif
    }
};

class message : public internal::dialog
{
public:
    message(std::string const &title,
            std::string const &text,
            choice choice = choice::ok_cancel,
            icon icon = icon::info)
    {
#if _WIN32
        UINT style = MB_TOPMOST;
        switch (icon)
        {
            case icon::warning: style |= MB_ICONWARNING; break;
            case icon::error: style |= MB_ICONERROR; break;
            case icon::question: style |= MB_ICONQUESTION; break;
            /* case icon::info: */ default: style |= MB_ICONINFORMATION; break;
        }

        switch (choice)
        {
            case choice::ok_cancel: style |= MB_OKCANCEL; break;
            case choice::yes_no: style |= MB_YESNO; break;
            case choice::yes_no_cancel: style |= MB_YESNOCANCEL; break;
            case choice::retry_cancel: style |= MB_RETRYCANCEL; break;
            case choice::abort_retry_ignore: style |= MB_ABORTRETRYIGNORE; break;
            /* case choice::ok: */ default: style |= MB_OK; break;
        }

        m_mappings[IDCANCEL] = button::cancel;
        m_mappings[IDOK] = button::ok;
        m_mappings[IDYES] = button::yes;
        m_mappings[IDNO] = button::no;
        m_mappings[IDABORT] = button::abort;
        m_mappings[IDRETRY] = button::retry;
        m_mappings[IDIGNORE] = button::ignore;

        m_async->start([text, title, style](int *exit_code) -> std::string
        {
            auto wtext = internal::str2wstr(text);
            auto wtitle = internal::str2wstr(title);
            // using set context to apply new visual style (required for all windows versions)
            new_style_context ctx;
            *exit_code = MessageBoxW(GetForegroundWindow(), wtext.c_str(), wtitle.c_str(), style);
            return "";
        });
#else
        auto command = desktop_helper();

        if (is_osascript())
        {
            command += " -e 'display dialog " + osascript_quote(text) +
                       "     with title " + osascript_quote(title);
            switch (choice)
            {
                case choice::ok_cancel:
                    command += "buttons {\"OK\", \"Cancel\"} "
                               "default button \"OK\" "
                               "cancel button \"Cancel\"";
                    m_mappings[256] = button::cancel;
                    break;
                case choice::yes_no:
                    command += "buttons {\"Yes\", \"No\"} "
                               "default button \"Yes\" "
                               "cancel button \"No\"";
                    m_mappings[256] = button::no;
                    break;
                case choice::yes_no_cancel:
                    command += "buttons {\"Yes\", \"No\", \"Cancel\"} "
                               "default button \"Yes\" "
                               "cancel button \"Cancel\"";
                    m_mappings[256] = button::cancel;
                    break;
                case choice::retry_cancel:
                    command += "buttons {\"Retry\", \"Cancel\"} "
                        "default button \"Retry\" "
                        "cancel button \"Cancel\"";
                    m_mappings[256] = button::cancel;
                    break;
                case choice::abort_retry_ignore:
                    command += "buttons {\"Abort\", \"Retry\", \"Ignore\"} "
                        "default button \"Retry\" "
                        "cancel button \"Retry\"";
                    m_mappings[256] = button::cancel;
                    break;
                case choice::ok: default:
                    command += "buttons {\"OK\"} "
                               "default button \"OK\" "
                               "cancel button \"OK\"";
                    m_mappings[256] = button::ok;
                    break;
            }
            command += " with icon ";
            switch (icon)
            {
                #define PFD_OSX_ICON(n) "alias ((path to library folder from system domain) as text " \
                    "& \"CoreServices:CoreTypes.bundle:Contents:Resources:" n ".icns\")"
                case icon::info: default: command += PFD_OSX_ICON("ToolBarInfo"); break;
                case icon::warning: command += "caution"; break;
                case icon::error: command += "stop"; break;
                case icon::question: command += PFD_OSX_ICON("GenericQuestionMarkIcon"); break;
                #undef PFD_OSX_ICON
            }
            command += "'";
        }
        else if (is_zenity())
        {
            switch (choice)
            {
                case choice::ok_cancel:
                    command += " --question --ok-label=OK --cancel-label=Cancel"; break;
                case choice::yes_no:
                    // Do not use standard --question because it causes “No” to return -1,
                    // which is inconsistent with the “Yes/No/Cancel” mode below.
                    command += " --question --switch --extra-button No --extra-button Yes"; break;
                case choice::yes_no_cancel:
                    command += " --question --switch --extra-button No --extra-button Yes --extra-button Cancel"; break;
                case choice::retry_cancel:
                    command += " --question --switch --extra-button Retry --extra-button Cancel"; break;
                case choice::abort_retry_ignore:
                    command += " --question --switch --extra-button Abort --extra-button Retry --extra-button Ignore"; break;
                default:
                    switch (icon)
                    {
                        case icon::error: command += " --error"; break;
                        case icon::warning: command += " --warning"; break;
                        default: command += " --info"; break;
                    }
            }

            command += " --title " + shell_quote(title)
                     + " --width 300 --height 0" // sensible defaults
                     + " --text " + shell_quote(text)
                     + " --icon-name=dialog-" + get_icon_name(icon);
        }
        else if (is_kdialog())
        {
            if (choice == choice::ok)
            {
                switch (icon)
                {
                    case icon::error: command += " --error"; break;
                    case icon::warning: command += " --sorry"; break;
                    default: command += " --msgbox"; break;
                }
            }
            else
            {
                command += " --";
                if (icon == icon::warning || icon == icon::error)
                    command += "warning";
                command += "yesno";
                if (choice == choice::yes_no_cancel)
                    command += "cancel";
                if (choice == choice::yes_no || choice == choice::yes_no_cancel)
                {
                    m_mappings[0] = button::yes;
                    m_mappings[256] = button::no;
                }
            }

            command += " " + shell_quote(text)
                     + " --title " + shell_quote(title);

            // Must be after the above part
            if (choice == choice::ok_cancel)
                command += " --yes-label OK --no-label Cancel";
        }

        if (flags(flag::is_verbose))
            std::cerr << "pfd: " << command << std::endl;

        m_async->start(command);
#endif
    }

    button result()
    {
        int exit_code;
        auto ret = m_async->result(&exit_code);
        // osascript will say "button returned:Cancel\n"
        // and others will just say "Cancel\n"
        if (exit_code < 0 || // this means cancel
            internal::ends_with(ret, "Cancel\n"))
            return button::cancel;
        if (internal::ends_with(ret, "OK\n"))
            return button::ok;
        if (internal::ends_with(ret, "Yes\n"))
            return button::yes;
        if (internal::ends_with(ret, "No\n"))
            return button::no;
        if (internal::ends_with(ret, "Abort\n"))
            return button::abort;
        if (internal::ends_with(ret, "Retry\n"))
            return button::retry;
        if (internal::ends_with(ret, "Ignore\n"))
            return button::ignore;
        if (m_mappings.count(exit_code) != 0)
            return m_mappings[exit_code];
        return exit_code == 0 ? button::ok : button::cancel;
    }

private:
    // Some extra logic to map the exit code to button number
    std::map<int, button> m_mappings;
};

class open_file : public internal::file_dialog
{
public:
    open_file(std::string const &title,
              std::string const &default_path = "",
              std::vector<std::string> filters = { "All Files", "*" },
              bool allow_multiselect = false)
      : file_dialog(type::open, title, default_path,
                    filters, allow_multiselect, false)
    {
    }

    std::vector<std::string> result()
    {
        return vector_result();
    }
};

class save_file : public internal::file_dialog
{
public:
    save_file(std::string const &title,
              std::string const &default_path = "",
              std::vector<std::string> filters = { "All Files", "*" },
              bool confirm_overwrite = true)
      : file_dialog(type::save, title, default_path,
                    filters, false, confirm_overwrite)
    {
    }

    std::string result()
    {
        return string_result();
    }
};

class select_folder : public internal::file_dialog
{
public:
    select_folder(std::string const &title,
                  std::string const &default_path = "")
      : file_dialog(type::folder, title, default_path)
    {
    }

    std::string result()
    {
        return string_result();
    }
};

} // namespace pfd

