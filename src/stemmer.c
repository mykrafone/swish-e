/*
** NOTE: This implementation was originally part of the WAIS system
** The main function, Stem(), was incorporated into Swish-E 1.1
** to provide a stemming function.
**   11/24/98 Mark Gaulin
**
** Stem returns original word if words stems to empty string
** Bill Moseley 10/11/99
**
** Repeats stemming until word will stem no more
** Bill Moseley 10/17/99
**
** function: EndsWithCVC patched a bug. see below.  Moseley 10/19/99
**
** Added word length arg to ReplaceEnd and Stem to avoid strcat overflow
** 11/17/99 - SRE
**
** fixed int cast, missing return value, braces around initializations: problems pointed out by "gcc -Wall"
** SRE 2/22/00
**
** Jose Ruiz 18/10/00
** Remove static word from end var and make the code thread safe
*/

/* WIDE AREA INFORMATION SERVER SOFTWARE:
   No guarantees or restrictions.  See the readme file for the full standard
   disclaimer.

   francois@welchgate.welch.jhu.edu
*/

/* Copyright (c) CNIDR (see ../COPYRIGHT) */



/* 
 * stems a word.
 * 
 */

/* the main functions are:
 *   stemmer
 *
 */

#include "swish.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>


#include "stemmer.h"
#include "mem.h"


#define FALSE	0
#define TRUE	1



/*******************************   stem.c   ***********************************

   Purpose:    Implementation of the Porter stemming algorithm documented 
               in: Porter, M.F., "An Algorithm For Suffix Stripping," 
               Program 14 (3), July 1980, pp. 130-137.

   Provenance: Written by B. Frakes and C. Cox, 1986.
               Changed by C. Fox, 1990.
                  - made measure function a DFA
                  - restructured structs
                  - renamed functions and variables
                  - restricted function and variable scopes
               Changed by C. Fox, July, 1991.
                  - added ANSI C declarations 
                  - branch tested to 90% coverage

   Notes:      This code will make little sense without the the Porter
               article.  The stemming function converts its input to
               lower case.
**/

/************************   Standard Include Files   *************************/

/*****************************************************************************/
/*****************   Private Defines and Data Structures   *******************/

#define EOS                         '\0'

#define IsVowel(c)        ('a'==(c)||'e'==(c)||'i'==(c)||'o'==(c)||'u'==(c))

typedef struct {
           int id;                 /* returned if rule fired */
           char *old_end;          /* suffix replaced */
           char *new_end;          /* suffix replacement */
           int old_offset;         /* from end of word to start of suffix */
           int new_offset;         /* from beginning to end of new suffix */
           int min_root_size;      /* min root word size for replacement */
           int (*condition)();     /* the replacement test function */
           } RuleList;

#define LAMBDA ""        /* the constant empty string */

/*****************************************************************************/
/********************   Private Function Declarations   **********************/

#ifdef __STDC__

int WordSize( char *word );
int ContainsVowel( char *word, char **end );
int EndsWithCVC( char *word, char **end );
int AddAnE( char *word, char **end );
int RemoveAnE( char *word, char **end );
int ReplaceEnd( char **word, RuleList *rule ,char **end);

#else

int WordSize( /* word */ );
int ContainsVowel( /* word, end */ );
int EndsWithCVC( /* word, end */ );
int AddAnE( /* word, end */ );
int RemoveAnE( /* word, end */ );
int ReplaceEnd( /* word, rule, end */ );

#endif

/******************************************************************************/
/*****************   Initialized Private Data Structures   ********************/
/* 2/22/00 - added braces around each line */

static RuleList step1a_rules[] =
           {
             {101,  "sses",      "ss",    3,  1, -1,  NULL,},
             {102,  "ies",       "i",     2,  0, -1,  NULL,},
             {103,  "ss",        "ss",    1,  1, -1,  NULL,},
             {104,  "s",         LAMBDA,  0, -1, -1,  NULL,},
             {000,  NULL,        NULL,    0,  0,  0,  NULL,}
           };

static RuleList step1b_rules[] =
           {
             {105,  "eed",       "ee",    2,  1,  0,  NULL,},
             {106,  "ed",        LAMBDA,  1, -1, -1,  ContainsVowel,},
             {107,  "ing",       LAMBDA,  2, -1, -1,  ContainsVowel,},
             {000,  NULL,        NULL,    0,  0,  0,  NULL,}
           };

