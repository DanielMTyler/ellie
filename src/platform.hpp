/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef PLATFORM_HPP_INCLUDED
#define PLATFORM_HPP_INCLUDED

#include "global.hpp"
#include <string>

// @todo Unicode support.

// PLATFORM_PATH_SEPARATOR is defined by the platform.
// PLATFORM_SHARED_LIBRARY_PREFIX is defined by the platform, e.g., "" or "lib".
// PLATFORM_SHARED_LIBRARY_EXT is defined by the platform, e.g., ".dll" or ".so".

ResultBool PlatformCopyFile(std::string src, std::string dst, bool failIfExists);
ResultBool PlatformDeleteFile(std::string file);
ResultBool PlatformFileExists(std::string file);
ResultBool PlatformFolderExists(std::string folder);
// cwd will end with a path separator.
ResultBool PlatformGetCWD(std::string& cwd);
ResultBool PlatformCreateTempFile(std::string& filePath);

#endif
