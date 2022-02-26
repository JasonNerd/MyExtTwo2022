// 读取一个文件块file block = 2*512B = 1024B
#include "disk/disk.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
说明: 函数读入buf字符串, 对于字符串按splch分割(splch将变为0)
并把每一个字符串的首指针记录到cmd_argvs中, endch表示遇到endch就停止处理
返回值为一个表示argv数量的整数
*/
int parse_cmd(char buf[], char *cmd_argvs[], char splch, char endch) {
	int i, need_split = 0, arg_cnt = 0;
	// 对输入进行解析
	cmd_argvs[arg_cnt] = buf;
	++arg_cnt;
	for (i = 0; buf[i] != endch; i++) {	// fgets函数从命令窗口读入一行，换行符就是结束标志
		// ' ' or '/' means seperator
		if (buf[i] == splch) {
			buf[i] = 0;
			need_split = 1;	// need_split的标志是防止多个连续splch
		} else if (need_split) {
			cmd_argvs[arg_cnt] = buf + i;
			++arg_cnt;
			need_split = 0;
		}
	}
	buf[i] = 0; // 去掉endch
	return arg_cnt;
//	for (i = 0; i <= arg_cnt; i++) {
//		printf("arglen:%d, arg: %s\n", strlen(cmd_argvs[i]), cmd_argvs[i]);
//	}
}

int read_block(unsigned int blk_index, char *buf) {
	//读取一个文件块，包含两个磁盘块
	if (0 == disk_read_block(blk_index, buf) && 0 == disk_read_block(blk_index + 1, buf + DEVICE_BLOCK_SIZE))
		return 0;
	return -1;
}

int write_block(unsigned blk_index, char *buf) {
	if (disk_write_block(blk_index, buf) && disk_write_block(blk_index + 1, buf + DEVICE_BLOCK_SIZE))
		return 0;
	return -1;
}

void read_spblk(super_block *spblk) {
	// 读取超级块
	char buf[FILE_BLOCK_SIZE];
	read_block(0, buf);
	memcpy(spblk, buf, sizeof(super_block));
}

void write_spblk(super_block *spblk) {
	// 写超级块
	char buf[FILE_BLOCK_SIZE];
	memcpy(buf, spblk, sizeof(super_block));
	write_block(0, buf);
}

void write_inode_table(inode itb[]) {
	// 默认inode有1024 == INODE_MAX_NUM个, 存储在disk的第1(从0开始)~第32个数据块
	char *byte_ptr = (char *)itb;
	int i, it_blk_num = INODE_MAX_NUM * sizeof(inode) / FILE_BLOCK_SIZE, bias = 1;
	for (i = 0; i < it_blk_num; i++)
		write_block(i + bias, byte_ptr + i * FILE_BLOCK_SIZE);
}

void read_inode_table(inode itb[]) {
	// 默认inode有1024 == INODE_MAX_NUM个, 存储在disk的第1(从0开始)~第32个数据块
	char *byte_ptr = (char *)itb;
	int i, it_blk_num = INODE_MAX_NUM * sizeof(inode) / FILE_BLOCK_SIZE, bias = 1;
	for (i = 0; i < it_blk_num; i++)
		read_block(i + bias, byte_ptr + i * FILE_BLOCK_SIZE);
}
