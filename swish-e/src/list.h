/*
** Copyright (C) 1995, 1996, 1997, 1998 Hewlett-Packard Company
** Originally by Kevin Hughes, kev@kevcom.com, 3/11/94
**
** This program and library is free software; you can redistribute it and/or
** modify it under the terms of the GNU (Library) General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU (Library) General Public License for more details.
**
** You should have received a copy of the GNU (Library) General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*
** use _AP() for easier cross-compiler (non-ANSI) porting 
** <return value> <functionname> _AP( (<arg prototypes>) );
*/

struct swline *addswline _AP ((struct swline *, char *));
struct swline *dupswline _AP ((struct swline *));
IndexFILE *addindexfile _AP ((IndexFILE *, char *));
struct filter *addfilter _AP((struct filter *rp, char *FilterSuffix, char *FilterProg, char *FilterDir)); 
void init_header _AP ((INDEXDATAHEADER *));
void free_header _AP ((INDEXDATAHEADER *));
