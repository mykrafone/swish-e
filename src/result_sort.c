/*
$Id$
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
**
**
** jmruiz - 02/2001 - Sorting results module
**
** 2001-05-04 jmruiz added new string comparison routines for proper sorting
**                   sw_strcasecmp and sw_strcmp
**                   also added the skeleton to initModule_ResultSort
**                   and freeModule_ResultSort
**
** 2001-05-05 rasc   just rearranged functions, to make modules look similar
**                   (makes code better to read and understand)
**
**
*/

#include "swish.h"
#include "mem.h"
#include "merge.h"
#include "docprop.h"
#include "metanames.h"
#include "compress.h"
#include "search.h"
#include "error.h"
#include "string.h"
#include "result_sort.h"





/*
** ----------------------------------------------
** 
**  Module management code starts here
**
** ----------------------------------------------
*/



/*
  -- init structures for this module
*/

void initModule_ResultSort (SWISH  *sw)

{
   struct MOD_ResultSort *md;

          /* Allocate structure */
    md =(struct MOD_ResultSort *) emalloc(sizeof(struct MOD_ResultSort));
    sw->ResultSort = md;

          /* Init translation sortorder tables */
    initStrCmpTranslationTable(md->iSortTranslationTable);
    initStrCaseCmpTranslationTable(md->iSortCaseTranslationTable);
          /* Init data for -s command option */
    md->numPropertiesToSort=0;
    md->currentMaxPropertiesToSort=0;
    md->propNameToSort=NULL;
    md->propModeToSort=NULL;
}


/*
  -- release all wired memory for this module
*/

/* Frees memory of vars used by Output properties configuration */
/* Code moved from FreeOutputPropertiesVars (docprop.c) */
void freeModule_ResultSort (SWISH *sw)

{
  struct MOD_ResultSort *md = sw->ResultSort;
  int i;
  IndexFILE *tmpindexlist;


                /* First the common part to all the index files */
        if (md->propNameToSort)
        {
                for(i=0;i<md->numPropertiesToSort;i++)
                        efree(md->propNameToSort[i]);
                efree(md->propNameToSort);
        }
        if (md->propModeToSort)
                efree(md->propModeToSort);
        md->propNameToSort=NULL;
        md->propModeToSort=NULL;
        md->numPropertiesToSort=0;
        md->currentMaxPropertiesToSort=0;
                /* Now free memory for the IDs of each index file */
        for(tmpindexlist=sw->indexlist;tmpindexlist;tmpindexlist=tmpindexlist->next)
        {
                if (tmpindexlist->propIDToSort)
                        efree(tmpindexlist->propIDToSort);
                tmpindexlist->propIDToSort=NULL;
        }

      /* Free Module Data Structure */
      /* should not be freed here */
      //efree (sw->ResultSort);
      //sw->ResultSort = NULL;

  return;
}




/* Frees memory of vars used by Output properties configuration */
/* Code moved from FreeOutputPropertiesVars (docprop.c) */
void resetModule_ResultSort (SWISH *sw)

{
  struct MOD_ResultSort *md = sw->ResultSort;
  int i;
 

   //$$$
   //$$$   Jose would should be moved here 
   //$$$   from freeModule...?


   return;
}






/*
** ----------------------------------------------
** 
**  Module config code starts here
**
** ----------------------------------------------
*/


/*
 -- Config Directives
 -- Configuration directives for this Module
 -- return: 0/1 = none/config applied
*/

int configModule_ResultSort  (SWISH *sw, StringList *sl)

{
  struct MOD_ResultSort *md = sw->ResultSort;
  char *w0    = sl->word[0];
  int  retval = 1;



  // @ Jose:  I don't want to add code, while your work is in progress
  // but here is the idea:
  //   wordsequence of character orders 
  // Config statement:   ResultSortOrder   "A�"  "a�"
  //    could work like follows:   v = value[*s];  loop i rest: value[*(s+i)] = v+i;     


  // md->mydata = ...
  // best practice: copy from _mod_example.c


  retval = 0; // tmp due to empty routine

  return retval;
}




