/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	"sym.h"
#include	"timeout.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<setjmp.h>
#include	<readline/readline.h>

UFD output = 2;
static BOOL beenhere = FALSE;
CHAR tmpout[20] = "/tmp/sh-";
FILEBLK stdfile;
FILE standin = &stdfile;
char* tempfile;

static void exfile(BOOL);

#ifdef BUILD_FSH
/* Completion support is only compiled into fsh (BUILD_FSH). */
#include	<dirent.h>

extern SYSTAB commands;

static int streq_prefix(const char *s, const char *prefix, size_t plen)
{
	while (plen--) {
		if (*s++ != *prefix++)
			return 0;
	}
	return 1;
}

static void lcp_update(char *lcp, const char *s)
{
	while (*lcp && *s && *lcp == *s) {
		lcp++;
		s++;
	}
	*lcp = 0;
}

static int complete_from_systab(SYSTAB tab, const char *prefix, size_t plen,
				char *first, size_t firstlen,
				char *lcp, size_t lcplen,
				int *matches)
{
	SYSPTR sp = tab;
	const char *s;
	while ((s = sp->sysnam) != 0) {
		if (plen && streq_prefix(s, prefix, plen)) {
			if (*matches == 0) {
				strncpy(first, s, firstlen);
				first[firstlen - 1] = 0;
				strncpy(lcp, s, lcplen);
				lcp[lcplen - 1] = 0;
			} else {
				lcp_update(lcp, s);
			}
			(*matches)++;
		}
		sp++;
	}
	return *matches;
}

static int is_word_sep(char c)
{
	/* Keep it simple: whitespace and common shell metacharacters. */
	return (c == ' ' || c == '\t' || c == '\n' || c == ';' || c == '|' ||
		c == '&' || c == '(' || c == ')' || c == '<' || c == '>');
}

static const char *fsh_expand_prompt(const char *ps)
{
	static char pbuf[128];
	char cwd[96];
	char *o = pbuf;
	char *e = pbuf + sizeof(pbuf) - 1;
	const char *s = ps ? ps : "";
	int have_cwd = 0;

	while (*s && o < e) {
		if (*s == '\\') {
			s++;
			if (!*s)
				break;
			if (*s == 'w') {
				if (!have_cwd) {
					if (getcwd(cwd, sizeof(cwd)) == NULL)
						strcpy(cwd, "?");
					have_cwd = 1;
				}
				{
					char *cp = cwd;
					while (*cp && o < e)
						*o++ = *cp++;
				}
				s++;
				continue;
			}
			/* Unknown escape: print literally. */
			if (o < e)
				*o++ = '\\';
			if (o < e)
				*o++ = *s++;
			continue;
		}
		*o++ = *s++;
	}
	*o = 0;
	return pbuf;
}

static int complete_files(const char *prefix, size_t plen, char *insert, size_t insert_len)
{
	char dirbuf[128];
	char basebuf[64];
	char first[64];
	char lcp[64];
	int matches = 0;
	DIR *d;
	struct dirent *de;
	const char *slash;
	size_t dirlen, baselen;
	struct stat st;
	char path[196];
	size_t i;

	if (plen == 0)
		return 0;

	slash = NULL;
	for (i = 0; i < plen; i++) {
		if (prefix[i] == '/')
			slash = prefix + i;
	}

	if (slash) {
		dirlen = (size_t)(slash - prefix);
		if (dirlen >= sizeof(dirbuf))
			return 0;
		memcpy(dirbuf, prefix, dirlen);
		dirbuf[dirlen] = 0;
		/* Special-case absolute root */
		if (dirlen == 0)
			strcpy(dirbuf, "/");
		prefix = slash + 1;
		plen = plen - (dirlen + 1);
	} else {
		strcpy(dirbuf, ".");
	}

	baselen = plen;
	if (baselen >= sizeof(basebuf))
		return 0;
	memcpy(basebuf, prefix, baselen);
	basebuf[baselen] = 0;

	d = opendir(dirbuf);
	if (!d)
		return 0;

	while ((de = readdir(d)) != NULL) {
		const char *n = de->d_name;
		if (n[0] == '.' && basebuf[0] != '.')
			continue;
		if (streq_prefix(n, basebuf, baselen)) {
			if (matches == 0) {
				strncpy(first, n, sizeof(first));
				first[sizeof(first) - 1] = 0;
				strncpy(lcp, n, sizeof(lcp));
				lcp[sizeof(lcp) - 1] = 0;
			} else {
				lcp_update(lcp, n);
			}
			matches++;
		}
	}
	closedir(d);

	if (matches == 0)
		return 0;

	if (matches == 1) {
		/* If it's a directory, append '/' */
		path[0] = 0;
		if (strcmp(dirbuf, ".") == 0) {
			strncpy(path, first, sizeof(path));
			path[sizeof(path) - 1] = 0;
		} else if (strcmp(dirbuf, "/") == 0) {
			strncpy(path, "/", sizeof(path));
			path[sizeof(path) - 1] = 0;
			strncat(path, first, sizeof(path) - strlen(path) - 1);
		} else {
			strncpy(path, dirbuf, sizeof(path));
			path[sizeof(path) - 1] = 0;
			strncat(path, "/", sizeof(path) - strlen(path) - 1);
			strncat(path, first, sizeof(path) - strlen(path) - 1);
		}
		if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
			char tmp[64];
			strncpy(tmp, first, sizeof(tmp));
			tmp[sizeof(tmp) - 1] = 0;
			strncat(tmp, "/", sizeof(tmp) - strlen(tmp) - 1);
			strncpy(first, tmp, sizeof(first));
			first[sizeof(first) - 1] = 0;
			strncpy(lcp, first, sizeof(lcp));
			lcp[sizeof(lcp) - 1] = 0;
		}
	}

	/* Insert the part after what was already typed */
	if (strlen(lcp) <= baselen)
		return 0;
	if (strlen(lcp) - baselen + 1 > insert_len)
		return 0;
	strcpy(insert, lcp + baselen);
	return (int)strlen(insert);
}

