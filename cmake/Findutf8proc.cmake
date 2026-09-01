# Copyright (C) 2026 Xavier Del Campo Romero
# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

find_path(utf8proc_INCLUDE_DIRS
    NAMES
        utf8proc.h
    HINTS
        ENV utf8proc_PATH
    PATH_SUFFIXES
        include
)

if(utf8proc_INCLUDE_DIRS)
    set(regex_path ${utf8proc_INCLUDE_DIRS}/utf8proc.h)
    file(STRINGS ${regex_path} major_line REGEX "^#define[ \t]+UTF8PROC_VERSION_MAJOR[ \t]+[0-9]+$")
    file(STRINGS ${regex_path} minor_line REGEX "^#define[ \t]+UTF8PROC_VERSION_MINOR[ \t]+[0-9]+$")
    file(STRINGS ${regex_path} patch_line REGEX "^#define[ \t]+UTF8PROC_VERSION_PATCH[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+UTF8PROC_VERSION_MAJOR[ \t]+([0-9]+)$" "\\1" version_major "${major_line}")
    string(REGEX REPLACE "^#define[ \t]+UTF8PROC_VERSION_MINOR[ \t]+([0-9]+)$" "\\1" version_minor "${minor_line}")
    string(REGEX REPLACE "^#define[ \t]+UTF8PROC_VERSION_PATCH[ \t]+([0-9]+)$" "\\1" version_patch "${patch_line}")
    set(utf8proc_VERSION_STRING ${version_major}.${version_minor}.${version_patch})
    unset(version_major)
    unset(version_minor)
    unset(version_patch)
    unset(major_line)
    unset(minor_line)
    unset(patch_line)
    unset(regex_path)
endif()

find_library(utf8proc_LIBRARIES
    NAMES
        utf8proc
    HINTS
        ENV utf8proc_PATH
    PATH_SUFFIXES
        lib
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(utf8proc
    REQUIRED_VARS
        utf8proc_LIBRARIES utf8proc_INCLUDE_DIRS
    VERSION_VAR utf8proc_VERSION_STRING
)

if(utf8proc_FOUND)
    if(NOT TARGET utf8proc::utf8proc)
        add_library(utf8proc::utf8proc INTERFACE IMPORTED)
        set_target_properties(utf8proc::utf8proc PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${utf8proc_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${utf8proc_LIBRARIES}")
    endif()
endif()
