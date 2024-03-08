#ifndef OPENPWD_TABLE_H
#define OPENPWD_TABLE_H

#include "config.h"
#include <sys/cdefs.h>
#include <stdio.h>

__BEGIN_DECLS;

struct op_keyring;

struct op_row {
	char	type; /* p: name->password, d: name->domain */
	char	key[OP_MAX_PASS];
	char	val[2][OP_MAX_PASS];
} __attribute__((packed));

struct op_table {
	char	magic[32];
	struct	op_row rows[OP_TABLE_ROWS];
	char    padding[(OP_TABLE_SIZE) - sizeof(char)*OP_MAX_PASS - sizeof(struct op_row)*OP_TABLE_ROWS];
} __attribute__((packed));

struct op_row	*op_table_search(struct op_table *, const char [], char);
struct op_row	*op_table_search_empty_slot(struct op_table *);
void		 op_table_save(struct op_table *, struct op_keyring *, char const *, char const *);
void		 op_table_load(struct op_table *, struct op_keyring *, char const *, char const *);
void		 op_table_list(struct op_table const *, FILE *, char);
void		 op_table_print_keys_from_clipboard(struct op_table *, FILE *);

#define OP_TABLE_FOREACH(ROW,TABLE) for (ROW = (TABLE)->rows; row < (TABLE)->rows+OP_TABLE_ROWS; row++)

__END_DECLS;

#endif
