# define _actsize 2500
# define _memsize 3000
# define _nstates 700
# define _nterms 95
# define _nprod 300
# define _nnonterm 150
# define _tempsize 700
# define _cnamsz 3500
# define _lsetsize 600
# define _wsetsize 400

# define _tbitset 6

int tbitset;  /* size of lookahed sets */
int nolook 0; /* flag to suppress lookahead computations */
struct looksets { int lset[ _tbitset ]; } ;
struct item { int *pitem; } ;

/* this file contains the definitions for most externally known data */

int nstate 0;		/* number of states */
struct item *pstate[_nstates];	/* pointers to the descriptions of the states */
int apstate[_nstates];	/* index to the actions for the states */
int tystate[_nstates];	/* contains type information about the states */
int stsize _nstates;	/* maximum number of states, at present */
int memsiz _memsize;	/* maximum size for productions and states */
int mem0[_memsize] ; /* production storage */
int *mem mem0;
int amem[_actsize];  /* action table storage */
int actsiz _actsize; /* action table size */
int memact 0;		/* next free action table position */
int nprod 1;	/* number of productions */
int *prdptr[_nprod];	/* pointers to descriptions of productions */
int prdlim _nprod ;  /* the maximum number of productions */
	/* levprd - productions levels to break conflicts */
int levprd[_nprod] {0,0};
  /* last two bits code associativity:
       0 = no definition
       1 = left associative
       2 = binary
       3 = right associative
     bit 04 is 1 if the production has an action
     the high 13 bits have the production level
     */
int nterms 0;	/* number of terminals */
int tlim _nterms ; /* the maximum number of terminals */
/*	the ascii representations of the terminals	*/
int extval 0;  /* start of output values */
struct sxxx1 {char *name; int value;} trmset[_nterms];
char cnames[_cnamsz];
int cnamsz _cnamsz;
char *cnamp;
int maxtmp _tempsize;	/* the size of the temp1 array */
int temp1[_tempsize]; /* temporary storage, indexed by terms + nterms or states */
int temp2[_nnonterm]; /* temporary storage indexed by nonterminals */
int trmlev[_nterms];	/* vector with the precedence of the terminals */
  /* The levels are the same as for levprd, but bit 04 is always 0 */
/* the ascii representations of the nonterminals */
struct sxxx2 { char *name; } nontrst[_nnonterm];
int ntlim _nnonterm ; /* limit to the number of nonterminals */
int indgo[_nstates];		/* index to the stored goto table */
int ***pres; /* vector of pointers to the productions yielding each nonterminal */
struct looksets **pfirst; /* vector of pointers to first sets for each nonterminal */
int *pempty 0 ; /* table of nonterminals nontrivially deriving e */
int nnonter -1;	/* the number of nonterminals */
int lastred 0;	/* the number of the last reduction of a state */
int ftable;		/* y.tab.c file */
int foutput;		/* y.output file */
int arrndx; /* used in the output of arrays on y.tab.c */
int zzcwset 0;
int zzpairs 0;
int zzgoent 0;
int zzgobest 0;
int zzacent 0;
int zzacsave 0;
int zznsave 0;
int zzclose 0;
int zzsrconf 0;
int zzrrconf 0;
char *ctokn;
int lineno 1; /* current input line number */
int peekc -1; /* look-ahead character */
int tstates[ _nterms ]; /* states generated by terminal gotos */
int ntstates[ _nnonterm ]; /* states generated by nonterminal gotos */
int mstates[ _nstates ]; /* chain of overflows of term/nonterm generation lists  */

struct looksets clset;
struct looksets lkst [ _lsetsize ];
int nlset 0; /* next lookahead set index */
int lsetsz _lsetsize; /* number of lookahead sets */

struct wset { int *pitem, flag, ws[_tbitset]; } wsets[ _wsetsize ];
int cwset;
int wssize _wsetsize;

int numbval;  /* the value of an input number */
int rflag 0;  /* ratfor flag */
int oflag 0;  /* optimization flag */

int ndefout 3;  /* number of defined symbols output */
int nerrors 0;	/* number of errors */
int fatfl 1;  	/* if on, error is fatal */
int machine;   /* has a number describing the machine */
