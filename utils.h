#ifndef UTILS_H
#define UTILS_H

#include "ext2.h"
#define CMDLEN 20
#define ARGNMX 5

/**
 * @brief read a file block(1KB) into the buf[>=1024]
 *
 * @param blk_index the index of the block
 * @param buf buffer
 * @return 0 means success and -1 means failure
 */
// ��д�ļ���(1KB)
int read_block(unsigned int blk_index, char *buf);
int write_block(unsigned blk_index, char *buf);
// ��д������(super blockר��)
void read_spblk(super_block *spblk);
void write_spblk(super_block *spblk);
// ��дinode tableר��
void write_inode_table(inode itb[]);
void read_inode_table(inode itb[]);
// �ָ��ַ���, ����ָ��
int parse_cmd(char buf[], char *cmd_argvs[], char splch, char endch);
// չʾ����
void disp_err(char *func, int err);
// ���ݵ�ǰ�ļ���inode, Ѱ�õ�ǰ�ļ�������block, �������ҵ�ǰ�ļ������Ƿ�
// ����fldir�ļ���/�ļ�, ���ҳɹ��򷵻���inode �� id, ��Ϊ-1���ʾʧ��
int search_fldir(inode cur, char *fldir, void(*oprate_valid)(dirent *));
void op_empty(dirent *item);
void op_ls(dirent *item);
// ��bitmap��������һ������dst��bit, ��ת��һλ��������λ��
int reverse_bit(unsigned int bitmap[], int n, int dst);
#endif