#define _POSIX_C_SOURCE 200809L
#include "openpwd/table.h"
#include "openpwd/keyring.h"
#include "openpwd/dmenu.h"
#include "util/fgets2.h"
#include "util/rsalt.h"
#include <stdlib.h>
#include <unistd.h>

const char HELP[] =
    "Usage: openpwd MODIFIERS... COMMANDS..."                             "\n"
    ""                                                                    "\n"
    "Small password manager with redundant salt support. By default"      "\n"
    "passwords are saved in ~/" OP_TABLE_PATH " encrypted with aes256."   "\n"
    ""                                                                    "\n"
    "It is important to configure cron to clean up shared memory."        "\n"
    ""                                                                    "\n"
    "  5 * * * * <user> openpwd -c 3600 /clear"                           "\n"
    ""                                                                    "\n"
    "Modifiers:"                                                          "\n"
    ""                                                                    "\n"
    "    -c SECS  : Expire session if older than SECS seconds."           "\n"
    ""                                                                    "\n"
    "Commands:"                                                           "\n"
    ""                                                                    "\n"
    "    <NAME>      : Select a <NAME> credentials named <NAME> and put"  "\n"
    "                : user in 'primary' and password in 'clipboard'."    "\n"
    "    /select     : Ask user to select a credential."                  "\n"
    "    /user       : Print username of selected credential."             "\n"
    "    /pass[q]    : Print password of selected credential [Without salt].\n"
    "    /delete     : Delete selected credentials."                      "\n"
    "    /add K U P  : Add new credentials (Fail if it exists)."          "\n"
    "    /set K U P  : Add/replace credentials."                          "\n"
    "    /clear      : Quit session."                                     "\n"
    "    /import DIR : Import passwords from pass(1)"                     "\n"
    "    @PASS       : Return PASS next time /pass is called."            "\n"
    ""                                                                    "\n"
    ;

static char const *table_path = OP_TABLE_PATH;
static char const *keyring_path = OP_KEYRING_PATH;
static struct op_table table = {0};
static struct op_keyring keyring = {0};