static int complete_commands(const char *prefix, size_t plen, char *insert, size_t insert_len)
{
	char first[64];
	char lcp[64];
	int matches = 0;
	const char *path;

	if (plen == 0)
		return 0;

	first[0] = 0;
	lcp[0] = 0;
	complete_from_systab(commands, prefix, plen, first, sizeof(first), lcp, sizeof(lcp), &matches);
	complete_from_systab(reserved, prefix, plen, first, sizeof(first), lcp, sizeof(lcp), &matches);

	/* Now scan $PATH for executables (best-effort; no X_OK filtering). */
	path = pathnod.namval ? pathnod.namval : defpath;
	while (path) {
		char dirbuf[128];
		size_t dlen = 0;
		DIR *d;
		struct dirent *de;
		const char *p = path;

		while (*p && *p != ':')
			p++;
		dlen = (size_t)(p - path);
		if (dlen == 0) {
			strcpy(dirbuf, ".");
		} else if (dlen < sizeof(dirbuf)) {
			memcpy(dirbuf, path, dlen);
			dirbuf[dlen] = 0;
		} else {
			dirbuf[0] = 0;
		}

		if (dirbuf[0]) {
			d = opendir(dirbuf);
			if (d) {
				while ((de = readdir(d)) != NULL) {
					const char *n = de->d_name;
					if (n[0] == '.' && prefix[0] != '.')
						continue;
					if (streq_prefix(n, prefix, plen)) {
						if (matches == 0) {
							strncpy(first, n, sizeof(first));
							first[sizeof(first) - 1] = 0;
							strncpy(lcp, n, sizeof(lcp));
							lcp[sizeof(lcp) - 1] = 0;
						} else {
							lcp_update(lcp, n);
						}
						matches++;
					}
				}
				closedir(d);
			}
		}

		path = (*p == ':') ? (p + 1) : 0;
	}

	if (matches == 0)
		return 0;

	if (strlen(lcp) > plen) {
		if (strlen(lcp) - plen + 1 > insert_len)
			return 0;
		strcpy(insert, lcp + plen);
		return (int)strlen(insert);
	}

	if (matches == 1) {
		/* Exact match already typed: insert a space */
		if (insert_len < 2)
			return 0;
		insert[0] = ' ';
		insert[1] = 0;
		return 1;
	}

	return 0;
}

static int fsh_complete(const char *buf, size_t len, size_t cursor,
				char *insert, size_t insert_len)
{
	/* Find the start of the current word. */
	size_t wstart = cursor;
	size_t first_nonws = 0;
	int in_first_word = 0;

	while (first_nonws < len && (buf[first_nonws] == ' ' || buf[first_nonws] == '\t'))
		first_nonws++;

	while (wstart > 0) {
		char c = buf[wstart - 1];
		if (is_word_sep(c))
			break;
		wstart--;
	}

	/* Determine if we are completing the first word (command position). */
	if (wstart == first_nonws)
		in_first_word = 1;

	if (cursor < wstart)
		return 0;

	/* Extract prefix (current word up to cursor) into a temporary buffer. */
	{
		char prefix[128];
		size_t plen = cursor - wstart;
		if (plen == 0 || plen >= sizeof(prefix))
			return 0;
		memcpy(prefix, buf + wstart, plen);
		prefix[plen] = 0;
		if (in_first_word)
			return complete_commands(prefix, plen, insert, insert_len);
		else
			return complete_files(prefix, plen, insert, insert_len);
	}
}

static char history[1024];
static char inbuf[256];
static unsigned int inleft;
static char *inptr;
static char ineof;

static int line_input(const char *prmpt)
{
	register int l;
	if (!isatty(standin->fdes))
		return -1;	/* Not a tty */
	do {
		l = rl_edit(standin->fdes, output,
			fsh_expand_prompt(prmpt),
			inbuf, 256);
		if (l >= 0) {
			inbuf[l] = '\n';
			inptr = inbuf;
			inleft = l + 1;
			ineof = 0;
		}
		else
			ineof = 1;
	} while(l == -2);
	/* 0 - EOF, 1+ buffer including \n */
	return l + 1;
}