/*
** ----------------------------------------------
** 
**  Module code starts here
**
** ----------------------------------------------
*/


/* Routine to add the properties specified in -s to the internal structure */
void addSearchResultSortProperty(SWISH *sw, char *propName,int mode)
{
IndexFILE *indexf;
struct MOD_ResultSort *rs=sw->ResultSort;

	/* add a property to the list of properties that will be displayed */
	if (rs->numPropertiesToSort >=rs->currentMaxPropertiesToSort)
	{
                      /* Allocate memory */
		if(rs->currentMaxPropertiesToSort) {
                             /* Reallocate memory */
			rs->currentMaxPropertiesToSort+=2;
			rs->propNameToSort=(char **)erealloc(rs->propNameToSort,rs->currentMaxPropertiesToSort*sizeof(char *));
			for(indexf=sw->indexlist;indexf;indexf=indexf->next)
				indexf->propIDToSort=(int *)erealloc(indexf->propIDToSort,rs->currentMaxPropertiesToSort*sizeof(int));
			rs->propModeToSort=(int *)erealloc(rs->propModeToSort,rs->currentMaxPropertiesToSort*sizeof(int));
		} else {
                             /* Allocate memory */
			rs->currentMaxPropertiesToSort=5;
			rs->propNameToSort=(char **)emalloc(rs->currentMaxPropertiesToSort*sizeof(char *));
			rs->propModeToSort=(int *)emalloc(rs->currentMaxPropertiesToSort*sizeof(int));
		}
                    /* End allocation of memory */
	}
	rs->propNameToSort[rs->numPropertiesToSort] = estrdup(propName);
	rs->propModeToSort[rs->numPropertiesToSort++] = mode;
}




/* preprocess Sort Result Properties to get the ID */
/* If there is not a sort properties then use rank */

int     initSortResultProperties(SWISH * sw)
{
    int     i;
    IndexFILE *indexf;

    if (sw->ResultSort->numPropertiesToSort == 0)
    {
        /* hack -> If no sort perperties have been specified then
           use rank in descending mode */
        addSearchResultSortProperty(sw, AUTOPROPERTY_RESULT_RANK, 1);
        for (indexf = sw->indexlist; indexf; indexf = indexf->next)
        {
            indexf->propIDToSort = (int *) emalloc(sizeof(int));

            indexf->propIDToSort[0] = AUTOPROP_ID__RESULT_RANK;
        }

        return RC_OK;
    }
    for (indexf = sw->indexlist; indexf; indexf = indexf->next)
        indexf->propIDToSort = (int *) emalloc(sw->ResultSort->numPropertiesToSort * sizeof(int));

    for (i = 0; i < sw->ResultSort->numPropertiesToSort; i++)
    {
        makeItLow(sw->ResultSort->propNameToSort[i]);
        /* Get ID for each index file */
        for (indexf = sw->indexlist; indexf; indexf = indexf->next)
        {
            indexf->propIDToSort[i] = getMetaNameID(indexf, sw->ResultSort->propNameToSort[i]);
            if (indexf->propIDToSort[i] == 1)
            {
                /* Check fot RANK Autoproperty */
                if (strcasecmp(sw->ResultSort->propNameToSort[i], AUTOPROPERTY_RESULT_RANK) == 0)
                {
                    indexf->propIDToSort[i] = AUTOPROP_ID__RESULT_RANK;
                }
                else
                {
                    progerr("Unknown Sort property name \"%s\" in one of the index files", sw->ResultSort->propNameToSort[i]);
                    return (sw->lasterror = UNKNOWN_PROPERTY_NAME_IN_SEARCH_SORT);
                }
            }
        }
    }
    return RC_OK;
}


