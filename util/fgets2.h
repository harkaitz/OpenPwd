
#ifndef FGETS2_H
#define FGETS2_H

#include <sys/cdefs.h>
#include <stdio.h>
#include <string.h>

__BEGIN_DECLS;

static inline char *
fgets2(char _b[], size_t _bsz, FILE *_fp)
{
	char *s,*cr,*nl;
	_b[_bsz-1] = '\0';
	s = fgets(_b, _bsz, _fp);
	if (!s || _b[_bsz-1])
		return NULL;
	if ((cr = strchr(s, '\r')))
		*cr = '\0';
	if ((nl = strchr(s, '\n')))
		*nl = '\0';
	if (s[0] == '\0')
		return NULL;
	return s;
}

__END_DECLS;

#endif
