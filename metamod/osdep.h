// vi: set ts=4 sw=4 :
// vim: set tw=75 :

// osdep.h - operating system dependencies

/*
 * Copyright (c) 2001-2006 Will Day <willday@hpgx.net>
 *
 *    This file is part of Metamod.
 *
 *    Metamod is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    Metamod is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Metamod; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */

#ifndef OSDEP_H
#define OSDEP_H

#include <string.h>			// strerror()
#include <ctype.h>			// isupper, tolower
#include <errno.h>			// errno

// Various differences between WIN32 and Linux.

#include "comp_dep.h"
#include "types_meta.h"		// mBOOL
#include "mreg.h"			// REG_CMD_FN, etc

// String describing platform/DLL-type, for matching lines in plugins.ini.
#ifdef linux
	#define PLATFORM	"linux"
#  if defined(__x86_64__) || defined(__amd64__)
	#define PLATFORM_SPC	"lin64"
#  else
	#define PLATFORM_SPC	"lin32"
#  endif
	#define PLATFORM_DLEXT	".so"
#elif defined(_WIN32)
	#define PLATFORM	"mswin"
	#define PLATFORM_SPC	"win32"
	#define PLATFORM_DLEXT	".dll"
#else /* unknown */
	#error "OS unrecognized"
#endif /* unknown */

// Macro for function-exporting from DLL..
// from SDK dlls/cbase.h:
//! C functions for external declarations that call the appropriate C++ methods

// Windows uses "__declspec(dllexport)" to mark functions in the DLL that
// should be visible/callable externally.
//
// It also apparently requires WINAPI for GiveFnptrsToDll().
//
// See doc/notes_windows_coding for more information..

// Attributes to specify an "exported" function, visible from outside the
// DLL.
#undef DLLEXPORT
#ifdef _WIN32
	#define DLLEXPORT	__declspec(dllexport)
	// WINAPI should be provided in the windows compiler headers.
	// It's usually defined to something like "__stdcall".
#elif defined(linux)
	#define DLLEXPORT	/* */
	#define WINAPI		/* */
#endif /* linux */

// Simplified macro for declaring/defining exported DLL functions.  They
// need to be 'extern "C"' so that the C++ compiler enforces parameter
// type-matching, rather than considering routines with mis-matched
// arguments/types to be overloaded functions...
//
// AFAIK, this is os-independent, but it's included here in osdep.h where
// DLLEXPORT is defined, for convenience.
#define C_DLLEXPORT		extern "C" DLLEXPORT

// Set filename and pathname maximum lengths.  Note some windows compilers
// provide a <limits.h> which is incomplete and/or causes problems; see
// doc/windows_notes.txt for more information.
//
// Note that both OS's include room for null-termination:
//   linux:    "# chars in a path name including nul"
//   win32:    "note that the sizes include space for 0-terminator"
#ifdef linux
	#include <limits.h>
#elif defined(_WIN32)
	#include <stdlib.h>
	#define NAME_MAX	_MAX_FNAME
	#ifndef PATH_MAX
		#define PATH_MAX	_MAX_PATH
	#endif
#endif /* _WIN32 */

// Various other windows routine differences.
#ifdef linux
	#include <unistd.h>	// sleep
	#ifndef O_BINARY
    		#define O_BINARY 0
	#endif	
#elif defined(_WIN32)
	#include <io.h>
	#include <direct.h>
	
	#define sleep(x) Sleep(x*1000)

	// Fixed MSVC compiling, by Nikolay "The Storm" Baklicharov.
	#if defined(__GNUC__) || defined (_MSC_VER) && _MSC_VER >= 1400
		#define snprintf	_snprintf
		#define vsnprintf	_vsnprintf
		#define unlink		_unlink
		#define strlwr		_strlwr
		#define strdup		_strdup
		#define strcasecmp	_stricmp
		#define strncasecmp	_strnicmp
		#define getcwd		_getcwd
		#define open		_open
		#define read		_read
		#define write		_write
		#define close		_close
	#endif /* GCC or MSVC 8.0+ */
#endif /* _WIN32 */

#if !defined WIN32 && !defined _MSC_VER
#include <unistd.h>	// getcwd
#endif

#include <sys/stat.h>
#ifndef S_ISREG
	// Linux gcc defines this; earlier mingw didn't, later mingw does;
	// MSVC doesn't seem to.
	#define S_ISREG(m)	((m) & S_IFREG)
#endif /* not S_ISREG */
#ifdef _WIN32
	// The following two are defined in mingw but not in MSVC
    #ifndef S_IRUSR
        #define S_IRUSR _S_IREAD
    #endif
    #ifndef S_IWUSR
        #define S_IWUSR _S_IWRITE
    #endif
	
	// The following two are defined neither in mingw nor in MSVC
    #ifndef S_IRGRP
        #define S_IRGRP S_IRUSR
    #endif
    #ifndef S_IWGRP
        #define S_IWGRP S_IWUSR
    #endif
#endif /* _WIN32 */

#endif /* OSDEP_H */
