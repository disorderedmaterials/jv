---
title: Code Overview
brief: Brief overview of the coding style and program dependencies
taxonomy:
  category: docs
docroot: /jv/docs
header_class: alt
---

**JournalViewer** is written entirely in C++, and uses the Qt toolkit for user interface and network access functionality. Reading of instrument `raw` files is performed through use of `libget` by F. Akeroyd, which is encapsulated in a C++ wrapper for ease of use throughout the code. Reading of `nxs` files is achieved through the HDF5 library, again using a custom C++ wrapper to provide easy insertion within the code.

**JournalViewer** makes use of various third-party libraries, namely [Qt5](http://www.qt.io) and the [HDF5 library](http://www.hdfgroup.org/HDF5/). The latter also incorporates the [SZIP library](http://www.hdfgroup.org/doc_resource/SZIP/), as provided by The HDF Group, and [zlib](http://www.zlib.net/), &copy; Jean-loup Gailly and Mark Adler.