/* 02/2001 Jose Ruiz */
/* function for comparing data in order to
get sorted results with qsort (including combinations of asc and descending
fields */
int     compResultsByNonSortedProps(const void *s1, const void *s2)
{
    RESULT *r1 = *(RESULT * const *) s1;
    RESULT *r2 = *(RESULT * const *) s2;
    int     i,
            rc,
            num_fields,
            sortmode;
    SWISH  *sw = (SWISH *) r1->sw;

    num_fields = sw->ResultSort->numPropertiesToSort;
    for (i = 0; i < num_fields; i++)
    {
        sortmode = sw->ResultSort->propModeToSort[i];
        if ((rc = sortmode * sw_strcasecmp(r1->PropSort[i], r2->PropSort[i], sw->ResultSort->iSortCaseTranslationTable)))
            return rc;
    }
    return 0;
}

/* 02/2001 Jose Ruiz */
/* function for comparing data in order to
get sorted results with qsort (including combinations of asc and descending
fields */
/* This routine uses the presorted tables built during the index proccess */
int     compResultsBySortedProps(const void *s1, const void *s2)
{
    RESULT *r1 = *(RESULT * const *) s1;
    RESULT *r2 = *(RESULT * const *) s2;
    register int i,
            num_fields;
    int     rc,
            sortmode;
    SWISH  *sw = (SWISH *) r1->sw;

    num_fields = sw->ResultSort->numPropertiesToSort;
    for (i = 0; i < num_fields; i++)
    {
        sortmode = sw->ResultSort->propModeToSort[i];
        rc = sortmode * (r1->iPropSort[i] - r2->iPropSort[i]);
        if (rc)
            return rc;
    }
    return 0;
}

/* Adds the results of a search, sorts them by rank.
*/

/* Jose Ruiz 04/00
** Complete rewrite
** Sort was made before calling this function !! -> FASTER!!
** This one just reverses order
*/
RESULT *addsortresult(sw, sphead, r)
     SWISH  *sw;
     RESULT *sphead;
     RESULT *r;
{
    if (r->rank > sw->bigrank)
        sw->bigrank = r->rank;
    if (sphead == NULL)
    {
        r->nextsort = NULL;
    }
    else
    {
        r->nextsort = sphead;
    }
    return r;
}

/* Routine to load from the index file the presorted data */
int    *readSortedData(IndexFILE * indexf, struct metaEntry *m)
{
    FILE   *fp = (FILE *) indexf->DB;
    int     j,
            value;
    int    *data = emalloc(indexf->filearray_cursize * sizeof(int));

    fseek(fp, m->sort_offset, SEEK_SET);
    for (j = 0; j < indexf->filearray_cursize; j++)
    {
        uncompress1(value, fp);
        data[j] = value - 1;    /* It was stored as value + 1 */
    }
    return data;
}

/* Routine to get the presorted lookupdata for a result for all the specified properties */
int    *getLookupResultSortedProperties(RESULT * r)
{
    int     i;
    int    *props = NULL;       /* Array to Store properties Lookups */
    struct metaEntry *m = NULL;
    IndexFILE *indexf = r->indexf;
    SWISH  *sw = (SWISH *) r->sw;

    props = (int *) emalloc(sw->ResultSort->numPropertiesToSort * sizeof(int));

    for (i = 0; i < sw->ResultSort->numPropertiesToSort; i++)
    {
        switch (indexf->propIDToSort[i])
        {
        case AUTOPROP_ID__REC_COUNT:
            props[i] = 0;
            break;
        case AUTOPROP_ID__RESULT_RANK:
            props[i] = r->rank;
            break;
        case AUTOPROP_ID__DOCPATH:
            props[i] = r->filenum;
            break;
        case AUTOPROP_ID__INDEXFILE:
            props[i] = 0;       /* Same value for all of them */
            break;
        case AUTOPROP_ID__DOCSIZE:
        case AUTOPROP_ID__LASTMODIFIED:
        case AUTOPROP_ID__STARTPOS:
        case AUTOPROP_ID__TITLE:
        case AUTOPROP_ID__SUMMARY:
        default:               /* User properties */
            m = getMetaIDData(indexf, indexf->propIDToSort[i]);
            if (m)
            {
                /* Read presorted data if not yet loaded (it will load only once */
                if (!m->sorted_data)
                    m->sorted_data = readSortedData(indexf, m);
                props[i] = m->sorted_data[r->filenum - 1];
            }
            else
                props[i] = 0;
            break;
        }
    }
    return props;
}


