#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ext2.h"
#include "disk.h"
#include "utils.h"
// 根目录 or 当前工作目录文件夹的inode id
static int root_inode_id = 0;

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
	memset(&supblk, 0, sizeof(super_block));
	read_spblk(&supblk);
	// then we check the magic num stored in spb
	if (supblk.extfs_magic != 0x20001209) {
		// then the file system is empty or spoiled, needs initial
		supblk.extfs_magic = 0x20001209;
		// inode is 32B, we need 1024 inodes, 32 blocks
		inode inode_table[INODE_MAX_NUM];
		memset(inode_table, 0, sizeof(inode)*INODE_MAX_NUM);
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
	// ls: 从工作目录(根目录)开始, 寻访指定文件夹, 并列出该文件夹下的文件文件夹信息
	// 特点是: 不会向disk写内容, super-block, inode-table均不会发生改变
	if (argc > 2) {	// 参数过多
		disp_err(FUNC_ls, ARG_TOO_MANY);
		return -1;
	}
	// argvs[0]=ls argvs[1]=home/red
	int cur_id = root_inode_id;
	inode inode_table[INODE_MAX_NUM];
	memset(inode_table, 0, sizeof(inode)*INODE_MAX_NUM);
	read_inode_table(inode_table); // 读inode表
	int i;
	if (argc == 2) {
		// 从根目录开始寻访文件夹, 根目录的inode_id = 0
		char *path[ARGNMX] = {0}; // 不支持过深的文件夹, 注意全部初始化为空
		int path_depth = parse_cmd(argvs[1], path, '/', 0);
		for (i = 0; i < path_depth; i++) {
			cur_id = search_fldir(inode_table[cur_id], path[i], op_empty);
			if (-1 == cur_id) {
				// 说明查找失败
				disp_err(FUNC_ls, NOT_EXIST);
				return -1;
			}
		}
	}
	// 如果argc == 1, 那么cur_id = root_inode_id表示当前文件夹inode id
	// 如果argc == 2, 那么经过if(){}的查找, cur_id也已经表示当前文件夹inode id
	/** 下面就依据cur_id对当前文件夹内容进行展示 **/
	// 实际上这本身类似于search_fldir
	printf("name\t\ttype\n");
	search_fldir(inode_table[cur_id], "", op_ls);
	//文件名不可能为空, 因此遍历到所有的文件, 使用op_ls打印信息
}

int mkdir(int argc, char *argvs[]) {
	// mkdir: 创建文件夹, 先寻访至待创建文件夹的父目录,再在父目录下创建文件夹, 需要修改disk
	// 1. 分配一个空闲inode    2. 父目录分配一个空闲dirent    3. 修改super-block中的inode-map
	// 将数据部分(dirent)与管理部分(inode-map)所作修改写回disk
	if (argc != 2) {
		printf("recieve 2 arguments only!\n");
		return -1;
	}
	int cur_id = root_inode_id;
	// 读取super block 与 inode table
	super_block spblk;
	memset(&spblk, 0, sizeof(super_block));
	read_spblk(&spblk);
	inode inode_table[INODE_MAX_NUM];
	memset(inode_table, 0, sizeof(inode)*INODE_MAX_NUM);
	read_inode_table(inode_table);
	// 分割path
	char *path[ARGNMX] = {0};
	int path_depth = parse_cmd(argvs[1], path, '/', 0);
	int i, j;
	for (i = 0; i < path_depth - 1; i++) {
		cur_id = search_fldir(inode_table[cur_id], path[i], op_empty);
		if (-1 == cur_id) {
			// 说明查找失败
			disp_err(FUNC_ls, NOT_EXIST);
			return -1;
		}
	}
	// cur_id即为父文件夹inode id
	// 找到一个空闲dirent分配给新建的文件夹
	inode parent = inode_table[cur_id];
	dirent nwdir;
	int if_find = 0;
	char buf[FILE_BLOCK_SIZE];
	for (i = 0; i < 6; i++) {
		// 先判断inode下的blk_pointer有没有分配新块
		if (0 == parent.blk_pointer[i]) {
			parent.blk_pointer[i] = reverse_bit(spblk.blk_map, spblk.free_blk_num, 0);
			spblk.free_blk_num--;
			parent.file_size += FILE_BLOCK_SIZE;
		}
		//读入一个数据块
		memset(buf, 0, FILE_BLOCK_SIZE);
		read_block(parent.blk_pointer[i], buf);
		//开始寻找
		for (j = 0; j < FILE_BLOCK_SIZE; j += sizeof(dirent)) {
			memcpy(&nwdir, buf + j, sizeof(dirent));
			if (nwdir.valid == 0) {
				// 找到一个未用(待分配)的dirent
				nwdir.file_type = TYPE_DIR;
				// 为待分配的dirent提供索引
				nwdir.inode_id = reverse_bit(spblk.inode_map, INODE_MAX_NUM, 0);
				memset(&inode_table[nwdir.inode_id], 0, sizeof(inode));
				inode_table[nwdir.inode_id].file_type = TYPE_DIR;
				spblk.free_inode_num--;
				// 其他信息补充
				memcpy(nwdir.fd_name, path[path_depth - 1], sizeof(path[path_depth - 1]));
				nwdir.valid = 1;
				memcpy(buf + j, &nwdir, sizeof(dirent));
				if_find = 1;
				break;
			}
			if (nwdir.valid == 1 && nwdir.file_type == TYPE_DIR) {
				if (0 == strcmp(nwdir.fd_name, path[path_depth - 1])) {
					printf("dir already exist!\n");
					return -1;
				}
			}
		}
		//找到了就把更改写回disk并跳出循环
		if (if_find) {
			// 首先是super block
			write_spblk(&spblk);
			// 其次是inode-table
			memcpy(&inode_table[cur_id], &parent, sizeof(inode));
			write_inode_table(inode_table);
			// 最后数据块(父目录增加一个dirent)
			write_block(parent.blk_pointer[i], buf);
			break;
		}
	}
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

