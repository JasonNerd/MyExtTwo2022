// ��ȡһ���ļ���file block = 2*512B = 1024B
#include "disk/disk.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
˵��: ��������buf�ַ���, �����ַ�����splch�ָ�(splch����Ϊ0)
����ÿһ���ַ�������ָ���¼��cmd_argvs��, endch��ʾ����endch��ֹͣ����
����ֵΪһ����ʾargv����������
*/
int parse_cmd(char buf[], char *cmd_argvs[], char splch, char endch) {
	int i, need_split = 0, arg_cnt = 0;
	// ��������н���
	cmd_argvs[arg_cnt] = buf;
	++arg_cnt;
	for (i = 0; buf[i] != endch; i++) {	// fgets����������ڶ���һ�У����з����ǽ�����־
		// ' ' or '/' means seperator
		if (buf[i] == splch) {
			buf[i] = 0;
			need_split = 1;	// need_split�ı�־�Ƿ�ֹ�������splch
		} else if (need_split) {
			cmd_argvs[arg_cnt] = buf + i;
			++arg_cnt;
			need_split = 0;
		}
	}
	buf[i] = 0; // ȥ��endch
	return arg_cnt;
//	for (i = 0; i <= arg_cnt; i++) {
//		printf("arglen:%d, arg: %s\n", strlen(cmd_argvs[i]), cmd_argvs[i]);
//	}
}

int read_block(unsigned int blk_index, char *buf) {
	//��ȡһ���ļ��飬�����������̿�
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
	// ��ȡ������
	char buf[FILE_BLOCK_SIZE];
	read_block(0, buf);
	memcpy(spblk, buf, sizeof(super_block));
}

void write_spblk(super_block *spblk) {
	// д������
	char buf[FILE_BLOCK_SIZE];
	memcpy(buf, spblk, sizeof(super_block));
	write_block(0, buf);
}

void write_inode_table(inode itb[]) {
	// Ĭ��inode��1024 == INODE_MAX_NUM��, �洢��disk�ĵ�1(��0��ʼ)~��32�����ݿ�
	char *byte_ptr = (char *)itb;
	int i, it_blk_num = INODE_MAX_NUM * sizeof(inode) / FILE_BLOCK_SIZE, bias = 1;
	for (i = 0; i < it_blk_num; i++)
		write_block(i + bias, byte_ptr + i * FILE_BLOCK_SIZE);
}

void read_inode_table(inode itb[]) {
	// Ĭ��inode��1024 == INODE_MAX_NUM��, �洢��disk�ĵ�1(��0��ʼ)~��32�����ݿ�
	char *byte_ptr = (char *)itb;
	int i, it_blk_num = INODE_MAX_NUM * sizeof(inode) / FILE_BLOCK_SIZE, bias = 1;
	for (i = 0; i < it_blk_num; i++)
		read_block(i + bias, byte_ptr + i * FILE_BLOCK_SIZE);
}
