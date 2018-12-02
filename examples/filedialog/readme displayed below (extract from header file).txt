tiny file dialogs ( cross-platform C C++ ) v3.3.8 [Nov 4, 2018] zlib licence
 _________
/         \   Beep Popup InputBox PasswordBox MessageBox ColorPicker
|tiny file|   OpenFileDialog SaveFileDialog SelectFolderDialog
| dialogs |   ASCII UTF-8 (and also MBCS UTF-16 for windows)
\____  ___/   Native dialog library for WINDOWS MAC OSX GTK+ QT CONSOLE
     \|       SSH support via automatic switch to console mode or X11 forwarding

Please upvote my stackoverflow answer https://stackoverflow.com/a/47651444

C89 & C++98 compliant: tested with C & C++ compilers
VisualStudio MinGW GCC Clang TinyCC OpenWatcom-v2 BorlandC SunCC ZapCC
on Windows Mac Linux Bsd Solaris Minix Raspbian
using Gnome Kde Mate Enlightenment Cinnamon Budgie Unity Lxde Lxqt Xfce
      WindowMaker IceWm Cde Jds OpenBox Awesome Jwm Xdm

Bindings for LUA and C# dll, Haskell. Included in LWJGL(java), Rust, Allegrobasic

                   http://tinyfiledialogs.sourceforge.net
         git clone http://git.code.sf.net/p/tinyfiledialogs/code tinyfd
  _________________________________________________________________________
 |                                                                         |
 | CONTACT me directly via the email address at the top of the header file |
 |_________________________________________________________________________|
                   _____________________________________
                  |                                     |
                  | version 3 has the same good old API |
                  |       dialogs now stay on top       |
                  |       better UNICODE handling       |
                  | new dialog: TRAY NOTIFICATION POPUP |
                  |     2nd new dialog: SYSTEM BEEP     |
                  |_____________________________________|
     ___________________________________________________________________
    |                                                                   |
    | the windows only wchar_t UTF-16 prototypes are in the header file |
    |              NOW ALSO FOR THE INPUT / PASSWORD BOX                |
    |___________________________________________________________________|

void tinyfd_beep() ;

int tinyfd_notifyPopup (
    char const * const aTitle, // NULL or ""
    char const * const aMessage, // NULL or "" may contain \n \t
    char const * const aIconType); // "info" "warning" "error"

int tinyfd_messageBox (
    char const * const aTitle , // NULL or ""
    char const * const aMessage , // NULL or "" may contain \n \t
    char const * const aDialogType , // "ok" "okcancel" "yesno" "yesnocancel"
    char const * const aIconType , // "info" "warning" "error" "question"
    int const aDefaultButton ) ;
        // 0 for cancel/no , 1 for ok/yes , 2 for no in yesnocancel

char const * tinyfd_inputBox (
    char const * const aTitle , // NULL or ""
    char const * const aMessage , // NULL or "" may NOT contain \n \t on windows
    char const * const aDefaultInput ) ; // "" , if NULL it's a passwordBox
        // returns NULL on cancel

char const * tinyfd_saveFileDialog (
    char const * const aTitle , // NULL or ""
    char const * const aDefaultPathAndFile , // NULL or ""
    int const aNumOfFilterPatterns , // 0
    char const * const * const aFilterPatterns , // NULL | {"*.txt"}
    char const * const aSingleFilterDescription ) ; // NULL | "text files"
        // returns NULL on cancel

char const * tinyfd_openFileDialog (
    char const * const aTitle , // NULL or ""
    char const * const aDefaultPathAndFile , // NULL or ""
    int const aNumOfFilterPatterns , // 0
    char const * const * const aFilterPatterns , // NULL {"*.jpg","*.png"}
    char const * const aSingleFilterDescription , // NULL | "image files"
    int const aAllowMultipleSelects ) ; // 0
        // in case of multiple files, the separator is |
        // returns NULL on cancel

char const * tinyfd_selectFolderDialog (
    char const * const aTitle , // NULL or ""
    char const * const aDefaultPath ) ; // NULL or ""
        // returns NULL on cancel

