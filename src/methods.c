/*

$Id$


    This file is part of Swish-e.

    Swish-e is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Swish-e is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along  with Swish-e; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
    
    Swish-e includes a library for searching with a well-defined API. The library
    is named libswish-e.
    
    Linking libswish-e statically or dynamically with other modules is making a
    combined work based on Swish-e.  Thus, the terms and conditions of the GNU
    General Public License cover the whole combination.

    As a special exception, the copyright holders of Swish-e give you
    permission to link Swish-e with independent modules that communicate with
    Swish-e solely through the libswish-e API interface, regardless of the license
    terms of these independent modules, and to copy and distribute the
    resulting combined work under terms of your choice, provided that
    every copy of the combined work is accompanied by a complete copy of
    the source code of Swish-e (the version of Swish-e used to produce the
    combined work), being distributed under the terms of the GNU General
    Public License plus this exception.  An independent module is a module
    which is not derived from or based on Swish-e.

    Note that people who make modified versions of Swish-e are not obligated
    to grant this special exception for their modified versions; it is
    their choice whether to do so.  The GNU General Public License gives
    permission to release a modified version without this exception; this
    exception also makes it possible to release a modified version which
    carries forward this exception.
    
** Mon May  9 14:58:45 CDT 2005
** added GPL

*/

#include "swish.h"

#ifdef ALLOW_FILESYSTEM_INDEXING_DATA_SOURCE
extern struct _indexing_data_source_def FileSystemIndexingDataSource;
#endif

#ifdef ALLOW_HTTP_INDEXING_DATA_SOURCE
extern struct _indexing_data_source_def HTTPIndexingDataSource;
#endif

#ifdef ALLOW_EXTERNAL_PROGRAM_DATA_SOURCE
extern struct _indexing_data_source_def ExternalProgramDataSource;
#endif    


struct _indexing_data_source_def *data_sources[] = {

#ifdef ALLOW_FILESYSTEM_INDEXING_DATA_SOURCE
    &FileSystemIndexingDataSource,
#endif

#ifdef ALLOW_HTTP_INDEXING_DATA_SOURCE
    &HTTPIndexingDataSource,
#endif

#ifdef ALLOW_EXTERNAL_PROGRAM_DATA_SOURCE
    &ExternalProgramDataSource,
#endif    

    0
};
