/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "global.hpp"
#include <string>

ResultBool PlatformCopyFile(std::string src, std::string dst, bool failIfExists);
ResultBool PlatformDeleteFile(std::string file);
ResultBool PlatformFileExists(std::string file);
ResultBool PlatformFolderExists(std::string folder);
// cwd will end with a path separator.
ResultBool PlatformGetCWD(std::string& cwd);
ResultBool PlatformCreateTempFile(std::string& filePath);
const char *PlatformPathSeparator();