char const * tinyfd_colorChooser (
    char const * const aTitle , // NULL or ""
    char const * const aDefaultHexRGB , // NULL or "#FF0000”
    unsigned char const aDefaultRGB[3] , // { 0 , 255 , 255 }
    unsigned char aoResultRGB[3] ) ; // { 0 , 0 , 0 }
        // returns the hexcolor as a string "#FF0000"
        // aoResultRGB also contains the result
        // aDefaultRGB is used only if aDefaultHexRGB is NULL
        // aDefaultRGB and aoResultRGB can be the same array
        // returns NULL on cancel


- This is not for android nor ios.
- The code is pure C, perfectly compatible with C++.
- the windows only wchar_t (utf-16) prototypes are in the header file
- windows is fully supported from XP to 10 (maybe even older versions)
- C# & LUA via dll, see files in the folder EXTRAS 
- OSX supported from 10.4 to latest (maybe even older versions)
- Avoid using " and ' in titles and messages.
- There's one file filter only, it may contain several patterns.
- If no filter description is provided,
  the list of patterns will become the description.
- char const * filterPatterns[3] = { "*.obj" , "*.stl" , "*.dxf" } ;
- On windows char defaults to MBCS, set tinyfd_winUtf8=1 to use UTF-8
- On windows link against Comdlg32.lib and Ole32.lib
  (on windows the no linking claim is a lie)
  This linking is not compulsary for console mode (see header file).
- On unix: it tries command line calls, so no such need (NO LINKING).
- On unix you need one of the following:
  applescript, kdialog, zenity, matedialog, shellementary, qarma,
  python (2 or 3)/tkinter/python-dbus (optional), Xdialog
  or dialog (opens terminal if running without console) or xterm.
- One of those is already included on most (if not all) desktops.
- In the absence of those it will use gdialog, gxmessage or whiptail
  with a textinputbox.
- If nothing is found, it switches to basic console input,
  it opens a console if needed (requires xterm + bash).
- Use windows separators on windows and unix separators on unix.
- String memory is preallocated statically for all the returned values.
- File and path names are tested before return, they are valid.
- If you pass only a path instead of path + filename,
  make sure it ends with a separator.
- tinyfd_forceConsole=1; at run time, forces dialogs into console mode.
- On windows, console mode only make sense for console applications.
- On windows, Console mode is not implemented for wchar_T UTF-16.
- Mutiple selects are not allowed in console mode.
- The package dialog must be installed to run in enhanced console mode.
  It is already installed on most unix systems.
- On osx, the package dialog can be installed via
  http://macappstore.org/dialog or http://macports.org
- On windows, for enhanced console mode,
  dialog.exe should be copied somewhere on your executable path.
  It can be found at the bottom of the following page:
  http://andrear.altervista.org/home/cdialog.php
- If dialog is missing, it will switch to basic console input.
- You can query the type of dialog that will be use.
- MinGW needs gcc >= v4.9 otherwise some headers are incomplete.

- Here is the Hello World:
            if a console is missing, it will use graphic dialogs
            if a graphical display is absent, it will use console dialogs


