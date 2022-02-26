#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ext2.h"
#include "disk/disk.h"
#include "utils.h"

/*
�ļ�ϵͳ��ʼ�������裺
	1. �򿪴��̣���ȡ������(0���ļ���)
	2. �Ƚ��ļ�ϵͳ����������һ������Ҫ��ʼ��
		a. �����ļ�ϵͳ����
		b. 1024��inode����inode_tableд��disk, ��һ��inode�����root��Ŀ¼, super-block��Ӧ������inode-map
		c. super-block������ռ��1��, inode-tableռ��32��, super-block��Ӧ������block-map
		d. ����superblock ��free block count ��ȥ(32+1), free inode count ��ȥ1
*/
int fsys_init() {
	// open the virtual disk file
	if (-1 == open_disk())
		return -1;
	// read the super block(default we put superblock at block 0)
	super_block supblk;
	read_spblk(&supblk);
	// then we check the magic num stored in spb
	if (supblk.extfs_magic != 0x20001209) {
		// then the file system is empty or spoiled, needs initial
		supblk.extfs_magic = 0x20001209;
		// inode is 32B, we need 1024 inodes, 32 blocks
		inode inode_table[INODE_MAX_NUM];
		// root directory need an inode;
		inode_table[0].file_type = TYPE_DIR;
		supblk.free_inode_num = INODE_MAX_NUM - 1;
		supblk.inode_map[0] = 0x1 << 31;	// little end store
		// update free block num, superblock-1, inodetable-32
		supblk.free_blk_num = get_disk_size() / FILE_BLOCK_SIZE - 1 - 32;
		supblk.blk_map[0] = 0xFFFFFFFF;
		supblk.blk_map[1] = 0x80000000;
		// ��super-block, inode-tableд��
		write_spblk(&supblk);
		write_inode_table(inode_table);
	}
	// init a new file system or read the old file system
	return 0;
}

int ls(int argc, char *argvs[]) {
	// ls main �г�main�ļ��е�����
	printf("in func ls\n");
}

int mkdir(int argc, char *argvs[]) {
	// mkdir main �����ļ���main
	printf("in func ls\n");
}

int touch(int argc, char *argvs[]) {
	// touch main �����ļ�main
	printf("in func touch\n");
}

int cp(int argc, char *argvs[]) {
	// cp main me ����ǰĿ¼�µ��ļ�main���Ƶ�me�ļ�����
	int i;
	printf("in func cp\n");
	for (i = 0; i < argc; i++) {
		printf("%s\n", argvs[i]);
	}
}