static RuleList step1b1_rules[] =
           {
             {108,  "at",        "ate",   1,  2, -1,  NULL,},
             {109,  "bl",        "ble",   1,  2, -1,  NULL,},
             {110,  "iz",        "ize",   1,  2, -1,  NULL,},
             {111,  "bb",        "b",     1,  0, -1,  NULL,},
             {112,  "dd",        "d",     1,  0, -1,  NULL,},
             {113,  "ff",        "f",     1,  0, -1,  NULL,},
             {114,  "gg",        "g",     1,  0, -1,  NULL,},
             {115,  "mm",        "m",     1,  0, -1,  NULL,},
             {116,  "nn",        "n",     1,  0, -1,  NULL,},
             {117,  "pp",        "p",     1,  0, -1,  NULL,},
             {118,  "rr",        "r",     1,  0, -1,  NULL,},
             {119,  "tt",        "t",     1,  0, -1,  NULL,},
             {120,  "ww",        "w",     1,  0, -1,  NULL,},
             {121,  "xx",        "x",     1,  0, -1,  NULL,},
             {122,  LAMBDA,      "e",    -1,  0, -1,  AddAnE,},
             {000,  NULL,        NULL,    0,  0,  0,  NULL,}
             };

static RuleList step1c_rules[] =
           {
             {123,  "y",         "i",      0,  0, -1,  ContainsVowel,},
             {000,  NULL,        NULL,    0,  0,  0,  NULL,}
           };

static RuleList step2_rules[] =
           {
             {203,  "ational",   "ate",   6,  2,  0,  NULL,},
             {204,  "tional",    "tion",  5,  3,  0,  NULL,},
             {205,  "enci",      "ence",  3,  3,  0,  NULL,},
             {206,  "anci",      "ance",  3,  3,  0,  NULL,},
             {207,  "izer",      "ize",   3,  2,  0,  NULL,},
             {208,  "abli",      "able",  3,  3,  0,  NULL,},
             {209,  "alli",      "al",    3,  1,  0,  NULL,},
             {210,  "entli",     "ent",   4,  2,  0,  NULL,},
             {211,  "eli",       "e",     2,  0,  0,  NULL,},
             {213,  "ousli",     "ous",   4,  2,  0,  NULL,},
             {214,  "ization",   "ize",   6,  2,  0,  NULL,},
             {215,  "ation",     "ate",   4,  2,  0,  NULL,},
             {216,  "ator",      "ate",   3,  2,  0,  NULL,},
             {217,  "alism",     "al",    4,  1,  0,  NULL,},
             {218,  "iveness",   "ive",   6,  2,  0,  NULL,},
             {219,  "fulnes",    "ful",   5,  2,  0,  NULL,},
             {220,  "ousness",   "ous",   6,  2,  0,  NULL,},
             {221,  "aliti",     "al",    4,  1,  0,  NULL,},
             {222,  "iviti",     "ive",   4,  2,  0,  NULL,},
             {223,  "biliti",    "ble",   5,  2,  0,  NULL,},
             {000,  NULL,        NULL,    0,  0,  0,  NULL,}
           };

static RuleList step3_rules[] =
           {
             {301,  "icate",     "ic",    4,  1,  0,  NULL,},
             {302,  "ative",     LAMBDA,  4, -1,  0,  NULL,},
             {303,  "alize",     "al",    4,  1,  0,  NULL,},
             {304,  "iciti",     "ic",    4,  1,  0,  NULL,},
             {305,  "ical",      "ic",    3,  1,  0,  NULL,},
             {308,  "ful",       LAMBDA,  2, -1,  0,  NULL,},
             {309,  "ness",      LAMBDA,  3, -1,  0,  NULL,},
             {000,  NULL,        NULL,    0,  0,  0,  NULL,}
           };

