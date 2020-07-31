#ifndef _VERSION_H
#define _VERSION_H
#pragma once

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define ENDPOINT_NAME				"SCZ"
 
#define VERSION_MAJOR               0
#define VERSION_MINOR               1
#define VERSION_REVISION            0
 
#define VER_FILE_DESCRIPTION_STR    ENDPOINT_NAME " Credential Provider for Windows logon"
#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION
#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR)        \
                                    "." STRINGIZE(VERSION_MINOR)    \
                                    "." STRINGIZE(VERSION_REVISION) \
 
#define VER_PRODUCTNAME_STR         ENDPOINT_NAME "_CP"
#define VER_PRODUCT_VERSION         VER_FILE_VERSION
#define VER_PRODUCT_VERSION_STR     VER_FILE_VERSION_STR
#define VER_ORIGINAL_FILENAME_STR   VER_PRODUCTNAME_STR ".dll"
#define VER_INTERNAL_NAME_STR       VER_ORIGINAL_FILENAME_STR
#define VER_COPYRIGHT_STR           "Copyright (C) 2020 SURFnet"

#endif
