// ��ȡһ���ļ���file block = 2*512B = 1024B
#include "disk.h"
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
}

int read_block(unsigned int blk_index, char *buf) {
	//��ȡһ���ļ��飬�����������̿�
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
	// ��ȡ������
	char buf[FILE_BLOCK_SIZE];
	memset(buf, 0, FILE_BLOCK_SIZE);
	read_block(0, buf);
	memcpy(spblk, buf, sizeof(super_block));
}

void write_spblk(super_block *spblk) {
	// д������
	char buf[FILE_BLOCK_SIZE];
	memset(buf, 0, FILE_BLOCK_SIZE);
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
		// ������ļ���, ��ô�������е�block
		int i, j;
		char buf[FILE_BLOCK_SIZE];
		dirent item;
		for (i = 0; i < 6; i++) {
			memset(buf, 0, FILE_BLOCK_SIZE);	// ÿ�ζ���Ҫ��0
			read_block(cur.blk_pointer[i], buf);
			// buf ��1KB�����ݿ�, ����Ŀ¼����, ÿ��blkװ����8��dirent
			for (j = 0; j < FILE_BLOCK_SIZE; j += sizeof(dirent)) {
				memcpy(&item, buf + j, sizeof(dirent));
				if (item.valid) { //��ǰĿ¼����Ч
					if (0 == strcmp(fldir, item.fd_name))
						// ����ƥ��ɹ�
						return item.inode_id;
					// �������е�valid��, ִ��void(*oprate_all(dirent)
					oprate_valid(&item);
				} else { //��ǰĿ¼����Ч
					// oprate_inval(&item);
					return -1; //ƥ��ʧ��
				}
			}
		}
		return -1; //ƥ��ʧ��
	} else
		return -1;
}

void op_empty(dirent *item) {
	return ;
}

void op_ls(dirent *item) {
	// ��ӡitem����Ϣ
	printf("%s\t\t", item->fd_name);
	if (item->file_type == TYPE_DIR)
		printf("dir\n");
	else
		printf("file\n");
}

// �ҵ���һ��dst bit, ���䷴ת, ������id
int reverse_bit(unsigned int bitmap[], int n, int dst) {
	int i, j;
	unsigned int tmp, cur_bit;
	for (i = 0; i < n; i++) {
		tmp = bitmap[i]; //tmp��32λ����
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

