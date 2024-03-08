#define _POSIX_C_SOURCE 200809L
#include "keyring.h"
#include "dmenu.h"
#include "../util/sexec.h"
#include "../util/fgets2.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

void
op_keyring_load(struct op_keyring *_kr, char const *_path, bool _askpass)
{
	int fd, e;
	
	fd = shm_open(_path, O_RDONLY, 0600);
	if (fd == -1) {
		op_keyring_fill(_kr, _askpass);
		return;
	}
	e = read(fd, _kr, sizeof(*_kr));
	if (e != sizeof(*_kr)/*err*/) {
		op_keyring_fill(_kr, _askpass);
		return;
	}
	
	return;
}

void
op_keyring_save(struct op_keyring *_kr, char const *_path)
{
	int fd, e;
	
	fd = shm_open(_path, O_CREAT | O_RDWR, 0600);
	if (fd == -1/*err*/) {
		dfail(1, "Can't create shared memory");
	}
	e = ftruncate(fd, sizeof(*_kr));
	if (e == -1/*err*/) {
		dfail(1, "Can't set size of shared memory");
	}
	_kr->last_access = time(NULL);
	e = write(fd, _kr, sizeof(*_kr));
	if (e != sizeof(*_kr)/*err*/) {
		dfail(1, "Can't write to shared memory");
	}
	close(fd);
}

void
op_keyring_fill(struct op_keyring *_kr, bool _askpass)
{
	char	*pw;
	FILE	*fp;
	pid_t	 pid;
	int	 e;
	
	if (_askpass) {
		pid = X_DMENU_PASS(&fp, NULL, "General password");
		pw = fgets2(_kr->password, sizeof(_kr->password), fp);
		fclose(fp);
		e = sexec_wait(pid);
		if (!pw || e/*err*/) {
			dfailx(1, "Operation aborted by the user.");
		}
	}
	_kr->last_access = time(NULL);
}

void
op_keyring_delete(char const *_path)
{
	int e;
	
	e = shm_unlink(_path);
	if (e == -1 && errno != ENOENT/*err*/) {
		dfail(1, "Can't delete shared memory");
	}
	return;
}

void
op_keyring_store(struct op_keyring *_kr, char const *_key, char const *_user, char const *_pass)
{
	if (_key) {
		strncpy(_kr->st_key, _key, sizeof(_kr->st_key)-1);
	}
	if (_user) {
		strncpy(_kr->st_user, _user, sizeof(_kr->st_user)-1);
	}
	if (_pass) {
		strncpy(_kr->st_pass, _pass, sizeof(_kr->st_pass)-1);
	}
}
