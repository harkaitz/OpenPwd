#ifndef SEXEC_DMENU_H
#define SEXEC_DMENU_H

#include <sys/cdefs.h>
#include "../util/sexec.h"

__BEGIN_DECLS;

#define X_DMENU(_FP0,_FP1, PROMPT)      sexec(_FP0,_FP1, "dmenu", "-p", "OpenPwd 🔒 " PROMPT ":", "-l", "10", "-sf", "black", "-sb", "#FF8C00", NULL)
#define X_DMENU_PASS(_FP0,_FP1, PROMPT) sexec(_FP0,_FP1, "dmenu", "-p", "OpenPwd 🔓 " PROMPT ":", "-l", "10", "-sf", "black", "-sb", "#ea4335", "-nf", "black", "-nb", "black", NULL)
#define X_XCLIP_GET(_FP0)               sexec(_FP0,NULL, "xclip", "-o", NULL)
void	dfail(int _eval, char const _fmt[], ...);
void	dfailx(int _eval, char const _fmt[], ...);
void	dwarnx(int _eval, char const _fmt[], ...);


__END_DECLS;

#endif