static RuleList step4_rules[] =
           {
             {401,  "al",        LAMBDA,  1, -1,  1,  NULL,},
             {402,  "ance",      LAMBDA,  3, -1,  1,  NULL,},
             {403,  "ence",      LAMBDA,  3, -1,  1,  NULL,},
             {405,  "er",        LAMBDA,  1, -1,  1,  NULL,},
             {406,  "ic",        LAMBDA,  1, -1,  1,  NULL,},
             {407,  "able",      LAMBDA,  3, -1,  1,  NULL,},
             {408,  "ible",      LAMBDA,  3, -1,  1,  NULL,},
             {409,  "ant",       LAMBDA,  2, -1,  1,  NULL,},
             {410,  "ement",     LAMBDA,  4, -1,  1,  NULL,},
             {411,  "ment",      LAMBDA,  3, -1,  1,  NULL,},
             {412,  "ent",       LAMBDA,  2, -1,  1,  NULL,},
             {423,  "sion",      "s",     3,  0,  1,  NULL,},
             {424,  "tion",      "t",     3,  0,  1,  NULL,},
             {415,  "ou",        LAMBDA,  1, -1,  1,  NULL,},
             {416,  "ism",       LAMBDA,  2, -1,  1,  NULL,},
             {417,  "ate",       LAMBDA,  2, -1,  1,  NULL,},
             {418,  "iti",       LAMBDA,  2, -1,  1,  NULL,},
             {419,  "ous",       LAMBDA,  2, -1,  1,  NULL,},
             {420,  "ive",       LAMBDA,  2, -1,  1,  NULL,},
             {421,  "ize",       LAMBDA,  2, -1,  1,  NULL,},
             {000,  NULL,        NULL,    0,  0,  0,  NULL,}
           };

static RuleList step5a_rules[] =
           {
             {501,  "e",         LAMBDA,  0, -1,  1,  NULL,},
             {502,  "e",         LAMBDA,  0, -1, -1,  RemoveAnE,},
             {000,  NULL,        NULL,    0,  0,  0,  NULL,}
           };

static RuleList step5b_rules[] =
           {
             {503,  "ll",        "l",     1,  0,  1,  NULL,},
             {000,  NULL,        NULL,    0,  0,  0,  NULL,}
           };

/*****************************************************************************/
/********************   Private Function Declarations   **********************/

/*FN***************************************************************************

       WordSize( word )

   Returns: int -- a weird count of word size in adjusted syllables

   Purpose: Count syllables in a special way:  count the number 
            vowel-consonant pairs in a word, disregarding initial 
            consonants and final vowels.  The letter "y" counts as a
            consonant at the beginning of a word and when it has a vowel
            in front of it; otherwise (when it follows a consonant) it
            is treated as a vowel.  For example, the WordSize of "cat" 
            is 1, of "any" is 1, of "amount" is 2, of "anything" is 3.

   Plan:    Run a DFA to compute the word size

   Notes:   The easiest and fastest way to compute this funny measure is
            with a finite state machine.  The initial state 0 checks
            the first letter.  If it is a vowel, then the machine changes
            to state 1, which is the "last letter was a vowel" state.
            If the first letter is a consonant or y, then it changes
            to state 2, the "last letter was a consonant state".  In
            state 1, a y is treated as a consonant (since it follows
            a vowel), but in state 2, y is treated as a vowel (since
            it follows a consonant.  The result counter is incremented
            on the transition from state 1 to state 2, since this
            transition only occurs after a vowel-consonant pair, which
            is what we are counting.
**/

int
WordSize( word )
   char *word;   /* in: word having its WordSize taken */
   {
   register int result;   /* WordSize of the word */
   register int state;    /* current state in machine */

   result = 0;
   state = 0;

                 /* Run a DFA to compute the word size */
   while ( EOS != *word )
      {
      switch ( state )
         {
         case 0: state = (IsVowel(*word)) ? 1 : 2;
                 break;
         case 1: state = (IsVowel(*word)) ? 1 : 2;
                 if ( 2 == state ) result++;
                 break;
         case 2: state = (IsVowel(*word) || ('y' == *word)) ? 1 : 2;
                 break;
         }
      word++;
      }

   return( result );

   } /* WordSize */

/*FN**************************************************************************

       ContainsVowel( word, end )

   Returns: int -- TRUE (1) if the word parameter contains a vowel,
            FALSE (0) otherwise.

   Purpose: Some of the rewrite rules apply only to a root containing
            a vowel, where a vowel is one of "aeiou" or y with a
            consonant in front of it.

   Plan:    Obviously, under the definition of a vowel, a word contains
            a vowel iff either its first letter is one of "aeiou", or
            any of its other letters are "aeiouy".  The plan is to
            test this condition.

   Notes:   None
**/

