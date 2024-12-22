#include <3rd_party/easyloggingpp/easylogging++.cc>
#include <3rd_party/easyloggingpp/easylogging++.h>
#include <3rd_party/json/json.hpp>
#include <3rd_party/portable_file_dialogs/portable_file_dialogs.h>
#include <ShlObj.h>   // for SHGetFolderPathA
#include <Windows.h>	
#include <algorithm>
#include <algorithm>	// for std::min and std::max
#include <cassert>
#include <chrono>
#include <cmath>
#include <codecvt>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstdlib>  // for std::getenv
#include <cstring>
#include <cstring>  // for strcmp
#include <ctime>
#include <ctime>	    // for get_time_string()
#include <direct.h>   // for _mkdir
#include <dirent.h>
#include <easy3d/core/types.h>
#include <easy3d/util/console_style.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/export.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/progress.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/setting.h>
#include <easy3d/util/stop_watch.h>
#include <easy3d/util/string.h>
#include <easy3d/util/version.h>
#include <fstream>
#include <functional>
#include <io.h>
#include <iomanip>
#include <iostream>
#include <libproc.h>
#include <ostream>
#include <pwd.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/stat.h> // for _stat64
#include <thread>
#include <unistd.h>
#include <vector>
