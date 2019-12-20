# Portable File Dialogs

A free C++11 file dialog library.

  * works on Windows, Mac OS X, Linux
  * **single-header**, no extra library dependencies
  * **synchronous *or* asynchronous** (does not block the rest of your program!)

Similar to [Tiny File Dialogs](https://sourceforge.net/projects/tinyfiledialogs/) but I like it better.

## Status

This is still experimental and nearly not as feature-complete as
[Tiny File Dialogs](https://sourceforge.net/projects/tinyfiledialogs/),
but for once it seemed more constructive to start a project from scratch
than try to fix its almost 1200 unchecked `strcat` or `strcpy` calls,
lack of proper shell escaping, and synchronous architecture.

The currently available backends are:

  * Win32 API (all known versions of Windows)
  * Mac OS X (using AppleScript)
  * GNOME desktop (using [Zenity](https://en.wikipedia.org/wiki/Zenity) or its clones Matedialog and Qarma)
  * KDE desktop (using [KDialog](https://github.com/KDE/kdialog))

## Documentation

  * [Message Box API](https://github.com/samhocevar/portable-file-dialogs/issues/1)
  * [Notification API](https://github.com/samhocevar/portable-file-dialogs/issues/2)
  * [File Open API](https://github.com/samhocevar/portable-file-dialogs/issues/3)
  * [Folder Selection Open API](https://github.com/samhocevar/portable-file-dialogs/issues/13)

## Screenshots (Windows 10)

![warning-win32](https://user-images.githubusercontent.com/245089/47136607-76919a00-d2b4-11e8-8f42-e2d62c4f9570.png)
![notify-win32](https://user-images.githubusercontent.com/245089/47142453-2ff76c00-d2c3-11e8-871a-1a110ac91eb2.png)
![open-win32](https://user-images.githubusercontent.com/245089/47155865-0f8cd900-d2e6-11e8-8041-1e20b6f77dee.png)

## Screenshots (Mac OS X, dark theme)

![warning-osxdark](https://user-images.githubusercontent.com/245089/56053001-22dba700-5d53-11e9-8233-ca7a2c58188d.png)
![notify-osxdark](https://user-images.githubusercontent.com/245089/56053188-bc0abd80-5d53-11e9-8298-68aa96315c6c.png)
![open-osxdark](https://user-images.githubusercontent.com/245089/56053378-39363280-5d54-11e9-9583-9f1c978fa0db.png)

## Screenshots (Linux, GNOME desktop)

![warning-gnome](https://user-images.githubusercontent.com/245089/47136608-772a3080-d2b4-11e8-9e1d-60a7e743e908.png)
![notify-gnome](https://user-images.githubusercontent.com/245089/47142455-30900280-d2c3-11e8-8b76-ea16c7e502d4.png)
![open-gnome](https://user-images.githubusercontent.com/245089/47155867-0f8cd900-d2e6-11e8-93af-275636491ec4.png)

## Screenshots (Linux, KDE Plasma desktop)

![warning-kde](https://user-images.githubusercontent.com/245089/47149255-4dcccd00-d2d3-11e8-84c9-f85612784680.png)
![notify-kde](https://user-images.githubusercontent.com/245089/47149206-27a72d00-d2d3-11e8-8f1b-96e462f08c2b.png)
![open-kde](https://user-images.githubusercontent.com/245089/47155866-0f8cd900-d2e6-11e8-8006-f14b948afc55.png)