int
ContainsVowel( word, end )
   char *word;   /* in: buffer with word checked */
   char **end;   /* pointer to the end of the word */
   {

   if ( EOS == *word )
      return( FALSE );
   else
      return( IsVowel(*word) || (NULL != strpbrk(word+1,"aeiouy")) );


   } /* ContainsVowel */

/*FN**************************************************************************

       EndsWithCVC( word )

   Returns: int -- TRUE (1) if the current word ends with a
            consonant-vowel-consonant combination, and the second
            consonant is not w, x, or y, FALSE (0) otherwise.

   Purpose: Some of the rewrite rules apply only to a root with
            this characteristic.

   Plan:    Look at the last three characters.

   Notes:   None
**/

int
EndsWithCVC( word, end )
   char *word;   /* in: buffer with the word checked */
   char **end;   /* pointer to the end of the word */
   {
   int length;         /* for finding the last three characters */

   if ( (length = (int)strlen(word)) < 3 ) /* This was < 2 in original - Moseley 10/19/99 */
      return( FALSE );
   else
      {
      *end = word + length - 1;
      return(    (NULL == strchr("aeiouwxy",*(*end)--))      /* consonant */
              && (NULL != strchr("aeiouy",  *(*end)--))        /* vowel */
              && (NULL == strchr("aeiou",   *(*end)  )) );   /* consonant */
      }

   } /* EndsWithCVC */

/*FN**************************************************************************

       AddAnE( word )

   Returns: int -- TRUE (1) if the current word meets special conditions
            for adding an e.

   Purpose: Rule 122 applies only to a root with this characteristic.

   Plan:    Check for size of 1 and a consonant-vowel-consonant ending.

   Notes:   None
**/

int
AddAnE( word, end )
   char *word;
   char **end;        /* pointer to the end of the word */
   {

   return( (1 == WordSize(word)) && EndsWithCVC(word, end) );

   } /* AddAnE */

/*FN**************************************************************************

       RemoveAnE( word )

   Returns: int -- TRUE (1) if the current word meets special conditions
            for removing an e.

   Purpose: Rule 502 applies only to a root with this characteristic.

   Plan:    Check for size of 1 and no consonant-vowel-consonant ending.

   Notes:   None
**/

int
RemoveAnE( word, end )
   char *word;
   char **end;        /* pointer to the end of the word */
   {

   return( (1 == WordSize(word)) && !EndsWithCVC(word, end) );

   } /* RemoveAnE */

/*FN**************************************************************************

       ReplaceEnd( word, rule, end)

   Returns: int -- the id for the rule fired, 0 is none is fired

   Purpose: Apply a set of rules to replace the suffix of a word

   Plan:    Loop through the rule set until a match meeting all conditions
            is found.  If a rule fires, return its id, otherwise return 0.
            Connditions on the length of the root are checked as part of this
            function's processing because this check is so often made.

   Notes:   This is the main routine driving the stemmer.  It goes through
            a set of suffix replacement rules looking for a match on the
            current suffix.  When it finds one, if the root of the word
            is long enough, and it meets whatever other conditions are
            required, then the suffix is replaced, and the function returns.
**/

int
ReplaceEnd( word, rule, end )
   char **word;      /* in/out: buffer with the stemmed word */
                     /* Jose Ruiz 10/00: pass it as a reference and realloc*/
                     /* size if neccessary */
   RuleList *rule;    /* in: data structure with replacement rules */
   char **end;        /* pointer to the end of the word */
   {
   char *ending;   /* set to start of possible stemmed suffix */
   char tmp_ch;             /* save replaced character when testing */
   char *newword=NULL;

   while ( 0 != rule->id )
      {
      ending = *end - rule->old_offset;
      if ( (*word) <= ending )
         if ( 0 == strcmp(ending,rule->old_end) )
            {
            tmp_ch = *ending;
            *ending = EOS;
            if ( rule->min_root_size < WordSize(*word))
               if ( !rule->condition || (*rule->condition)((*word), end) )
                  {
                  newword=emalloc((int)strlen(*word) + (int)strlen(rule->new_end) + 1);
                  (void)strcpy( newword, *word );
                  (void)strcat( newword, rule->new_end );
                  *end = ending + rule->new_offset;
                  *end = newword + ((*end) - (*word));
                  efree(*word);
                  *word = newword;
                  break;
                  }
            *ending = tmp_ch;
            }
      rule++;
      }

   return( rule->id );

   } /* ReplaceEnd */