int lineread(int fd, char *bufp, int len)
{
	register char *buf = bufp;
	int bias = 0;
	int r;
	if (fd == standin->fdes && inleft) {
		if (len <= inleft) {
			memcpy(buf, inptr, len);
			inleft -= len;
			inptr += len;
			return len;
		}
		memcpy(buf, inptr, inleft);
		len -= inleft;
		buf += inleft;
		bias = inleft;
		inleft = 0;
	}
	r = 0;
	if (!ineof)
		r = read(fd, buf, len);
	if (r >= 0)
		r += bias;
	return r;
}

#else
#define rl_hinit(x,y)
#define rl_set_complete(x)
#define line_input(x)	(-1)
#endif

int main(int c, const char *v[])
{
	register int rflag = ttyflg;

	/* initialise storage allocation */
	blokinit();

	stdsigs();

	setbrk(BRKINCR);
	addblok((POS) 0);

	rl_hinit(history, sizeof(history));
	rl_set_complete(fsh_complete);

	/* set names from userenv */
	/* sh_getenv can call error handlers so initialize the
	   subshell trap and if it fails (eg being passed a broken
	   environment) just carry on instead of entering hyperspace */
	if (setjmp(subshell) == 0)
		sh_getenv();

	/* look for restricted */
/*	if(c>0 && any('r', *v) ) { rflag=0 ;} */

	/* look for options */
	dolc = options(c, v);
	if (dolc < 2)
		flags |= stdflg;

	if ((flags & stdflg) == 0)
		dolc--;

	dolv = v + c - dolc;
	dolc--;

	/* return here for shell file execution */
	setjmp(subshell);

	/* number of positional parameters */
	assnum(&dolladr, dolc);
	cmdadr = (char *)dolv[0];

	/* set pidname */
	assnum(&pidadr, getpid());

	/* set up temp file names */
	settmp();

	/* default ifs */
	dfault(&ifsnod, sptbnl);

	if ((beenhere++) == FALSE) {	/* ? profile */
		if (*cmdadr == '-'
		    && (input = pathopen(nullstr, profile)) >= 0) {
			exfile(rflag);
			flags &= ~ttyflg;
			;
		}
		if (rflag == 0) {
			flags |= rshflg;
		}

		/* open input file if specified */
		if (comdiv) {
			estabf(comdiv);
			input = -1;
		} else {
			input = ((flags & stdflg) ? 0 : chkopen(cmdadr));
			comdiv--;
			;
		}
//      } else {        *execargs=(char *)dolv; /* for `ps' cmd */
		;
	}
	exfile(0);
	done();
}

static void exfile(BOOL prof)
{
	register L_INT mailtime = 0;
	register int userid;
	struct stat statb;

	/* move input */
	if (input > 0) {
		Ldup(input, INIO);
		input = INIO;
	}

	/* move output to safe place */
	if (output == 2) {
		Ldup(dup(2), OTIO);
		output = OTIO;
	}

	userid = getuid();

	/* decide whether interactive */
	if ((flags & intflg)
	    || ((flags & oneflg) == 0 && isatty(output) && isatty(input))) {
		dfault(&ps1nod, (userid ? stdprompt : supprompt));
		dfault(&ps2nod, readmsg);
		flags |= ttyflg | prompt;
		ignsig(KILL);
	} else {
		flags |= prof;
		flags &= ~prompt;
	}

	if (setjmp(errshell) && prof) {
		close(input);
		return;
	}

	/* error return here */
	loopcnt = breakcnt = peekc = 0;
	iopend = 0;
	if (input >= 0)
		initf(input);

	/* command loop */
	for (;;) {
		tdystak(0);
		stakchk();	/* may reduce sbrk */
		exitset();
		if ((flags & prompt) && standin->fstak == 0 && !eof) {
			if (mailnod.namval
			    && stat(mailnod.namval, &statb) >= 0
			    && statb.st_size
			    && (statb.st_mtime != mailtime)
			    && mailtime) {
				prs(mailmsg);
			}
			mailtime = statb.st_mtime;
			if (line_input(ps1nod.namval) < 0)
			{
				prs(ps1nod.namval);
				alarm(TIMEOUT);
			}
			flags |= waiting;
		}

		trapnote = 0;
		peekc = readc();
		if (eof)
			return;
		alarm(0);
		flags &= ~waiting;
		execute(cmd(NL, MTFLG), 0, NULL, NULL);
		eof |= (flags & oneflg);
	}
}

void chkpr(char eor)
{
	if ((flags & prompt) && standin->fstak == 0 && eor == NL) {
		if (line_input(ps2nod.namval) < 0)
			prs(ps2nod.namval);
	}
}

void settmp(void)
{
	itos(getpid());
	serial = 0;
	tempfile = movstr(numbuf, &tmpout[TMPNAM]);
}

void Ldup(register int fa, register int fb)
{
	dup2(fa, fb);
	close(fa);
	fcntl(fb, F_SETFD, FD_CLOEXEC);
}
