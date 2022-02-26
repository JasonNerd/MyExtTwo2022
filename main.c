#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ext2.h"
#include "utils.h"
#include "disk/disk.h"
#define FUNC_ls "ls"
#define FUNC_touch "touch"
#define FUNC_mkdir "mkdir"
#define FUNC_cp "cp"
#define FUNC_shut "shutdown"
void welcome();

//char input_cmd[CMDLEN] = {0};
/**
 * @brief 维持shell的运行，从命令行读取命令，解析和执行命令
 * gcc main.c ext2.c ext2.h utils.c utils.h -o ext2filesys
 */


int main(int argc, char **argv) {
	// 打印问候语
	welcome();
	// 初始化一个buf缓冲区缓存命令行输入
	fsys_init();
	static char buf[CMDLEN * ARGNMX];
	// 类比main()函数, 将命令行输入变为(字符串组, 参数个数), 以后的所有命令在此基础上进行
	char *cmd_argvs[ARGNMX];	// 参数列表
	int arg_cnt = 0;			// 参数个数
	while (1) {
		printf(">>>  ");
		memset(buf, 0, CMDLEN * ARGNMX);
		fgets(buf, CMDLEN * ARGNMX, stdin); // 获取命令行输入
		arg_cnt = parse_cmd(buf, cmd_argvs, ' ', '\n');
		// 注意比较字符串是否相等不能使用 ==, 而应该改为strcmp
		if (0 == strcmp(cmd_argvs[0], FUNC_ls))
			ls(arg_cnt, cmd_argvs);
		else if (0 == strcmp(cmd_argvs[0], FUNC_touch))
			touch(arg_cnt, cmd_argvs);
		else if (0 == strcmp(cmd_argvs[0], FUNC_mkdir))
			mkdir(arg_cnt, cmd_argvs);
		else if (0 == strcmp(cmd_argvs[0], FUNC_cp))
			cp(arg_cnt, cmd_argvs);
		else if (0 == strcmp(cmd_argvs[0], FUNC_shut)) {
			close_disk();
			return 0;
		} else
			printf("%s not supported!\n", cmd_argvs[0]);
	}
	return 0;
}

void welcome() {
	printf("------------Welcome to MyExt2 FileSystem!----------\n");
	printf("------------------  Functions  ---------------------\n");
	printf("----    1. ls: list info about the given dir if exist [ls|ls home]\n");
	printf("----    2. mkdir: make a directory[mkdir home]\n");
	printf("----    3. touch: touch a file[touch readme]\n");
	printf("----    4. cp: copy a file to another dir[cp readme home]\n");
	printf("----    5. more function is expected then ... ...\n\n");
	printf("--------------------------------------------------\n\n\n");
}

