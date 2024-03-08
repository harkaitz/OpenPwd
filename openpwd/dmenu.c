#define _POSIX_C_SOURCE 200809L

#include "dmenu.h"
#include <errno.h>

void
dfail(int _eval, char const _fmt[], ...)
{
	va_list	 va1, va2;
	FILE	*fp;
	pid_t	 pid;
	
	va_start(va1, _fmt);
	va_copy(va2, va1);
	
	vwarn(_fmt, va2);
	pid = X_DMENU(NULL, &fp, ""); {
		fprintf(fp, "error (%i): ", errno);
		vfprintf(fp, _fmt, va1);
		fclose(fp);
		sexec_wait(pid);
	}
	
	va_end(va1);
	va_end(va2);
	
	exit(_eval);
}

void
dfailx(int _eval, char const _fmt[], ...)
{
	va_list	 va1, va2;
	FILE	*fp;
	pid_t	 pid;
	
	va_start(va1, _fmt);
	va_copy(va2, va1);
	
	vwarnx(_fmt, va2);
	pid = X_DMENU(NULL, &fp, ""); {
		fprintf(fp, "error: ");
		vfprintf(fp, _fmt, va1);
		fclose(fp);
		sexec_wait(pid);
	}
	
	va_end(va1);
	va_end(va2);
	
	exit(_eval);
}

void
dwarnx(int _eval, char const _fmt[], ...)
{
	va_list	 va1, va2;
	FILE	*fp;
	pid_t	 pid;
	
	va_start(va1, _fmt);
	va_copy(va2, va1);
	
	vwarnx(_fmt, va2);
	pid = X_DMENU(NULL, &fp, ""); {
		vfprintf(fp, _fmt, va1);
		fclose(fp);
		sexec_wait(pid);
	}
	
	va_end(va1);
	va_end(va2);
	
	return;
}
