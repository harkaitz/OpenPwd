#ifndef OPENPWD_KEYRING_H
#define OPENPWD_KEYRING_H

#include "config.h"
#include <sys/cdefs.h>
#include <time.h>
#include <stdbool.h>

__BEGIN_DECLS;

struct op_keyring {
	time_t	last_access;
	char	password[OP_MAX_PASS];
	char	st_key[OP_MAX_PASS];
	char	st_user[OP_MAX_PASS];
	char	st_pass[OP_MAX_PASS];
} __attribute__((packed));

void	op_keyring_load(struct op_keyring *, char const *, bool);
void	op_keyring_save(struct op_keyring *, char const *);
void	op_keyring_fill(struct op_keyring *, bool);
void	op_keyring_delete(char const *);
void	op_keyring_store(struct op_keyring *, char const *,char const *,char const *);

__END_DECLS;

#endif
