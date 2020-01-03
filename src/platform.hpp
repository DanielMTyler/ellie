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

// Return false if file doesn't exist or if an error occured.
ResultBool PlatformFileExists(std::string file);

// Return false if folder doesn't exist or if an error occured.
ResultBool PlatformFolderExists(std::string folder);

ResultBool PlatformGetCWD(std::string& cwd);

// Create a temp file in the proper location for the platform.
// On success, file will be the full path to the temp file.
ResultBool PlatformCreateTempFile(std::string& file);
