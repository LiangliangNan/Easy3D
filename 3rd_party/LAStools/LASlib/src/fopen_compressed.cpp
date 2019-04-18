/*
===============================================================================

  FILE:  fopen_compressed.cpp
  
  CONTENTS:
      
  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:

    (c) 2003-2012, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    20 March 2011 -- added capability for *.zip, *.rar, and *.7z on Windows
    12 December 2003 -- adapted from Stefan Gumhold's SIGGRAPH submission hack
  
===============================================================================
*/

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <windows.h>
#endif

enum PIPES { READ_HANDLE, WRITE_HANDLE }; /* Constants 0 and 1 for READ and WRITE */

// open a 7zipped/ZIPped/gzipped/rarzipped file as if they were regular a file

#ifdef _WIN32
static FILE* fopen7zipped(const char* filename, const char* mode)
{
	// check mode
	if (mode[0] == 'r')
	{
		// create the pipe
		int hPipe[2];
		if (_pipe(hPipe, 2048, ((mode[1] =='b') ? _O_BINARY : _O_TEXT) | _O_NOINHERIT) == -1)
		{
			fprintf(stderr, "could not create pipe\n");
			return NULL;
		}

		// duplicate stdin/stdout handle so we can restore them later
		int hStdOut = _dup(_fileno(stdout));

		// make the write end of pipe go to stdout
		if (_dup2(hPipe[WRITE_HANDLE], _fileno(stdout)) != 0)
		{
			fprintf(stderr, "could not set pipe output\n");
			return NULL;
		}

		// redirect read end of pipe to input file
		if (_dup2(hPipe[READ_HANDLE], _fileno(stdin)) != 0)
		{
			fprintf(stderr, "could not redirect input file\n");
			return NULL;
		}

    // close original write end of pipe
		_close(hPipe[WRITE_HANDLE]);

		// Spawn process
		HANDLE hProcess = (HANDLE)_spawnlp(P_NOWAIT, "7z", "7z", "e", "-so", filename, NULL);

		// redirect stdout back into stdout
		if (_dup2(hStdOut, _fileno(stdout)) != 0)
		{
			fprintf(stderr, "could not reconstruct stdout\n");
			return NULL;
		}

		// return redirected stdin 
		return stdin;
	}
	else
	{
		return NULL;
	}
}

static FILE* fopenZIPped(const char* filename, const char* mode)
{
	// check mode
	if (mode[0] == 'r')
	{
		// create the pipe
		int hPipe[2];
		if (_pipe(hPipe, 2048, ((mode[1] =='b') ? _O_BINARY : _O_TEXT) | _O_NOINHERIT) == -1)
		{
			fprintf(stderr, "could not create pipe\n");
			return NULL;
		}

		// duplicate stdin/stdout handle so we can restore them later
		int hStdOut = _dup(_fileno(stdout));

		// make the write end of pipe go to stdout
		if (_dup2(hPipe[WRITE_HANDLE], _fileno(stdout)) != 0)
		{
			fprintf(stderr, "could not set pipe output\n");
			return NULL;
		}

		// redirect read end of pipe to input file
		if (_dup2(hPipe[READ_HANDLE], _fileno(stdin)) != 0)
		{
			fprintf(stderr, "could not redirect input file\n");
			return NULL;
		}

    // close original write end of pipe
		_close(hPipe[WRITE_HANDLE]);

		// Spawn process
		HANDLE hProcess = (HANDLE)_spawnlp(P_NOWAIT, "unzip", "unzip", "-p", filename, NULL);

		// redirect stdout back into stdout
		if (_dup2(hStdOut, _fileno(stdout)) != 0)
		{
			fprintf(stderr, "could not reconstruct stdout\n");
			return NULL;
		}

		// return redirected stdin 
		return stdin;
	}
	else
	{
		return NULL;
	}
}

static FILE* fopenGzipped(const char* filename, const char* mode)
{
	// check mode
	if (mode[0] == 'r')
	{
		// open input file
		FILE* gzipInput = fopen(filename, mode);
		if (!gzipInput) return NULL;

		// create the pipe
		int hPipe[2];
		if (_pipe(hPipe, 2048, ((mode[1] =='b') ? _O_BINARY : _O_TEXT) | _O_NOINHERIT) == -1)
		{
			fprintf(stderr, "could not create pipe\n");
			return NULL;
		}

		// duplicate stdin handle
		int hStdIn = _dup(_fileno(stdin));
		// redirect stdin to input file
		if (_dup2(_fileno(gzipInput), _fileno(stdin)) != 0)
		{
			fprintf(stderr, "could not redirect stdin\n");
			return NULL;
		}

		// duplicate stdout handle
		int hStdOut = _dup(_fileno(stdout));
		// redirect stdout to write end of pipe
		if (_dup2(hPipe[WRITE_HANDLE], _fileno(stdout)) != 0)
		{
			fprintf(stderr, "could not set pipe output\n");
			return NULL;
		}

		// close original write end of pipe
		_close(hPipe[WRITE_HANDLE]);

		// redirect read end of pipe to input file
		if (_dup2(hPipe[READ_HANDLE], _fileno(gzipInput)) != 0)
		{
			fprintf(stderr, "could not redirect input file\n");
			return NULL;
		}

		// close original read end of pipe
		_close(hPipe[READ_HANDLE]);

		// Spawn process
		HANDLE hProcess = (HANDLE)_spawnlp(P_NOWAIT, "gzip", "gzip", "-d", NULL);

		// redirect stdin back into stdin
		if (_dup2(hStdIn, _fileno(stdin)) != 0)
		{
			fprintf(stderr, "could not reconstruct stdin\n");
			return NULL;
		}

		// redirect stdout back into stdout
		if (_dup2(hStdOut, _fileno(stdout)) != 0)
		{
			fprintf(stderr, "could not reconstruct stdout\n");
			return NULL;
		}

		// return input file
		return gzipInput;
	}
	else
	{
		return NULL;
	}
}

