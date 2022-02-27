// 读取一个文件块file block = 2*512B = 1024B
#include "disk.h"
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
}

int read_block(unsigned int blk_index, char *buf) {
	//读取一个文件块，包含两个磁盘块
	if (0 == disk_read_block(2 * blk_index, buf) && 0 == disk_read_block(2 * blk_index + 1, buf + DEVICE_BLOCK_SIZE))
		return 0;
	return -1;
}

int write_block(unsigned blk_index, char *buf) {
	if (disk_write_block(2 * blk_index, buf) && disk_write_block(2 * blk_index + 1, buf + DEVICE_BLOCK_SIZE))
		return 0;
	return -1;
}

void read_spblk(super_block *spblk) {
	// 读取超级块
	char buf[FILE_BLOCK_SIZE];
	memset(buf, 0, FILE_BLOCK_SIZE);
	read_block(0, buf);
	memcpy(spblk, buf, sizeof(super_block));
}

void write_spblk(super_block *spblk) {
	// 写超级块
	char buf[FILE_BLOCK_SIZE];
	memset(buf, 0, FILE_BLOCK_SIZE);
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
	memset(byte_ptr, 0, INODE_MAX_NUM * sizeof(inode));
	for (i = 0; i < it_blk_num; i++)
		read_block(i + bias, byte_ptr + i * FILE_BLOCK_SIZE);
}

void disp_err(char *func, int err) {
	printf("%s: ", func);
	if (err == ARG_TOO_MANY) {
		printf("too many aruments given!\n");
	} else if (err == NOT_EXIST) {
		printf("such file or path does not exist!\n");
	} else {
		printf("some unexpected error occured\n");
	}
}

int search_fldir(inode cur, char *fldir, void(*oprate_valid)(dirent *)) {
	if (cur.file_type == TYPE_DIR) {
		// 如果是文件夹, 那么遍历其中的block
		int i, j;
		char buf[FILE_BLOCK_SIZE];
		dirent item;
		for (i = 0; i < 6; i++) {
			memset(buf, 0, FILE_BLOCK_SIZE);	// 每次都需要清0
			read_block(cur.blk_pointer[i], buf);
			// buf 是1KB的数据块, 对于目录而言, 每个blk装入了8个dirent
			for (j = 0; j < FILE_BLOCK_SIZE; j += sizeof(dirent)) {
				memcpy(&item, buf + j, sizeof(dirent));
				if (item.valid) { //当前目录项有效
					if (0 == strcmp(fldir, item.fd_name))
						// 按名匹配成功
						return item.inode_id;
					// 对于所有的valid项, 执行void(*oprate_all(dirent)
					oprate_valid(&item);
				} else { //当前目录项无效
					// oprate_inval(&item);
					return -1; //匹配失败
				}
			}
		}
		return -1; //匹配失败
	} else
		return -1;
}

void op_empty(dirent *item) {
	return ;
}

void op_ls(dirent *item) {
	// 打印item的信息
	printf("%s\t\t", item->fd_name);
	if (item->file_type == TYPE_DIR)
		printf("dir\n");
	else
		printf("file\n");
}

// 找到第一个dst bit, 将其反转, 并返回id
int reverse_bit(unsigned int bitmap[], int n, int dst) {
	int i, j;
	unsigned int tmp, cur_bit;
	for (i = 0; i < n; i++) {
		tmp = bitmap[i]; //tmp是32位整数
		for (j = 0; j < 32; j++) {
			cur_bit = (tmp & 1 << (31 - j)) >> (31 - j);
			if (cur_bit == dst) {
				bitmap[i] ^= 1 << (31 - j);
				return i * 32 + j;
			}
		}
	}
	return -1;
}