char  **getResultSortProperties(RESULT * r)
{
    int     i;
    char  **props = NULL;       /* Array to Store properties */
    IndexFILE *indexf = r->indexf;
    SWISH  *sw = (SWISH *) r->sw;

    if (sw->ResultSort->numPropertiesToSort == 0)
        return NULL;

    props = (char **) emalloc(sw->ResultSort->numPropertiesToSort * sizeof(char *));

    for (i = 0; i < sw->ResultSort->numPropertiesToSort; i++)
    {
        props[i] = getResultPropAsString(r, indexf->propIDToSort[i]);
    }
    return props;
}


/* Jose Ruiz 04/00
** Sort results by property
*/
int     sortresults(SWISH * sw, int structure)
{
    int     i,
            j,
            TotalResults;
    RESULT **ptmp;
    RESULT *rtmp;
    RESULT *rp,
           *tmp;
    struct DB_RESULTS *db_results;
    int     (*compResults) (const void *, const void *);

    /* Sort each index file resultlist */
    for (TotalResults = 0, db_results = sw->db_results; db_results; db_results = db_results->next)
    {
        db_results->sortresultlist = NULL;
        db_results->currentresult = NULL;
        rp = db_results->resultlist;

        /* Asign comparison routine to be used by qsort */
        compResults = compResultsBySortedProps;

        /* As we are sorting a unique index file, we can use the presorted data in the index file */
        for (tmp = rp; tmp; tmp = tmp->next)
        {
            /* Load the presorted data */
            tmp->iPropSort = getLookupResultSortedProperties(tmp);
        }

        /* Compute number of results */
        for (i = 0, rtmp = rp; rtmp; rtmp = rtmp->next)
        {
            if (rtmp->structure & structure)
            {
                i++;
            }
        }
        if (i)                  /* If there is something to sort ... */
        {
            /* Compute array size */
            ptmp = (RESULT **) emalloc(i * sizeof(RESULT *));

            /* Build an array with the elements to compare and pointers to data */
            for (j = 0, rtmp = rp; rtmp; rtmp = rtmp->next)
            {
                if (rtmp->structure & structure)
                {
                    ptmp[j++] = rtmp;
                }
            }

            /* Sort them */
            qsort(ptmp, i, sizeof(RESULT *), compResults);

            /* Build the list */
            for (j = 0; j < i; j++)
            {
                db_results->sortresultlist = (RESULT *) addsortresult(sw, db_results->sortresultlist, ptmp[j]);
            }
            /* Free the memory of the array */
            efree(ptmp);
            if (db_results->sortresultlist)
            {
                db_results->currentresult = db_results->sortresultlist;
                TotalResults += countResults(db_results->sortresultlist);
            }
        }
    }
    return TotalResults;
}

/* 01/2001 Jose Ruiz */
/* function for comparing data in order to
get sorted results with qsort (including combinations of asc and descending
fields */
int     compFileProps(const void *s1, const void *s2)
{
    struct file *r1 = *(struct file * const *) s1;
    struct file *r2 = *(struct file * const *) s2;
    docPropertyEntry *p1,
           *p2;
    int     metaID = r1->currentSortProp->metaID,
            rc = 0,
            len = 0;

    for (p1 = r1->docProperties; p1; p1 = p1->next)
        if (metaID == p1->metaID)
            break;
    for (p2 = r2->docProperties; p2; p2 = p2->next)
        if (metaID == p2->metaID)
            break;
    if (!p1 && !p2)
        return 0;
    if (!p1 && p2)
        return 1;
    if (p1 && !p2)
        return -1;

    if (is_meta_number(r1->currentSortProp) || is_meta_date(r1->currentSortProp))
    {
        rc = *((int *) p1->propValue) - *((int *) p2->propValue);
    }
    else if (is_meta_string(r1->currentSortProp))
    {
        len = Min(p1->propLen, p2->propLen);
        rc = strncasecmp(p1->propValue, p2->propValue, len);
        if (!rc)
            rc = p1->propLen - p2->propLen;
    }
    return rc;
}

