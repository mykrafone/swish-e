/*
$Id$
**


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
    
** Mon May  9 18:19:34 CDT 2005
** added GPL


**
**
** 2001-05-25 rasc    new module
**
*/


#ifndef __HasSeenModule_DataTypes
#define __HasSeenModule_DataTypes	1


/* Global module data definition */



/* -- Swish Data Types 
   -- used for Properties and MetaNames
   -- Result handling structures, (types storage, values)
   -- Warnung! Changing types inflicts output routines, etc. 
   -- moved from swish.h and adapted, replaces former PropValue
   -- 2001-01  rasc
*/

typedef enum
{                               /* Property/MetaNames Datatypes */
    UNDEFINED = -1, UNKNOWN = 0, STRING, INTEGER, FLOAT, DATE
}
SwDataType;

typedef union
{                               /* storage of the DataValue */
    char   *v_str;              /* strings */
    int     v_int;              /* Integer */
    time_t  v_date;             /* Date    */
    double  v_float;            /* Double Float */
}
u_SwDataValue1;

typedef struct
{                               /* DataValue with type info */
    SwDataType     datatype;
    u_SwDataValue1 value;
}
SwDataValue;




/* Public prototypes */

char *datavalue2str (SwDataValue *v);
char *datavalue2strfmt (SwDataValue *v, char *altfmt);
SwDataValue *str2var (char *s, SwDataType dt);
int datavaluecmp (SwDataValue *a, SwDataValue *b);


#endif


