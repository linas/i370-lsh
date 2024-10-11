

#ifndef _defs_h_
#define _defs_h_

#include <stdio.h>		/* for the *FILE in some function defs    */
#include <time.h>

#define DEBUG
#ifdef  DEBUG			/* debug gives lots of garbage            */
#ifndef DIAGNOSE
#define DIAGNOSE		/* diagnose gives you the dump command    */
#endif
#endif

#define LFORCED   0x10
#define LTRUE     0x01
#define LFALSE    0x00

/* here are a couple of fd's : INBUF < INPUT < CMDV             */

#define MAXPATH  128
#define SYMBOLS  128
#define INBUF    128

#define TOKENS    64
#define INPUT    256
#define DRIVES    26		/* number of drives starting from a:      */
#define CPBUF    512		/* buffer for copying, independant        */

#define CMDV     512		/* starting size of vector passed to exec */
#define CMDVM  32768		/* maximum size -- arb -- can be upped    */
#define CMDC      64		/* initially provisions for 64 args       */
#define CMDCM   4096		/* but can go upto 4096 or more if u like */

#define PROZ     128		/* length of prompt                       */

#define DEFNICE      5		/* default increase in nice level         */
#define DEFTMOUT     0		/* quit after x seconds of inaction       */
#define DEFCOLS     80		/* some assumptions about screen size     */
#define DEFLINES    25		/* kinda like dos                         */
#define DEFPREVCHAR 23		/* default character recalling previous   */

#define OOALL   0x00		/* the of files to return in expansion    */
#define OOMIN   0x02
#define OOACC   0x04
#define OOPLA   0x08
#define OOREA   0x10
#define OOVIE   0x20

#define CALLDEP   64		/* the deapth of nested calls permitted   */

#define LVERSION  "MG-LSH Version 0.61 (And still buggy)"
#define DEFPROMPT "$N$G"

/* environment variable names                     */

#define STRSHLVL     "SHLVL"
#define STRPREVCHAR  "RECALL"
#define STRTESTTRUE  "TRUE"
#define STRUMASK     "UMASK"
#define STRPROMPT    "PROMPT"
#define STRALTPROMPT "prompt"
#define STRNICE      "NICE"
#define STRTMOUT     "TMOUT"
#define STRCOLS      "COLUMNS"
#define STRLINES     "LINES"
#define STRFUNNY     "FUNNY"
#define STRTERM      "TERM"

#define STRDIR       "DIRCMD"
#define STRCD        "CDCMD"
#define STRREN       "RENCMD"
#define STRDEL       "DELCMD"

#define STRNOFREEDOM "NOFREEDOM"
#define STRNOMAP     "NOMAP"
#define STRNOCHANGE  "NOCHANGE"
#define STRNOUNINIT  "NOUNINIT"
#define STRNOSET     "NOSET"
#define STRNOALIAS   "NOALIAS"
#define STRNOBATCH   "NOBATCH"
#define STRNOPATH    "NOROOT"
#define STRNOTRAP    "NOTRAP"

#define STRLSHOUT    "OUTPUT"
#define STRDEFLSHOUT "lsh.out"

#define PLBAD  0x01		/* bad command or file name   */
#define PLTMP  0x02		/* too many parameters        */
#define PLRPM  0x03		/* required parameter missing */
#define PLFNF  0x04		/* file not found             */
#define PLPNF  0x05		/* path not found             */
#define PLIND  0x06		/* invalid dir                */
#define PLMAE  0x07		/* memory allocation error    */
#define PLIDS  0x08		/* invalid drive specs        */
#define PLUCD  0x09		/* unable to create dir       */
#define PLIDN  0x0a		/* invalid path, not empty .. */
#define PLACD  0x0b		/* access denied              */
#define PLPFN  0x0c		/* parameter formant not corr */
#define PLOES  0x0d		/* outof env space            */
#define PLATR  0x0e		/* attempt to remove cur dir  */
#define PLSER  0x0f		/* syntax error               */
#define PLPAK  0x10		/* press any key              */
#define PLSRE  0x11		/* system resource exhausted  */
#define PLINS  0x12		/* insufficient disk space    */
#define PLRTF  0x13		/* read the manual          ! */
#define PLGFA  0x14		/* the general              ! */
#define PLISW  0x15		/* invalid switch           ! */
#define PLFEX  0x16		/* file exits               ! */
#define PLTER  0x17		/* terminal problems          */
#define PLINK  0xff		/* Serious error              */

/*#define DEBUG */

