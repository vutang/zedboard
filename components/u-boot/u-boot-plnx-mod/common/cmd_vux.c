#include <common.h>
#include <command.h>

static int do_vux(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	puts("Hi world\n");
	return 0;
}

U_BOOT_CMD(
	vux,	CONFIG_SYS_MAXARGS,	1,	do_vux,
	"Just want to say Hi Hi",
	"[args..]\n"
	"    - echo args to console; \\c suppresses newline"
);
