/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)error.c 1.4 3/31/82";

#include "whoami.h"
#include "0.h"
#ifndef PI1
#include "yy.h"
#endif

char	errpfx	= 'E';
extern	int yyline;
/*
 * Panic is called when impossible
 * (supposedly, anyways) situations
 * are encountered.
 * Panic messages should be short
 * as they do not go to the message
 * file.
 */
panic(s)
	char *s;
{

#ifdef DEBUG
#ifdef PI1
	printf("Snark (%s) line=%d\n", s, line);
	abort();
#else
	printf("Snark (%s) line=%d, yyline=%d\n", s, line, yyline);
	abort () ;	/* die horribly */
#endif
#endif
#ifdef PI1
	Perror( "Snark in pi1", s);
#else
	Perror( "Snark in pi", s);
#endif
	pexit(DIED);
}

/*
 * Error is called for
 * semantic errors and
 * prints the error and
 * a line number.
 */

/*VARARGS*/

error(a1, a2, a3, a4, a5)
	register char *a1;
{
	char errbuf[256]; 		/* was extern. why? ...pbk */
	register int i;

	if (errpfx == 'w' && opt('w') != 0) {
		errpfx = 'E';
		return;
	}
	Enocascade = 0;
	geterr(a1, errbuf);
	a1 = errbuf;
	if (line < 0)
		line = -line;
#ifndef PI1
	if (opt('l'))
		yyoutline();
#endif
	yysetfile(filename);
	if (errpfx == ' ') {
		printf("  ");
		for (i = line; i >= 10; i /= 10)
			pchr( ' ' );
		printf("... ");
	} else if (Enoline)
		printf("  %c - ", errpfx);
	else
		printf("%c %d - ", errpfx, line);
	printf(a1, a2, a3, a4, a5);
	if (errpfx == 'E')
#ifndef PI0
		eflg = TRUE, codeoff();
#else
		eflg = TRUE;
#endif
	errpfx = 'E';
	if (Eholdnl)
		Eholdnl = 0;
	else
		pchr( '\n' );
}

/*VARAGRS*/

cerror(a1, a2, a3, a4, a5)
{

	if (Enocascade)
		return;
	setpfx(' ');
	error(a1, a2, a3, a4, a5);
}

#ifdef PI1

/*VARARGS*/

derror(a1, a2, a3, a4, a5)
{

	if (!holdderr)
		error(a1, a2, a3, a4, a5);
	errpfx = 'E';
}

char	*lastname, printed, hadsome;

    /*
     *	this yysetfile for PI1 only.
     *	the real yysetfile is in yyput.c
     */
yysetfile(name)
	char *name;
{

	if (lastname == name)
		return;
	printed =| 1;
	gettime( name );
	printf("%s  %s:\n" , myctime( &tvec ) , name );
	lastname = name;
}
#endif
