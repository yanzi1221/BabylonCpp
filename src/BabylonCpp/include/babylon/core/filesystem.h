#ifndef BABYLON_CORE_FILESYSTEM_H
#define BABYLON_CORE_FILESYSTEM_H

#include <babylon/core/filesystem/filesystem_common.h>

#ifdef __unix__
// Unix filesystem functions
#include <babylon/core/filesystem/filesystem_unix.h>
#elif _WIN32
// Windows filesystem functions
#include <babylon/core/filesystem/filesystem_win32.h>
#endif

#endif // BABYLON_CORE_FILESYSTEM_H
