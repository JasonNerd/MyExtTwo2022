#ifndef FILSYS_H
#define FILSYS_H

#include<stdint.h>

/**macro definitions*/
#define MAX_INDOE_NUM 1024
#define BLOCK_SIZE 1024
#define MAX_FILE_NUM 6
#define TYPE_FOLD 1
#define TYPE_FILE 0
#define MAXCMDLINE 50
#define MAXCOMMAND 10
/**structures*/
//656Bytes
typedef struct super_block {
    int32_t magic_num;                  // 幻数
    int32_t free_block_count;           // 空闲数据块数
    int32_t free_inode_count;           // 空闲inode数
    int32_t dir_inode_count;            // 目录inode数
    uint32_t block_map[128];            // 数据块占用位图
    uint32_t inode_map[32];             // inode占用位图
} sp_block;
//32Bytes
typedef struct inode {
    uint32_t size;              // 文件大小
    uint16_t file_type;         // 文件类型（文件/文件夹）
    uint16_t link;              // 连接数
    uint32_t block_point[6];    // 数据块指针
}inode;
//128Bytes
typedef struct dir_item {               // 目录项一个更常见的叫法是 dirent(directory entry)
    uint32_t inode_id;          // 当前目录项表示的文件/目录的对应inode
    uint16_t valid;             // 当前目录项是否有效 
    uint8_t type;               // 当前目录项类型（文件/目录）
    char name[121];             // 目录项表示的文件/目录的文件名/目录名
}dir_item;

/** functions*/
void fs_init();
void welcome();
void spB_read(sp_block *superBlock);
void spB_write(sp_block *superBlock);
void blk_read(int blk_ind, char* buf);
void blk_write(int blk_ind, char* buf);
void it_get(inode* it);
void it_pull(inode* it);

int find_free_inode();
int find_free_block();
void ls(char *otherinfo);
void p_split(char* dirstr[MAX_FILE_NUM], char *otherinfo, char ch);
int get_inode_id(char* dirstr[MAX_FILE_NUM]);
void mkdir(char* otherinfo);


#endif