static FILE* fopenGzippedNew(const char* filename, const char* mode)
{
	// check mode
	if (mode[0] == 'r')
	{
		// create the pipe
		int hPipe[2];
		if (_pipe(hPipe, 2048, ((mode[1] =='b') ? _O_BINARY : _O_TEXT) | _O_NOINHERIT) == -1)
		{
			fprintf(stderr, "could not create pipe\n");
			return NULL;
		}

		// duplicate stdin/stdout handle so we can restore them later
		int hStdOut = _dup(_fileno(stdout));

		// make the write end of pipe go to stdout
		if (_dup2(hPipe[WRITE_HANDLE], _fileno(stdout)) != 0)
		{
			fprintf(stderr, "could not set pipe output\n");
			return NULL;
		}

		// redirect read end of pipe to input file
		if (_dup2(hPipe[READ_HANDLE], _fileno(stdin)) != 0)
		{
			fprintf(stderr, "could not redirect input file\n");
			return NULL;
		}

    // close original write end of pipe
		_close(hPipe[WRITE_HANDLE]);

		// Spawn process
		HANDLE hProcess = (HANDLE)_spawnlp(P_NOWAIT, "gzip", "gzip", "-dc", filename, NULL);

		// redirect stdout back into stdout
		if (_dup2(hStdOut, _fileno(stdout)) != 0)
		{
			fprintf(stderr, "could not reconstruct stdout\n");
			return NULL;
		}

		// return redirected stdin 
		return stdin;
	}
	else
	{
		return NULL;
	}
}

static FILE* fopenRARed(const char* filename, const char* mode)
{
	// check mode
	if (mode[0] == 'r')
	{
		// create the pipe
		int hPipe[2];
		if (_pipe(hPipe, 2048, ((mode[1] =='b') ? _O_BINARY : _O_TEXT) | _O_NOINHERIT) == -1)
		{
			fprintf(stderr, "could not create pipe\n");
			return NULL;
		}

		// duplicate stdin/stdout handle so we can restore them later
		int hStdOut = _dup(_fileno(stdout));

		// make the write end of pipe go to stdout
		if (_dup2(hPipe[WRITE_HANDLE], _fileno(stdout)) != 0)
		{
			fprintf(stderr, "could not set pipe output\n");
			return NULL;
		}

		// redirect read end of pipe to input file
		if (_dup2(hPipe[READ_HANDLE], _fileno(stdin)) != 0)
		{
			fprintf(stderr, "could not redirect input file\n");
			return NULL;
		}

    // close original write end of pipe
		_close(hPipe[WRITE_HANDLE]);

		// Spawn process
		HANDLE hProcess = (HANDLE)_spawnlp(P_NOWAIT, "unrar", "unrar", "p", "-ierr", filename, NULL);

		// redirect stdout back into stdout
		if (_dup2(hStdOut, _fileno(stdout)) != 0)
		{
			fprintf(stderr, "could not reconstruct stdout\n");
			return NULL;
		}

		// return redirected stdin 
		return stdin;
	}
	else
	{
		return NULL;
	}
}
#endif

extern "C"
{
FILE* fopen_compressed(const char* filename, const char* mode, bool* piped)
{
  FILE* file;

  if (strstr(filename, ".gz"))
  {
#ifdef _WIN32
    file = fopenGzipped(filename, mode);
    if (piped) *piped = true;
#else
    fprintf(stderr, "ERROR: no support for gzipped input\n");
    return 0;
#endif
  }
  else if (strstr(filename, ".zip"))
  {
#ifdef _WIN32
    file = fopenZIPped(filename, mode);
    if (piped) *piped = true;
#else
    fprintf(stderr, "ERROR: no support for ZIPped input\n");
    return 0;
#endif
  }
  else if (strstr(filename, ".7z"))
  {
#ifdef _WIN32
    file = fopen7zipped(filename, mode);
    if (piped) *piped = true;
#else
    fprintf(stderr, "ERROR: no support for 7zipped input\n");
    return 0;
#endif
  }
  else if (strstr(filename, ".rar"))
  {
#ifdef _WIN32
    file = fopenRARed(filename, mode);
    if (piped) *piped = true;
#else
    fprintf(stderr, "ERROR: no support for RARed input\n");
    return 0;
#endif
  }
  else
  {
    file = fopen(filename, mode);
    if (piped) *piped = false;
  }
  return file;
}
}