struct drive
  {
    char *path;			/* pointer to path        */
    int plen;			/* length of the path     */
    int slide;
    char valid;			/* is it currently mapped */
    char sonly;			/* is it search only      */
    char ronly;			/* is it read only        */
  };

struct sab
  {
    char id;			/* token id               */
    int ptr;			/* pointer into symstring */
    int len;			/* length of word         */

    char res;			/* reserved word          */
    char ins;			/* case insensitive       */
  };

struct ali
  {
    int len;
    char *aln;
    char *dfn;
    struct ali *next;
  };

#define LEM    0x00		/* empty   */

#define LCOP   0x01		/* copy    */
#define LCD    0x02		/* cd      */
#define LCLS   0x03		/* cls     */
#define LCTT   0x04		/* ctty  ! */
#define LDAT   0x05		/* date    */
#define LDIR   0x06		/* dir     */
#define LDEL   0x07		/* delete  */
#define LEXI   0x08		/* exit    */
#define LMD    0x09		/* makedir */
#define LPAT   0x0a		/* path    */
#define LPRO   0x0b		/* prompt  */
#define LRD    0x0c		/* rmdir   */
#define LREN   0x0d		/* rename  */
#define LSET   0x0e		/* set     */
#define LTIM   0x0f		/* time    */
#define LTYP   0x10		/* type    */
#define LVER   0x11		/* ver     */
#define LVOL   0x12		/* volume  */

 /* batch commands */

#define LCAL   0x13		/* call    */
#define LERR   0x14		/* errorl!v */
#define LFOR   0x15		/* for   ! */
#define LDO    0x16		/* do    ! */
#define LIN    0x17		/* in    ! */
#define LPAU   0x18		/* pause   */
#define LREM   0x19		/* remark  */
#define LIF    0x1a		/* if    ! */
#define LECH   0x1b		/* echo    */
#define LNOT   0x1c		/* not   ! */
#define LEST   0x1d		/* exist ! */

#define LDMP   0x2a		/* diagnostic -- remove in production */
#define LTRA   0x2b		/* trap    */
#define LALI   0x2c		/* alias   */
#define LUNA   0x2d		/* unalias */
#define LMAP   0x2e		/* map     */
#define LZAP   0x2f		/* unmap   */

#define LND    0x30		/* is request for new drive */
#define LLAB   0x31		/* is a label               */
#define LENV   0x32		/* is environment reference */
#define LSWI   0x33		/* is switch ie /w          */
#define LSTR   0x34		/* quouted string "gfd"     */
#define LELS   0x38		/* something else - command probably */

#define LPIP   0x50		/*      | */
#define LTO    0x51		/*      > */
#define LFRO   0x52		/*      < */
#define LAMP   0x53		/*      & */
#define LASS   0x54		/*      = */
#define LPLU   0x55		/*      + */
#define LSTA   0x56		/* star * */
#define LQES   0x57		/*      ? */
#define LSEP   0x58		/*      ; */
#define LTIL   0x59		/*      ~ */
#define LAT    0x5a		/* at   @ */
#define LBAN   0x5b		/* bang ! */

/* environment */

extern char **environ;

/* gobal data structures */

struct drive drivetab[DRIVES];	/* structure holding drive info */
struct sab symtab[SYMBOLS];	/* symbol info                  */
struct ali *head;		/* pointer to head of aliases   */

int drivecur;			/* current drive                */

char symstream[SYMBOLS * 8];
char instream[INPUT];

int tokenstream[TOKENS];
int tokencount;

int symtabcount;
int symstreamcount;
int symtabreserved;
int symstreamreserved;

char proz[PROZ];

/* state variables    */

int running;			/* when should we quit                  */
int calldep;			/* deapth of recursion                  */
int quitstat;			/* return value                         */
int doodle;			/* for fooling around                   */
int mailbytes;			/* bytes of mail                        */
int doexec;			/* for replacing shell                  */
int dobatch;			/* for doing jobs on logout             */
int niceval;			/* priority for batch jobs              */
int junior;			/* subprocess of a pipe                 */
int senior;			/* have we forked off a pipe            */
int testtrue;			/* what value true in conditionals      */
int funny;			/* how funny should we be ?             */

/* infor grabed from termcap */
int ccols;			/* columns of display                   */
int llines;			/* lines of display                     */
char *rsstr;			/* string to clear the screen           */
char *sostr;			/* string to enter standout             */
char *sestr;			/* string to exit standout              */
char *hostr;			/* string to redraw screen              */

char *k1str;			/* string for f1                        */
char *k3str;			/* string for f3                        */
char *kustr;			/* string for up                        */
char *krstr;			/* string for right                     */
char *klstr;			/* string for up                        */
char *kdstr;			/* string for right                     */

