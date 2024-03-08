#ifndef SEXEC_H
#define SEXEC_H
#ifndef _POSIX_C_SOURCE
#  error Please define _POSIX_C_SOURCE before sourcinf sexec.h
#endif


#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

__BEGIN_DECLS;

#define SEXEC_ENV(VAR,VAL) ((void*)0x01), VAR, VAL

static pid_t __attribute__((__unused__))
vsexec(FILE **_fp0, FILE **_fp1, char const _prog[], va_list _va)
{
	pid_t pid;
	int p1[2] = {-1, -1};
	int p0[2] = {-1, -1};
	int e;
	
	e = pipe(p0);
	if (e == -1/*err*/) {
		err(1, "Can't create pipe");
	}
	e = pipe(p1);
	if (e == -1/*err*/) {
		err(1, "Can't create pipe");
	}
	
	pid = fork();
	if (pid == -1/*err*/) {
		err(1, "Can't fork");
	}
	if (pid == 0) {
		int   argc = 1;
		char *argv[100] = {(char*)_prog, NULL};
		char *arg;
		while ((arg = va_arg(_va, char*))) {
			if (arg == (void*)0x01) {
				char *key = va_arg(_va, char*);
				char *val = va_arg(_va, char*);
				setenv(key, val, 1);//
			} else {
				argv[argc++] = arg;
			}
		}
		argv[argc] = NULL;
		dup2(p1[0], 0);
		dup2(p0[1], 1);
		close(p0[0]);
		close(p0[1]);
		close(p1[0]);
		close(p1[1]);
		execvp(_prog, argv);
		err(1, "Can't execute %s", _prog);
	}
	close(p0[1]);
	close(p1[0]);
	
	if (_fp0) {
		*_fp0 = fdopen(p0[0], "r");
		if (!*_fp0/*err*/) {
			err(1, "Can't open pipe");
		}
	} else {
		close(p0[0]);
	}
	
	if (_fp1) {
		*_fp1 = fdopen(p1[1], "w");
		if (!*_fp1/*err*/) {
			err(1, "Can't open pipe");
		}
	} else {
		close(p1[1]);
	}
	
	return pid;
}

static pid_t __attribute__((__unused__))
sexec(FILE **_f0, FILE **_fp1, char const _prog[], ...)
{
	va_list va;
	va_start(va, _prog);
	pid_t pid = vsexec(_f0, _fp1, _prog, va);
	va_end(va);
	return pid;
}

static int __attribute__((__unused__))
sexec_wait(pid_t _pid)
{
	int status, e;
	e = waitpid(_pid, &status, 0);
	if (e == -1/*err*/) {
		err(1, "Can't wait for %d", _pid);
	}
	return status;
}

__END_DECLS;

#endif
