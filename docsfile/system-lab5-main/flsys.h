#ifndef FLSYS_H_
#define FLSYS_H_
#include<stdint.h>

typedef struct super_block
{
    int32_t magic_num;        // 幻数
    int32_t free_block_count; // 空闲数据块数
    int32_t free_inode_count; // 空闲inode数
    int32_t dir_inode_count;  // 目录inode数
    uint32_t block_map[128];  // 数据块占用位图
    uint32_t inode_map[32];   // inode占用位图
} sp_block;

typedef struct inode
{
    uint32_t size;           // 文件大小
    uint16_t file_type;      // 文件类型（文件/文件夹）
    uint16_t link;           // 连接数
    uint32_t block_point[6]; // 数据块指针
} inode;

typedef struct dir_item
{                      // 目录项一个更常见的叫法是 dirent(directory entry)
    uint32_t inode_id; // 当前目录项表示的文件/目录的对应inode
    uint16_t valid;    // 当前目录项是否有效
    uint8_t type;      // 当前目录项类型（文件/目录）
    char name[121];    // 目录项表示的文件/目录的文件名/目录名
} dir_item;

// 读取数据块
int read_block(unsigned int block_num, char *buf);
// 写入数据块
int write_block(unsigned int block_num, char *buf);
// 读入inode_table
void read_inode(inode *inode_table);
// 写入inode_table
void write_inode(inode *inode_table);
// 读入spBlock
void read_sp(sp_block *spBlock);
// 写入spBlock
void write_sp(sp_block *spBlock);
// 找到空闲inode
int find_free_inode();
// 找到一个空闲块
int find_free_block();
// 初始化fs, 如果不是本文件系统，则初始化
void fs_init();
// 查找路径对应的inode_id
int find_inode(char *path);
// 列出文件
void ls(char *path);
// 创建文件
int create_file(char *path);
// 创建文件夹
int create_dir(char *path);
// 复制文件
void copy_file(char *from, char *to);
// 关闭系统
void shutdown();
#endif