char *tsstr;			/* status line                          */
char *fsstr;			/* exit status line                     */

char prevchar;			/* character which recalls previous     */

long timeout;			/* when should the shell quit           */

time_t wtmpy;			/* detect some logon/out                */
time_t maily;			/* detect some change on mailbox        */

char *mistr;			/* pointer to mail string               */
char *lshout;			/* where should background output go    */

/* options for dir    */

char goptwide;			/* wide display                         */
char goptpage;			/* page                                 */
char goptcomp;			/* more complete listing                */
char goptbrie;			/* brief listing                        */
char goptall;			/* also files starting with .           */

/* options for del   */

char goptprom;			/* prompt for delete                   */

/* options for ren   */

char goptconf;			/* confirm rename                      */

/* degrees of freedom */

char nomap;			/* do not permit new mappings           */
char nochange;			/* no copy, delete, set, md, rd, rename */
char noset;			/* no path searches                     */
char noalias;			/* no new alias assignments or deletions */
char nobatch;			/* no background jobs                   */
char nopath;			/* no explicit unix paths '/' -> '-'    */
char notrap;			/* no signal trapping                   */

/* command variables  */

char *cmdv;
int cmdvc;
char **cmdc;
int cmdcc;

/* map.c    */

int pathinit ();

int map (int drive, char *str);
int zap (int drive);

int setlo (int drive, char *str);

int fromlo (char *str, int len);
int tolo (int drive, char *str, int len);

/* lex.c    */

int syminit ();
int lex (char *in, int tokens);

/* misc.c   */

int checkopt (int tc, int tk, int *tks);
int mangle (char *src, char *mask, char *dest, int destspace);
int countmail ();
int dcp (char *dest, char *src, char *mo);
int always ();
int introspect ();
int cmdrun (char *fname, FILE * out);
int plerr (int pcode);
int onquit ();

/* prompt.c */

int printprompt (int drive, char *promtstr, FILE * stream);

/* exp.c    */

int expr (char *foos, char *tos, int tlen, int opts);
int expcount (char *ptr, int i);
int expwild (char *ptr, int i);

/* inp.c    */

int readl (char *buf, int len);
int readc (FILE * in);

/* alias.c  */

int addalias (char *as, int al, char *ds, int dl);
int zapalias (char *as, int al);
int cleanalias ();
int findalias (char *ss, int sc, int sl);

/* parse.c  */

int stmts (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int stmt (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int leade (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int cmd (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int norma (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int reser (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int lcd (int tc, int tk, int *tks, FILE * in, FILE * out);	/* ! */
int lnd (int tc, int tk, int *tks, FILE * in, FILE * out);	/* . */
int lcop (int tc, int tk, int *tks, FILE * in, FILE * out);	/* ! */
int lcls (int tc, int tk, int *tks, FILE * in, FILE * out);	/* . */
int lcal (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int lctt (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int ldat (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int ldir (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int ldel (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int lexi (int tc, int tk, int *tks, FILE * in, FILE * out);	/* . */
int lech (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int lmd (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int lmap (int tc, int tk, int *tks, FILE * in, FILE * out);	/* ! */
int lpat (int tc, int tk, int *tks, FILE * in, FILE * out);	/* ! */
int lpro (int tc, int tk, int *tks, FILE * in, FILE * out);	/* ! */
int lpau (int tc, int tk, int *tks, FILE * in, FILE * out);	/* . */
int lrd (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int lren (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int lrem (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int lset (int tc, int tk, int *tks, FILE * in, FILE * out);	/* ! */
int ltim (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int ltyp (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int lver (int tc, int tk, int *tks, FILE * in, FILE * out);	/*   */
int lvol (int tc, int tk, int *tks, FILE * in, FILE * out);	/* ! */
int lzap (int tc, int tk, int *tks, FILE * in, FILE * out);	/* ! */
int lali (int tc, int tk, int *tks, FILE * in, FILE * out);	/* ! */
int luna (int tc, int tk, int *tks, FILE * in, FILE * out);	/* ! */
int ldmp (int tc, int tk, int *tks, FILE * in, FILE * out);	/* . */
int ltra (int tc, int tk, int *tks, FILE * in, FILE * out);	/* . */

/* used to be diagnostics... now used normally     */

int aliasdump (FILE * out);
int naliasdump (FILE * out);
int pathdump (FILE * out);

/* diagnostics */

#ifdef DIAGNOSE
int symdump ();
int tokendump ();
#endif

/* should be a define */

#define CEM(s,m)     (((s/m)+1)*m)

#endif
