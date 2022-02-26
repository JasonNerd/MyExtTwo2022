#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ext2.h"
#include "disk/disk.h"
#include "utils.h"

/*
文件系统初始化，步骤：
	1. 打开磁盘，读取超级块(0号文件块)
	2. 比较文件系统幻数，若不一致则需要初始化
		a. 设置文件系统幻数
		b. 1024个inode构成inode_table写入disk, 第一个inode分配给root根目录, super-block相应的设置inode-map
		c. super-block超级块占用1块, inode-table占用32块, super-block相应的设置block-map
		d. 另外superblock 的free block count 减去(32+1), free inode count 减去1
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
		// 将super-block, inode-table写入
		write_spblk(&supblk);
		write_inode_table(inode_table);
	}
	// init a new file system or read the old file system
	return 0;
}

int ls(int argc, char *argvs[]) {
	// ls main 列出main文件夹的内容
	printf("in func ls\n");
}

int mkdir(int argc, char *argvs[]) {
	// mkdir main 创建文件夹main
	printf("in func ls\n");
}

int touch(int argc, char *argvs[]) {
	// touch main 创建文件main
	printf("in func touch\n");
}

int cp(int argc, char *argvs[]) {
	// cp main me 将当前目录下的文件main复制到me文件夹下
	int i;
	printf("in func cp\n");
	for (i = 0; i < argc; i++) {
		printf("%s\n", argvs[i]);
	}
}