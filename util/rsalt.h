#ifndef RSALT_H
#define RSALT_H

#include <sys/cdefs.h>
#include <string.h>

static char const RSALT_CHARSET[] = "abcdefghijklmnopqrstuvxyz";

static inline char
rsalt_end(char const _s[], size_t _l)
{
	size_t r = 0, i;
	for (i = 0; i < _l && _s[i]; i++) {
		r += _s[i];
	}
	return RSALT_CHARSET[r%(sizeof(RSALT_CHARSET)-1)];
}

static inline bool
rsalt_verify(char const _s[])
{
	size_t l = strlen(_s);
	if (l == 0 || l == 1) {
		return false;
	}
	char c = rsalt_end(_s, l-1);
	if (c != _s[l-1]) {
		return false;
	}
	return true;
}

#endif
