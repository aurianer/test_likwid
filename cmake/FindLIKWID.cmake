# Copyright (c)      2014 Thomas Heller
# Copyright (c) 2007-2012 Hartmut Kaiser
# Copyright (c) 2010-2011 Matt Anderson
# Copyright (c) 2011      Bryce Lelbach
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

if(NOT TARGET Likwid::likwid)
  find_path(
    LIKWID_INCLUDE_DIR likwid.h
    HINTS ${LIKWID_ROOT}
          ENV
          LIKWID_ROOT
          ${HPX_LIKWID_ROOT}
          ${PC_LIKWID_MINIMAL_INCLUDEDIR}
          ${PC_LIKWID_MINIMAL_INCLUDE_DIRS}
          ${PC_LIKWID_INCLUDEDIR}
          ${PC_LIKWID_INCLUDE_DIRS}
    PATH_SUFFIXES include
  )

  find_library(
    LIKWID_LIBRARY
    NAMES likwid liblikwid
    HINTS ${LIKWID_ROOT}
          ENV
          LIKWID_ROOT
          ${HPX_LIKWID_ROOT}
          ${PC_LIKWID_MINIMAL_LIBDIR}
          ${PC_LIKWID_MINIMAL_LIBRARY_DIRS}
          ${PC_LIKWID_LIBDIR}
          ${PC_LIKWID_LIBRARY_DIRS}
    PATH_SUFFIXES lib lib64
  )

  # Set LIKWID_ROOT in case the other hints are used
  if(LIKWID_ROOT)
    # The call to file is for compatibility with windows paths
    file(TO_CMAKE_PATH ${LIKWID_ROOT} LIKWID_ROOT)
  elseif("$ENV{LIKWID_ROOT}")
    file(TO_CMAKE_PATH $ENV{LIKWID_ROOT} LIKWID_ROOT)
  else()
    file(TO_CMAKE_PATH "${LIKWID_INCLUDE_DIR}" LIKWID_INCLUDE_DIR)
    string(REPLACE "/include" "" LIKWID_ROOT "${LIKWID_INCLUDE_DIR}")
  endif()

  set(LIKWID_LIBRARIES ${LIKWID_LIBRARY})
  set(LIKWID_INCLUDE_DIRS ${LIKWID_INCLUDE_DIR})

  find_package_handle_standard_args(
    LIKWID DEFAULT_MSG LIKWID_LIBRARY LIKWID_INCLUDE_DIR
  )

  get_property(
    _type
    CACHE LIKWID_ROOT
    PROPERTY TYPE
  )
  if(_type)
    set_property(CACHE LIKWID_ROOT PROPERTY ADVANCED 1)
    if("x${_type}" STREQUAL "xUNINITIALIZED")
      set_property(CACHE LIKWID_ROOT PROPERTY TYPE PATH)
    endif()
  endif()

  add_library(Likwid::likwid INTERFACE IMPORTED)
  target_include_directories(Likwid::likwid SYSTEM INTERFACE ${LIKWID_INCLUDE_DIR})
  target_link_libraries(Likwid::likwid INTERFACE ${LIKWID_LIBRARIES})

  mark_as_advanced(LIKWID_ROOT LIKWID_LIBRARY LIKWID_INCLUDE_DIR)
endif()
