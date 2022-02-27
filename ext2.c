#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ext2.h"
#include "disk.h"
#include "utils.h"
// ��Ŀ¼ or ��ǰ����Ŀ¼�ļ��е�inode id
static int root_inode_id = 0;

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
		// ��super-block, inode-tableд��
		write_spblk(&supblk);
		write_inode_table(inode_table);
	}
	// init a new file system or read the old file system
	return 0;
}

int ls(int argc, char *argvs[]) {
	// ls: �ӹ���Ŀ¼(��Ŀ¼)��ʼ, Ѱ��ָ���ļ���, ���г����ļ����µ��ļ��ļ�����Ϣ
	// �ص���: ������diskд����, super-block, inode-table�����ᷢ���ı�
	if (argc > 2) {	// ��������
		disp_err(FUNC_ls, ARG_TOO_MANY);
		return -1;
	}
	// argvs[0]=ls argvs[1]=home/red
	int cur_id = root_inode_id;
	inode inode_table[INODE_MAX_NUM];
	memset(inode_table, 0, sizeof(inode)*INODE_MAX_NUM);
	read_inode_table(inode_table); // ��inode��
	int i;
	if (argc == 2) {
		// �Ӹ�Ŀ¼��ʼѰ���ļ���, ��Ŀ¼��inode_id = 0
		char *path[ARGNMX] = {0}; // ��֧�ֹ�����ļ���, ע��ȫ����ʼ��Ϊ��
		int path_depth = parse_cmd(argvs[1], path, '/', 0);
		for (i = 0; i < path_depth; i++) {
			cur_id = search_fldir(inode_table[cur_id], path[i], op_empty);
			if (-1 == cur_id) {
				// ˵������ʧ��
				disp_err(FUNC_ls, NOT_EXIST);
				return -1;
			}
		}
	}
	// ���argc == 1, ��ôcur_id = root_inode_id��ʾ��ǰ�ļ���inode id
	// ���argc == 2, ��ô����if(){}�Ĳ���, cur_idҲ�Ѿ���ʾ��ǰ�ļ���inode id
	/** ���������cur_id�Ե�ǰ�ļ������ݽ���չʾ **/
	// ʵ�����Ȿ��������search_fldir
	printf("name\t\ttype\n");
	search_fldir(inode_table[cur_id], "", op_ls);
	//�ļ���������Ϊ��, ��˱��������е��ļ�, ʹ��op_ls��ӡ��Ϣ
}

int mkdir(int argc, char *argvs[]) {
	// mkdir: �����ļ���, ��Ѱ�����������ļ��еĸ�Ŀ¼,���ڸ�Ŀ¼�´����ļ���, ��Ҫ�޸�disk
	// 1. ����һ������inode    2. ��Ŀ¼����һ������dirent    3. �޸�super-block�е�inode-map
	// �����ݲ���(dirent)�������(inode-map)�����޸�д��disk
	if (argc != 2) {
		printf("recieve 2 arguments only!\n");
		return -1;
	}
	int cur_id = root_inode_id;
	// ��ȡsuper block �� inode table
	super_block spblk;
	memset(&spblk, 0, sizeof(super_block));
	read_spblk(&spblk);
	inode inode_table[INODE_MAX_NUM];
	memset(inode_table, 0, sizeof(inode)*INODE_MAX_NUM);
	read_inode_table(inode_table);
	// �ָ�path
	char *path[ARGNMX] = {0};
	int path_depth = parse_cmd(argvs[1], path, '/', 0);
	int i, j;
	for (i = 0; i < path_depth - 1; i++) {
		cur_id = search_fldir(inode_table[cur_id], path[i], op_empty);
		if (-1 == cur_id) {
			// ˵������ʧ��
			disp_err(FUNC_ls, NOT_EXIST);
			return -1;
		}
	}
	// cur_id��Ϊ���ļ���inode id
	// �ҵ�һ������dirent������½����ļ���
	inode parent = inode_table[cur_id];
	dirent nwdir;
	int if_find = 0;
	char buf[FILE_BLOCK_SIZE];
	for (i = 0; i < 6; i++) {
		// ���ж�inode�µ�blk_pointer��û�з����¿�
		if (0 == parent.blk_pointer[i]) {
			parent.blk_pointer[i] = reverse_bit(spblk.blk_map, spblk.free_blk_num, 0);
			spblk.free_blk_num--;
			parent.file_size += FILE_BLOCK_SIZE;
		}
		//����һ�����ݿ�
		memset(buf, 0, FILE_BLOCK_SIZE);
		read_block(parent.blk_pointer[i], buf);
		//��ʼѰ��
		for (j = 0; j < FILE_BLOCK_SIZE; j += sizeof(dirent)) {
			memcpy(&nwdir, buf + j, sizeof(dirent));
			if (nwdir.valid == 0) {
				// �ҵ�һ��δ��(������)��dirent
				nwdir.file_type = TYPE_DIR;
				// Ϊ�������dirent�ṩ����
				nwdir.inode_id = reverse_bit(spblk.inode_map, INODE_MAX_NUM, 0);
				memset(&inode_table[nwdir.inode_id], 0, sizeof(inode));
				inode_table[nwdir.inode_id].file_type = TYPE_DIR;
				spblk.free_inode_num--;
				// ������Ϣ����
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
		//�ҵ��˾ͰѸ���д��disk������ѭ��
		if (if_find) {
			// ������super block
			write_spblk(&spblk);
			// �����inode-table
			memcpy(&inode_table[cur_id], &parent, sizeof(inode));
			write_inode_table(inode_table);
			// ������ݿ�(��Ŀ¼����һ��dirent)
			write_block(parent.blk_pointer[i], buf);
			break;
		}
	}
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

