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
// 读写文件块(1KB)
int read_block(unsigned int blk_index, char *buf);
int write_block(unsigned blk_index, char *buf);
// 读写超级块(super block专用)
void read_spblk(super_block *spblk);
void write_spblk(super_block *spblk);
// 读写inode table专用
void write_inode_table(inode itb[]);
void read_inode_table(inode itb[]);
// 分割字符串, 分配指针
int parse_cmd(char buf[], char *cmd_argvs[], char splch, char endch);
// 展示错误
void disp_err(char *func, int err);
// 依据当前文件夹inode, 寻访当前文件夹所在block, 按名查找当前文件夹下是否
// 包含fldir文件夹/文件, 查找成功则返回其inode 的 id, 若为-1则表示失败
int search_fldir(inode cur, char *fldir, void(*oprate_valid)(dirent *));
void op_empty(dirent *item);
void op_ls(dirent *item);
// 在bitmap里搜索第一个等于dst的bit, 反转这一位并返回其位置
int reverse_bit(unsigned int bitmap[], int n, int dst);
#endif