/* 02/2001 jmruiz */
/* Routine to sort properties at index time */
void    sortFileProperties(IndexFILE * indexf)
{
    int     i,
            j,
            k,
            val;
    int    *sortFilenums;
    struct metaEntry *m;
    FILE   *fp = (FILE *) indexf->DB;

    /* Array of filenums to store the sorted docs (referenced by its filenum) */
    sortFilenums = emalloc(indexf->fileoffsetarray_cursize * sizeof(int));

    /* Execute for each property */
    for (j = 0; j < indexf->metaCounter; j++)
    {
        switch (indexf->metaEntryArray[j]->metaID)
        {
        case AUTOPROP_ID__REC_COUNT:
        case AUTOPROP_ID__RESULT_RANK:
        case AUTOPROP_ID__DOCPATH:
        case AUTOPROP_ID__INDEXFILE:
            break;              /* Do nothing : Files are already sorted */
            /* Rec_count and rank are computed in search */
        case AUTOPROP_ID__TITLE:
        case AUTOPROP_ID__DOCSIZE:
        case AUTOPROP_ID__LASTMODIFIED:
        case AUTOPROP_ID__SUMMARY:
        case AUTOPROP_ID__STARTPOS:
        default:               /* User properties */
            m = getMetaIDData(indexf, indexf->metaEntryArray[j]->metaID);
            for (i = 0; i < indexf->fileoffsetarray_cursize; i++)
                indexf->filearray[i]->currentSortProp = m;
            /* Sort them using qsort. The main work is done by compFileProps */
            qsort(indexf->filearray, indexf->fileoffsetarray_cursize, sizeof(struct file *), &compFileProps);

            /* Build the sorted table */
            for (i = 0, k = 1; i < indexf->fileoffsetarray_cursize; i++)
            {
                /* 02/2001 We can have duplicated values - So all them may have the same number asigned  - qsort justs sorts */
                if (i)
                {
                    /* If consecutive elements are different increase the number */
                    if ((compFileProps(&indexf->filearray[i - 1], &indexf->filearray[i])))
                        k++;
                }
                sortFilenums[indexf->filearray[i]->fi.filenum - 1] = k;
            }

            /* Write the sorted results to disk in compressed format */
            /* Get the offset of the index file */
            m->sort_offset = ftell(fp);
            /* Write the sorted table */
            for (i = 0; i < indexf->fileoffsetarray_cursize; i++)
            {
                val = sortFilenums[i];
                compress1(val, fp);
            }
            break;
        }
    }
    efree(sortFilenums);

}


/* Routines to get the proper sortorder of chars to be called when sorting */
/* sw_strcasecmp sw_strcmp */


/* Exceptions to the standard translation table for sorting strings */
/* See initStrCaseCmpTranslationTable to see how it works */
/* The table shows the equivalences in the following way: */
/*     val(from) = val(order) + offset */
/* where val is asciivalue * 256 */