hello.c
#include <stdio.h>
#include <string.h>
#include "tinyfiledialogs.h"
int main()
{
	char const * lTmp;
	char const * lTheSaveFileName;
	char const * lTheOpenFileName;
	char const * lTheSelectFolderName;
	char const * lTheHexColor;
	char const * lWillBeGraphicMode;
	unsigned char lRgbColor[3];
	FILE * lIn;
	char lBuffer[1024];
	char lString[1024];
	char const * lFilterPatterns[2] = { "*.txt", "*.text" };

	lWillBeGraphicMode = tinyfd_inputBox("tinyfd_query", NULL, NULL);

	if (lWillBeGraphicMode)
	{
		strcpy(lBuffer, "graphic mode: ");
	}
	else
	{
		strcpy(lBuffer, "console mode: ");
	}

	strcat(lBuffer, tinyfd_response);
	strcpy(lString, "v");
	strcat(lString, tinyfd_version);
	strcat(lString, " tinyfiledialogs");
	tinyfd_messageBox(lString, lBuffer, "ok", "info", 0);

	tinyfd_notifyPopup("the title", "the message\n\tfrom outer-space", "info");

	if ( lWillBeGraphicMode && ! tinyfd_forceConsole )
	{
		tinyfd_forceConsole = ! tinyfd_messageBox("Hello World",
			"graphic dialogs [yes] / console mode [no]?",
			"yesno", "question", 1);
	}

	lTmp = tinyfd_inputBox(
		"a password box", "your password will be revealed", NULL);

	if (!lTmp) return 1 ;

	/* copy lTmp because saveDialog would overwrites
	inputBox static buffer in basicinput mode */

	strcpy(lString, lTmp);

	lTheSaveFileName = tinyfd_saveFileDialog(
		"let us save this password",
		"passwordFile.txt",
		2,
		lFilterPatterns,
		NULL);

	if (! lTheSaveFileName)
	{
		tinyfd_messageBox(
			"Error",
			"Save file name is NULL",
			"ok",
			"error",
			1);
		return 1 ;
	}

	lIn = fopen(lTheSaveFileName, "w");
	if (!lIn)
	{
		tinyfd_messageBox(
			"Error",
			"Can not open this file in write mode",
			"ok",
			"error",
			1);
		return 1 ;
	}
	fputs(lString, lIn);
	fclose(lIn);

	lTheOpenFileName = tinyfd_openFileDialog(
		"let us read the password back",
		"",
		2,
		lFilterPatterns,
		NULL,
		0);

	if (! lTheOpenFileName)
	{
		tinyfd_messageBox(
			"Error",
			"Open file name is NULL",
			"ok",
			"error",
			1);
		return 1 ;
	}

	lIn = fopen(lTheOpenFileName, "r");

	if (!lIn)
	{
		tinyfd_messageBox(
			"Error",
			"Can not open this file in read mode",
			"ok",
			"error",
			1);
		return(1);
	}
	lBuffer[0] = '\0';
	fgets(lBuffer, sizeof(lBuffer), lIn);
	fclose(lIn);

	tinyfd_messageBox("your password is",
			lBuffer, "ok", "info", 1);

	lTheSelectFolderName = tinyfd_selectFolderDialog(
		"let us just select a directory", NULL);

	if (!lTheSelectFolderName)
	{
		tinyfd_messageBox(
			"Error",
			"Select folder name is NULL",
			"ok",
			"error",
			1);
		return 1;
	}

	tinyfd_messageBox("The selected folder is",
		lTheSelectFolderName, "ok", "info", 1);

	lTheHexColor = tinyfd_colorChooser(
		"choose a nice color",
		"#FF0077",
		lRgbColor,
		lRgbColor);

	if (!lTheHexColor)
	{
		tinyfd_messageBox(
			"Error",
			"hexcolor is NULL",
			"ok",
			"error",
			1);
		return 1;
	}

	tinyfd_messageBox("The selected hexcolor is",
		lTheHexColor, "ok", "info", 1);

	tinyfd_beep();

	return 0;
}


OSX :
$ clang -o hello.app hello.c tinyfiledialogs.c
( or gcc )

UNIX :
$ gcc -o hello hello.c tinyfiledialogs.c
( or clang tcc owcc cc CC )

Windows :
    MinGW needs gcc >= v4.9 otherwise some headers are incomplete
    > gcc -o hello.exe hello.c tinyfiledialogs.c -LC:/mingw/lib -lcomdlg32 -lole32

    TinyCC needs >= v0.9.27 (+ tweaks - contact me) otherwise some headers are missing
    > tcc -o hello.exe hello.c tinyfiledialogs.c
        -isystem C:\tcc\winapi-full-for-0.9.27\include\winapi
        -lcomdlg32 -lole32 -luser32 -lshell32

	Borland C: > bcc32c -o hello.exe hello.c tinyfiledialogs.c

	OpenWatcom v2: create a character-mode executable project.

    VisualStudio :
      Create a console application project,
      it links against Comdlg32.lib & Ole32.lib.
