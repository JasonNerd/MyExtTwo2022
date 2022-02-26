#ifndef EXT2_H
#define EXT2_H
/**
 * @file ext2.h
 * @author Refrain Kruso (you@domain.com)
 * @brief 支持EXT2文件系统的数据结构(data structure)和函数接口(function interface)
 * @version 0.1
 * @date 2022-02-25
 * @copyright Copyright (c) 2022
 * @details
 * 4MB的文件系统, 1KB大小的文件块, 一共4096块,
 * super_block: 超级块, 包含
 *      magic_num文件系统识别幻数
 *      free_inode_num空闲索引节点数1024(inode对应文件\文件夹)
 *      inode_map索引节点占用位图1024位 32*32
 *      free_blk_num空闲文件块数4096
 *      blk_map文件块占用位图
 */
#include <stdint.h>
// 一些必要的常数
#define TYPE_FILE 0
#define TYPE_DIR 1
#define INODE_MAX_NUM 1024
#define FILE_BLOCK_SIZE 1024
// 可实现的操作
#define FUNC_ls "ls"
#define FUNC_touch "touch"
#define FUNC_mkdir "mkdir"
#define FUNC_cp "cp"
#define FUNC_shut "shutdown"
// 错误类型
#define ARG_TOO_MANY 1


typedef struct spb {
	// super block: 4+(2+4*32)+(2+4*128) = 648B ---- 占用一个文件块
	uint32_t extfs_magic;       // 文件系统识别幻数
	uint16_t free_inode_num;    // 空闲节点数
	uint32_t inode_map[32];     // 索引节点位图
	uint16_t free_blk_num;      // 空闲文件块数
	uint32_t blk_map[128];      // 文件块占用位图
} super_block;

typedef struct index_node {
	// 索引节点, 仅支持直接索引
	uint16_t file_type; // directory of file
	uint16_t file_size; // 文件大小(B)
	uint32_t link;
	uint32_t blk_pointer[6];    // 支持直接索引，大小6KB的文件
} inode;

typedef struct directory {
	// 目录项，实现按名查找
	char fd_name[121];
	uint32_t inode_id;      // 文件/目录的inode index
	uint16_t file_type;     // 文件 or 目录
	uint8_t valid;          // 目录项是否有效, 提供检索依据
} dirent;

/**
 * @brief 初始化一个文件系统，主要是超级块的初始化
 * @return if success then return 0, otherwise -1
 */
int fsys_init();
int ls(int argc, char *argvs[]);
int mkdir(int argc, char *argvs[]);
int touch(int argc, char *argvs[]);
int cp(int argc, char *argvs[]);
#endif