/* Some comments about ��� ...
** In french and spanish this chars are equivalent to
** � -> a   (french)
** � -> o   (french)
** � -> u   (french + spanish)
** In the other hand, in german:
** � -> a + 1  (german)
** � -> o + 1  (german)
** � -> u + 1  (german)
** I have put the german default. I think that in spanish we can live with that
** If you cannot modify them (change 1 by 0)
** Any comments about other languages are always welcome
*/
struct {
  unsigned char from;
  unsigned char order;
  int offset;
} iTranslationTableExceptions[]=  { 
   { '�', 'A' , 1},    /* >>> german sort order of umlauts */
   { '�', 'O' , 1},    /*     2001-05-04 rasc */
   { '�', 'U' , 1},
   { '�', 'a' , 1},
   { '�', 'o' , 1},
   { '�', 'u' , 1},
   { '�', 's' , 1},    /* <<< german */
   { '�', 'a' , 0},    /* >>> spanish sort order exceptions */
   { '�', 'A' , 0},    /*     2001-05-04 jmruiz */
   { '�', 'e' , 0}, 
   { '�', 'E' , 0}, 
   { '�', 'i' , 0}, 
   { '�', 'I' , 0}, 
   { '�', 'o' , 0}, 
   { '�', 'O' , 0}, 
   { '�', 'u' , 0}, 
   { '�', 'U' , 0}, 
   { '�', 'n' , 1},
   { '�', 'N' , 1},   /* <<< spanish */
   { '�', 'a' , 0},   /* >>> french sort order exceptions */
   { '�', 'A' , 0},   /*     2001-05-04 jmruiz */
   { '�', 'a' , 0},   /*     Taken from the list - Please check */
   { '�', 'A' , 0},   /*     ���������� added in the spanish part */
   { '�', 'c' , 0},   /*     ������ added in the german part */
   { '�', 'C' , 0},
   { '�', 'e' , 0},
   { '�', 'E' , 0},
   { '�', 'e' , 0},
   { '�', 'E' , 0},
   { '�', 'i' , 0},
   { '�', 'I' , 0},
   { '�', 'i' , 0},
   { '�', 'I' , 0},
   { '�', 'o' , 0},
   { '�', 'O' , 0},
   { '�', 'u' , 0},
   { '�', 'U' , 0},    /* >>> french */
   { 0, 0, 0}
};

/* Initialization routine for the comparison table (ignoring case )*/
/* This routine should be called once  at the start of the module */
void initStrCaseCmpTranslationTable(int *iCaseTranslationTable)
{
int i;
      /* Build default table using tolower(asciival) * 256 */
      /* The goal of multiply by 256 is having holes to put values inside
         eg: � is between n and o */
   for(i=0;i<256;i++)
      iCaseTranslationTable[i] = tolower(i) * 256;

      /* Exceptions */
   for(i=0; iTranslationTableExceptions[i].from;i++)
      iCaseTranslationTable[iTranslationTableExceptions[i].from] = tolower(iTranslationTableExceptions[i].order) * 256 + iTranslationTableExceptions[i].offset;
}

/* Initialization routine for the comparison table (case sensitive) */
/* This routine should be called once at the start of the module */
void initStrCmpTranslationTable(int *iCaseTranslationTable)
{
int i;
      /* Build default table using asciival * 256 */
      /* The goal of multiply by 10 is having holes to put values inside
         eg: � is between n and o */
   for(i=0;i<256;i++)
      iCaseTranslationTable[i] = i * 256;

      /* Exceptions */
   for(i=0; iTranslationTableExceptions[i].from;i++)
      iCaseTranslationTable[iTranslationTableExceptions[i].from] = iTranslationTableExceptions[i].order * 256 + iTranslationTableExceptions[i].offset;
}

/* Comparison string routine function. 
** Similar to strcasecmp but using our own translation table
*/
int sw_strcasecmp(unsigned char *s1,unsigned char *s2,int *iTranslationTable)
{
   while ( iTranslationTable[*s1] == iTranslationTable[*s2])
      if (! *s1++) return 0;
      else s2++;
   return iTranslationTable[*s1] - iTranslationTable[*s2];
}

/* Comparison string routine function. 
** Similar to strcmp but using our own translation table
*/
int sw_strcmp(unsigned char *s1,unsigned char *s2,int *iTranslationTable)
{
   while ( iTranslationTable[*s1] == iTranslationTable[*s2])
      if (! *s1++) return 0;
      else s2++;
   return iTranslationTable[*s1] - iTranslationTable[*s2];
}