int
main (int _argc, char **_argv)
{
	char	 buffer[1024] = {0};
	int	 opt, e;
	char	*salt,*name,*cmd,*user,*pass,*pass2,*home,*dir,*gpg, *file;
	struct	 op_row *row;
	FILE	*fp[2] = {NULL, NULL};
	pid_t	 pid;
	char    *opt_c = NULL;
	bool     is_select = false;
	
	if ((file = getenv("OPENPWD_FILE"))) {
		keyring_path = strdup(file);
		if (!keyring_path/*err*/) { err(1, "Can't allocate memory"); }
	}
	
	while ((opt = getopt (_argc, _argv, "hc:")) != -1) {
		switch (opt) {
		case '?':
		case 'h':
			fputs(HELP, stdout);
			return 0;
		case 'c':
			opt_c = optarg;
			break;
		default: return 1;
		}
	}
	
	/* Move to home directory. */
	home = getenv("HOME");
	if (!home/*err*/) {
		dfailx(1, "HOME not set");
	}
	e = chdir(home);
	if (e == -1/*err*/) {
		dfail(1, "Can't change to home directory");
	}
	
	/* Load keyring and table. */
	op_keyring_load(&keyring, keyring_path, true);
	op_table_load(&table, &keyring, table_path, keyring_path);
	op_keyring_save(&keyring, keyring_path);
	
	/* Get operation. */
 repeat:
	if (optind < _argc && !strcmp(_argv[optind], "/select")) {
		pid = X_DMENU(&fp[0], &fp[1], "");
		op_table_list(&table, fp[1], 'p');
		fputs("/add\n", fp[1]);
		fclose(fp[1]);
		cmd = fgets2(buffer, sizeof(buffer), fp[0]);
		fclose(fp[0]);
		e = sexec_wait(pid);
		if (e || !cmd/*err*/) {
			return 0; 
		}
		optind++;
		is_select = true;
	} else if (optind < _argc) {
		cmd = _argv[optind++];
		is_select = false;
	} else {
		pid = X_DMENU(&fp[0], &fp[1], "");
		op_table_print_keys_from_clipboard(&table, fp[1]);
		if (keyring.st_key[0] && keyring.st_user[0]) {
			fprintf(fp[1], "/user %s\n", keyring.st_key);
		}
		if (keyring.st_key[0] && keyring.st_pass[0]) {
			fprintf(fp[1], "/pass %s\n", keyring.st_key);
		}
		op_table_list(&table, fp[1], 'p');
		if (keyring.st_key[0]) {
			fprintf(fp[1], "/delete %s\n", keyring.st_key);
			fprintf(fp[1], "/clear\n");
		}
		fputs("/add\n", fp[1]);
		fclose(fp[1]);
		cmd = fgets2(buffer, sizeof(buffer), fp[0]);
		fclose(fp[0]);
		e = sexec_wait(pid);
		if (e || !cmd/*err*/) {
			return 0; 
		}
		is_select = false;
	}
	
	/* Perform operations. */
	if (!strncmp(cmd, "/user", 5))
	{
		printf("%s\n", keyring.st_user);
		op_keyring_store(&keyring, NULL, "", NULL);
		op_keyring_save(&keyring, keyring_path);
	}
	else if (!strncmp(cmd, "/pass", 5))
	{
		if (strcmp(cmd, "/passq")) {
			pid = X_DMENU(&fp[0], NULL, "Password salt");
			salt = fgets2(buffer, sizeof(buffer)-1, fp[0]);
			fclose(fp[0]);
			e = sexec_wait(pid);
			if (e/*err*/) {
				return 0;
			}
			
			if (salt && !rsalt_verify(salt)/*err*/) {
				char c = rsalt_end(salt, strlen(salt));
				dfailx(1, "Invalid salt, append %c to it", c);
			}
		}
		printf("%s%s\n", keyring.st_pass, (salt)?salt:"");
		op_keyring_store(&keyring, "", "", "");
		op_keyring_save(&keyring, keyring_path);
	}
	else if (!strncmp(cmd, "/delete", 7))
	{
		if (keyring.st_key[0] && (row = op_table_search(&table, keyring.st_key, 'p'))) {
			memset(row->key, 0, sizeof(row->key));
			memset(row->val[0], 0, sizeof(row->val[0]));
			memset(row->val[1], 0, sizeof(row->val[1]));
			op_table_save(&table, &keyring, table_path, keyring_path);
			op_keyring_store(&keyring, "", "", "");
			op_keyring_save(&keyring, keyring_path);
		}
	}
	else if (!strcmp(cmd, "/add") || !strcmp(cmd, "/set"))
	{
		if (optind < _argc) {
			name = _argv[optind++];
		} else {
			pid = X_DMENU(&fp[0], NULL, "Account name");
			name = fgets2(buffer, sizeof(buffer), fp[0]);
			fclose(fp[0]);
			e = sexec_wait(pid);
			if (e || !name/*err*/) {
				return 0;
			}
		}
		
		row = op_table_search(&table, name, 'p');
		if (row) {
			if (!strcmp(cmd, "/add")/*err*/) {
				dfailx(1, "A password named %s already exists", name);
			}
		}
		if (!row) {
			row = op_table_search_empty_slot(&table);
			if (!row/*err*/) {
				dfailx(1, "No empty slots");
			}
		}
		row->type = 'p';
		strncpy(row->key, name, sizeof(row->key)-1);
		
		if (optind < _argc) {
			user = _argv[optind++];
			strncpy(row->val[0], user, sizeof(row->val[0])-1);
			user[sizeof(row->val[0])-1] = '\0';
		} else {
			pid = X_DMENU(&fp[0], NULL, "Username");
			user = fgets2(row->val[0], sizeof(row->val[0]), fp[0]);
			fclose(fp[0]);
			e = sexec_wait(pid);
			if (e || !user/*err*/) {
				return 0;
			}
		}
		
		if (optind < _argc) {
			pass = _argv[optind++];
			strncpy(row->val[1], pass, sizeof(row->val[1])-1);
			pass[sizeof(row->val[1])-1] = '\0';
		} else {
			
			pid = X_DMENU_PASS(&fp[0], NULL, "New password");
			pass = fgets2(row->val[1], sizeof(row->val[1]), fp[0]);
			fclose(fp[0]);
			e = sexec_wait(pid);
			if (e || !pass/*err*/) {
				return 0;
			}
			
			pid = X_DMENU_PASS(&fp[0], NULL, "Repeat new password");
			pass2 = fgets2(buffer, sizeof(buffer), fp[0]);
			fclose(fp[0]);
			e = sexec_wait(pid);
			if (e || !pass2/*err*/) {
				return 0;
			}
			
			e = strcmp(pass, pass2);
			if (e/*err*/) {
				dfailx(1, "The passwords do not match.");
			}
		}
		op_table_save(&table, &keyring, table_path, keyring_path);
		op_keyring_store(&keyring, row->key, row->val[0], row->val[1]);
		op_keyring_save(&keyring, keyring_path);
	}
	else if (!strcmp(cmd, "/clear"))
	{
		op_keyring_load(&keyring, keyring_path, false);
		if (!opt_c || (keyring.last_access < time(NULL) - atoi(optarg))) {
			op_keyring_delete(keyring_path);
		}
		
	}
	else if (!strcmp(cmd, "/import"))
	{
		if (optind < _argc) {
			dir = _argv[optind++];
		} else {
			pid = X_DMENU(&fp[0], &fp[1], "Directory to import");
			if (getenv("PASSWORD_STORE_DIR")) {
				fprintf(fp[1], "%s\n", getenv("PASSWORD_STORE_DIR"));
			} else {
				fprintf(fp[1], "~/.password-store");
			}
			fclose(fp[1]);
			dir = fgets2(buffer, sizeof(buffer), fp[0]);
			fclose(fp[0]);
			e = sexec_wait(pid);
			if (e || !dir/*err*/) {
				return 0; 
			}
		}
		
		pid = sexec(
		    &fp[0], NULL,
		    "/bin/sh", "-e", "-c",
		    "cd \"$PASSWORD_STORE_DIR\"                                                   \n"
		    "IFS=\"\n\"                                                                   \n"
		    "for f in $(find . -type f -name '*.gpg' | sed 's|^\\./||;s|\\.gpg$||'); do   \n"
		    "    p=\"$(pass \"$f\" | head -n 1 || true)\"                                 \n"
		    "    if test -n \"$p\"; then                  \n"
		    "        echo \"s$f;$(basename \"$f\");$p\"   \n"
		    "    else                                     \n"
		    "        echo \"e$f\"                         \n"
		    "    fi                                       \n"
		    "done                                         \n",
		    SEXEC_ENV("PASSWORD_STORE_DIR", dir),
		    NULL
		);
		buffer[sizeof(buffer)-1] = '\0';
		while ((gpg = fgets2(buffer, sizeof(buffer), fp[0]))) {
			if (gpg[0]=='e'/*err*/) {
				dwarnx(1, "Failed importing %s", gpg+1);
				continue;
			}
			name=++gpg;
			
			user = strchr(name, ';');
			if (!user/*err*/) {
				dfailx(1, "Fatal error");
			}
			*(user++) = '\0';
			
			pass = strchr(user, ';');
			if (!pass/*err*/) {
				dfailx(1, "Fatal error");
			}
			*(pass++) = '\0';
			
			row = op_table_search(&table, name, 'p');
			if (!row) {
				row = op_table_search_empty_slot(&table);
				if (!row/*err*/) {
					dfailx(1, "No empty slots");
				}
			}
			row->type = 'p';
			strncpy(row->key, name, sizeof(row->key)-1);
			strncpy(row->val[0], user, sizeof(row->val[0])-1);
			strncpy(row->val[1], pass, sizeof(row->val[1])-1);
			fprintf(stderr, "Adding: %s : %s\n", name, user);
		}
		fclose(fp[0]);
		if (e || !dir/*err*/) {
			return 0; 
		}
		op_table_save(&table, &keyring, table_path, keyring_path);
	}
	else if (cmd[0] == '/')
	{
		dfailx(1, "Unsupported command");
	}
	else if (cmd[0] == '@')
	{
		op_keyring_store(&keyring, "", "", cmd+1);
		op_keyring_save(&keyring, keyring_path);
	}
	else
	{
		row = op_table_search(&table, cmd, 'p');
		if (!row/*err*/) {
			dfailx(1, "Password not found");
		}
		op_keyring_store(&keyring, row->key, row->val[0], row->val[1]);
		op_keyring_save(&keyring, keyring_path);
		if (!is_select) {
			pid = sexec(NULL, &fp[1], "xclip", "-i", "-selection", "primary", NULL);
			fputs(row->val[0], fp[1]);
			fclose(fp[1]);
			e = sexec_wait(pid);
			if (e/*err*/) {
				dfailx(1, "xclip returned code %i", e);
			}
			pid = sexec(NULL, &fp[1], "xclip", "-i", "-selection", "clipboard", NULL);
			fputs(row->val[1], fp[1]);
			fclose(fp[1]);
			e = sexec_wait(pid);
			if (e/*err*/) {
				dfailx(1, "xclip returned code %i", e);
			}   
		}
	}
	
	if (optind < _argc) {
		goto repeat;
	}
	
	return 0;
}


