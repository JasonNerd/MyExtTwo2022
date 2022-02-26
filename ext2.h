#ifndef EXT2_H
#define EXT2_H
/**
 * @file ext2.h
 * @author Refrain Kruso (you@domain.com)
 * @brief ֧��EXT2�ļ�ϵͳ�����ݽṹ(data structure)�ͺ����ӿ�(function interface)
 * @version 0.1
 * @date 2022-02-25
 * @copyright Copyright (c) 2022
 * @details
 * 4MB���ļ�ϵͳ, 1KB��С���ļ���, һ��4096��,
 * super_block: ������, ����
 *      magic_num�ļ�ϵͳʶ�����
 *      free_inode_num���������ڵ���1024(inode��Ӧ�ļ�\�ļ���)
 *      inode_map�����ڵ�ռ��λͼ1024λ 32*32
 *      free_blk_num�����ļ�����4096
 *      blk_map�ļ���ռ��λͼ
 */
#include <stdint.h>
// һЩ��Ҫ�ĳ���
#define TYPE_FILE 0
#define TYPE_DIR 1
#define INODE_MAX_NUM 1024
#define FILE_BLOCK_SIZE 1024
// ��ʵ�ֵĲ���
#define FUNC_ls "ls"
#define FUNC_touch "touch"
#define FUNC_mkdir "mkdir"
#define FUNC_cp "cp"
#define FUNC_shut "shutdown"
// ��������
#define ARG_TOO_MANY 1


typedef struct spb {
	// super block: 4+(2+4*32)+(2+4*128) = 648B ---- ռ��һ���ļ���
	uint32_t extfs_magic;       // �ļ�ϵͳʶ�����
	uint16_t free_inode_num;    // ���нڵ���
	uint32_t inode_map[32];     // �����ڵ�λͼ
	uint16_t free_blk_num;      // �����ļ�����
	uint32_t blk_map[128];      // �ļ���ռ��λͼ
} super_block;

typedef struct index_node {
	// �����ڵ�, ��֧��ֱ������
	uint16_t file_type; // directory of file
	uint16_t file_size; // �ļ���С(B)
	uint32_t link;
	uint32_t blk_pointer[6];    // ֧��ֱ����������С6KB���ļ�
} inode;

typedef struct directory {
	// Ŀ¼�ʵ�ְ�������
	char fd_name[121];
	uint32_t inode_id;      // �ļ�/Ŀ¼��inode index
	uint16_t file_type;     // �ļ� or Ŀ¼
	uint8_t valid;          // Ŀ¼���Ƿ���Ч, �ṩ��������
} dirent;

/**
 * @brief ��ʼ��һ���ļ�ϵͳ����Ҫ�ǳ�����ĳ�ʼ��
 * @return if success then return 0, otherwise -1
 */
int fsys_init();
int ls(int argc, char *argvs[]);
int mkdir(int argc, char *argvs[]);
int touch(int argc, char *argvs[]);
int cp(int argc, char *argvs[]);
#endif