/*****************************************************************************/
/*********************   Public Function Declarations   **********************/

/*FN***************************************************************************

       Stem( word )

   Returns: int -- FALSE (0) if the word contains non-alphabetic characters
            and hence is not stemmed, TRUE (1) otherwise

   Purpose: Stem a word

   Plan:    Part 1: Check to ensure the word is all alphabetic
            Part 2: Run through the Porter algorithm
            Part 3: Return an indication of successful stemming

   Notes:   This function implements the Porter stemming algorithm, with
            a few additions here and there.  See:

               Porter, M.F., "An Algorithm For Suffix Stripping,"
               Program 14 (3), July 1980, pp. 130-137.

            Porter's algorithm is an ad hoc set of rewrite rules with
            various conditions on rule firing.  The terminology of
            "step 1a" and so on, is taken directly from Porter's
            article, which unfortunately gives almost no justification
            for the various steps.  Thus this function more or less
            faithfully refects the opaque presentation in the article.
            Changes from the article amount to a few additions to the
            rewrite rules;  these are marked in the RuleList data
            structures with comments.
**/

/* was Stem(), redefined and called ONLY from below - Moseley 10/17/99 */
int Stem_it( word )
   char **word;  /* in/out: the word stemmed */
   {
   int rule;    /* which rule is fired in replacing an end */
   char *end;   /* pointer to the end of the word */
   /*  Hack to make sure Stem() doesn't stem the word into nonexistence */
   char *saveword= (char *)estrdup(*word);

            /* Part 1: Check to ensure the word is all alphabetic */
   for ( end = (*word); *end != EOS; end++ )
      if ( !isalpha((int)((unsigned char)*end)) ) return( FALSE ); /* cast to int 2/22/00 */
      else *end = tolower( *end );
   end--;

                /*  Part 2: Run through the Porter algorithm */
   (void)ReplaceEnd( word, step1a_rules, &end );
   rule = ReplaceEnd( word, step1b_rules, &end );
   if ( (106 == rule) || (107 == rule) )
      (void)ReplaceEnd( word, step1b1_rules, &end );
   (void)ReplaceEnd( word, step1c_rules, &end );

   (void)ReplaceEnd( word, step2_rules, &end );

   (void)ReplaceEnd( word, step3_rules, &end );

   (void)ReplaceEnd( word, step4_rules, &end );

   (void)ReplaceEnd( word, step5a_rules, &end );
   (void)ReplaceEnd( word, step5b_rules, &end );

   /* Restore to original if stems to nothing - Moseley 10/17/99 */
   /* if ( !strcmp( word, "" ) ) strcpy( word, saveword ); */
   /* No. Now if stems to one or less characters. */
   if ( (int)strlen( *word ) < 2 ) 
      {
         efree(*word);
         *word=saveword;
      } else efree(saveword);

           /* Part 3: Return an indication of successful stemming */
   return( TRUE );

   } /* Stem_it */


/* This was added to continue to stem a word until it will stem no more. */
int Stem( word, lenword ) /* redefined - Moseley 10/17/99 */
   char **word;  /* in/out: the word stemmed */
   int *lenword;  /* in/out: the total length of the buffer of the word to be stemmed */
		  /* This value can be greater than the length of the word */
   {
    int retval;
    char *previous=NULL;
    char *savedword=NULL;

    if ( (int)strlen( *word ) < 3 ) return( TRUE ); /* might stem to null string */

    savedword=estrdup(*word);    /* We will work with a copy of the word */
    do {
        if(previous) efree(previous);
        previous = estrdup(savedword);
        retval = Stem_it( &savedword );
    } while ( strcmp( savedword, previous ) );
    efree(previous);

    /* Return values */
    if((*lenword)<((int)strlen(savedword)+1))
    {
       efree(*word);
       *word=savedword;
       *lenword=strlen(savedword);
    } else {
       strcpy((*word),savedword);
       efree(savedword);
    }
   return(retval); /* no return value prior to 2/22/00 */
   } /* Stem */

