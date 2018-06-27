/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these libraries and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */
/* $XConsortium: timer.c /main/3 1996/06/19 17:13:50 drk $ */
/*
 * Simple timing program for regcomp().
 *
 *	Copyright (c) 1986 by University of Toronto.
 *	Written by Henry Spencer.  Not derived from licensed software.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to redistribute it freely,
 *	subject to the following restrictions:
 *
 *	1. The author is not responsible for the consequences of use of
 *		this software, no matter how awful, even if they arise
 *		from defects in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *		by explicit claim or by omission.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *		be misrepresented as being the original software.
 *
 * Usage: timer ncomp nexec nsub
 *	or
 *	timer ncomp nexec nsub regexp string [ answer [ sub ] ]
 *
 * The second form is for timing repetitions of a single test case.
 * The first form's test data is a compiled-in copy of the "tests" file.
 * Ncomp, nexec, nsub are how many times to do each regcomp, regexec,
 * and regsub.  The way to time an operation individually is to do something
 * like "timer 1 50 1".
 */
#include <stdio.h>

struct try {
	char *re, *str, *ans, *src, *dst;
} tests[] = {
#include "timer.t.h"
{ NULL, NULL, NULL, NULL, NULL }
};

#include <tptregexp.h>

int errreport = 0;		/* Report errors via errseen? */
char *errseen = NULL;		/* Error message. */

char *progname;

/* ARGSUSED */
main(argc, argv)
int argc;
char *argv[];
{
	int ncomp, nexec, nsub;
	struct try one;
	char dummy[512];

	if (argc < 4) {
		ncomp = 1;
		nexec = 1;
		nsub = 1;
	} else {
		ncomp = atoi(argv[1]);
		nexec = atoi(argv[2]);
		nsub = atoi(argv[3]);
	}
	
	progname = argv[0];
	if (argc > 5) {
		one.re = argv[4];
		one.str = argv[5];
		if (argc > 6)
			one.ans = argv[6];
		else
			one.ans = "y";
		if (argc > 7) {	
			one.src = argv[7];
			one.dst = "xxx";
		} else {
			one.src = "x";
			one.dst = "x";
		}
		errreport = 1;
		try(one, ncomp, nexec, nsub);
	} else
		multiple(ncomp, nexec, nsub);
	exit(0);
}

void
tpt_regerror(s)
char *s;
{
	if (errreport)
		errseen = s;
	else
		error(s, "");
}

#ifndef ERRAVAIL
error(s1, s2)
char *s1;
char *s2;
{
	fprintf(stderr, "regexp: ");
	fprintf(stderr, s1, s2);
	fprintf(stderr, "\n");
	exit(1);
}
#endif

int lineno = 0;

multiple(ncomp, nexec, nsub)
int ncomp, nexec, nsub;
{
	int i;
	extern char *strchr();

	errreport = 1;
	for (i = 0; tests[i].re != NULL; i++) {
		lineno++;
		try(tests[i], ncomp, nexec, nsub);
	}
}

try(fields, ncomp, nexec, nsub)
struct try fields;
int ncomp, nexec, nsub;
{
	regexp *r;
	char dbuf[BUFSIZ];
	int i;

	errseen = NULL;
	r = tpt_regcomp(fields.re);
	if (r == NULL) {
		if (*fields.ans != 'c')
			complain("tpt_regcomp failure in `%s'", fields.re);
		return;
	}
	if (*fields.ans == 'c') {
		complain("unexpected tpt_regcomp success in `%s'", fields.re);
		free((char *)r);
		return;
	}
	for (i = ncomp-1; i > 0; i--) {
		free((char *)r);
		r = tpt_regcomp(fields.re);
	}
	if (!tpt_regexec(r, fields.str)) {
		if (*fields.ans != 'n')
			complain("tpt_regexec failure in `%s'", "");
		free((char *)r);
		return;
	}
	if (*fields.ans == 'n') {
		complain("unexpected tpt_regexec success", "");
		free((char *)r);
		return;
	}
	for (i = nexec-1; i > 0; i--)
		(void) tpt_regexec(r, fields.str);
	errseen = NULL;
	for (i = nsub; i > 0; i--)
		tpt_regsub(r, fields.src, dbuf);
	if (errseen != NULL) {	
		complain("tpt_regsub complaint", "");
		free((char *)r);
		return;
	}
	if (strcmp(dbuf, fields.dst) != 0)
		complain("tpt_regsub result `%s' wrong", dbuf);
	free((char *)r);
}

complain(s1, s2)
char *s1;
char *s2;
{
	fprintf(stderr, "try: %d: ", lineno);
	fprintf(stderr, s1, s2);
	fprintf(stderr, " (%s)\n", (errseen != NULL) ? errseen : "");
}
