#include <common.h>
#include <command.h>

static int do_vux(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	int i;
	puts("Hi world\n");
	for (i = 0; i < argc; i++)
		printf("%s\n", argv[i]);
	return 0;
}

U_BOOT_CMD(
	vux,	CONFIG_SYS_MAXARGS,	1,	do_vux,
	"Just want to say Hi Hi",
	"[args..]\n"
	"    - echo args to console; \\c suppresses newline"
);
