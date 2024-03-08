#define _POSIX_C_SOURCE 200809L
#include "table.h"
#include "keyring.h"
#include "dmenu.h"
#include "../util/sexec.h"
#include "../util/fgets2.h"
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

struct op_row *
op_table_search (struct op_table *_table, const char _key[], char type)
{
	struct op_row *row;
	OP_TABLE_FOREACH(row, _table) {
		if ((row->type == type) && !strcmp(row->key, _key)) {
			return row;
		}
	}
	return NULL;
}

struct op_row *
op_table_search_empty_slot (struct op_table *_table)
{
	struct op_row *row;
	OP_TABLE_FOREACH(row, _table) {
		if (row->key[0] == '\0') {
			return row;
		}
	}
	return NULL;
}

void
op_table_save (struct op_table *_table, struct op_keyring *_kr, char const *_path, char const *_shmpath)
{
	int e; FILE *fp;
	pid_t pid;
	
	pid = sexec(NULL, &fp,
	    "openssl", "enc", "-e",
	    "-aes256",
	    "-pass", "env:OPENPWD_PASSWORD",
	    "-pbkdf2",
	    "-out", _path,
	    SEXEC_ENV("OPENPWD_PASSWORD",_kr->password),
	    NULL
	);
	
	e = fwrite(_table, sizeof(*_table), 1, fp);
	if (e != 1/*err*/) {
		op_keyring_delete(_shmpath);
		dfail(1, "Can't save password file");
	}
	fclose(fp);
	
	e = sexec_wait(pid);
	if (e/*err*/) {
		op_keyring_delete(_shmpath);
		dfailx(1, "Can't encrypt file");
	}
	
	return;
}

void
op_table_load (struct op_table *_table, struct op_keyring *_kr, char const *_path, char const *_shmpath)
{
	int	 e;
	FILE	*fp;
	struct	 stat st;
	pid_t	 pid;
	char	 pass[OP_MAX_PASS] = {0}, *pw;
	
	e = stat(_path, &st);
	if (e == -1 && errno == ENOENT) {
		
		pid = X_DMENU_PASS(&fp, NULL, "Type general password again"); {
			pw = fgets2(pass, sizeof(pass), fp);
			fclose(fp);
			e = sexec_wait(pid);
			if (e || !pw/*err*/) {
				dfailx(1, "Operation aborted by the user");
			}
		}
		
		e = strcmp(pass, _kr->password);
		if (e/*err*/) {
			dfailx(1, "Invalid OpenPwd password");
		}
		
		strncpy(_table->magic, OP_MAGIC, sizeof(_table->magic)-1);
		op_table_save(_table, _kr, _path, _shmpath);
	}
	
	pid = sexec(&fp, NULL,
	    "openssl", "enc", "-d",
	    "-aes256",
	    "-pass", "env:OPENPWD_PASSWORD",
	    "-pbkdf2",
	    "-in", _path,
	    SEXEC_ENV("OPENPWD_PASSWORD", _kr->password),
	    NULL
	);
	
	e = fread(_table, sizeof(*_table), 1, fp);
	if (e != 1/*err*/) {
		op_keyring_delete(_shmpath);
		dfailx(1, "Invalid OpenPwd password");
	}
	fclose(fp);
	
	e = sexec_wait(pid);
	if (e !=  0/*err*/) {
		dfailx(e, "Can't decrypt file (error code %i).", e);
	}
	
	e = strcmp(_table->magic, OP_MAGIC);
	if (e != 0/*err*/) {
		dfailx(1, "Not a valid upm password file.");
	}
	
	return;
}

void
op_table_list (struct op_table const *_table, FILE *_fp, char type)
{
	struct op_row const *row;
	
	OP_TABLE_FOREACH(row, _table) {
		if (row->key[0] != '\0' && type == 'p' && row->type == 'p') {
			fprintf(_fp, "%s\n", row->key);
		}
	}
}

void
op_table_print_keys_from_clipboard(struct op_table *_table, FILE *_ofp) {
	pid_t	 pid;
	FILE	*fp;
	char	 buffer[1024] = {0};
	char	*line,*c,*start;
	int	 e;
	struct	 op_row *row;
	
	pid = sexec(&fp, NULL, "xclip", "-selection", "clipboard", "-o", NULL);
	line = fgets(buffer, sizeof(buffer)-1, fp);
	fclose(fp);
	e = sexec_wait(pid);
	if (!line || e/*err*/) { return; }
	if ((c = strchr(line, '\r'))) *c = '\0';
	if ((c = strchr(line, '\n'))) *c = '\0';
	
	start = strstr(line, "://");
	if (!start) { return; }
	start += 3;
	
	c = strchr(start, '/');
	if (c) *c = '\0';
	
	OP_TABLE_FOREACH(row, _table) {
		if ((row->type == 'p') && strstr(row->key, start)) {
			fprintf(_ofp, "%s\n", row->key);
		}
	}
	
	return;
}
