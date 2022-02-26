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
 * @brief ά��shell�����У��������ж�ȡ���������ִ������
 * gcc main.c ext2.c ext2.h utils.c utils.h -o ext2filesys
 */


int main(int argc, char **argv) {
	// ��ӡ�ʺ���
	welcome();
	// ��ʼ��һ��buf��������������������
	fsys_init();
	static char buf[CMDLEN * ARGNMX];
	// ���main()����, �������������Ϊ(�ַ�����, ��������), �Ժ�����������ڴ˻����Ͻ���
	char *cmd_argvs[ARGNMX];	// �����б�
	int arg_cnt = 0;			// ��������
	while (1) {
		printf(">>>  ");
		memset(buf, 0, CMDLEN * ARGNMX);
		fgets(buf, CMDLEN * ARGNMX, stdin); // ��ȡ����������
		arg_cnt = parse_cmd(buf, cmd_argvs, ' ', '\n');
		// ע��Ƚ��ַ����Ƿ���Ȳ���ʹ�� ==, ��Ӧ�ø�Ϊstrcmp
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

