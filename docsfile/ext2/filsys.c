#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include"filsys.h"
#include"disk.h"

#define FSID 0x20201209

// initialize the system when opening it.
void fs_init(){
    //some variables needed
    sp_block *superBlock;   //超级块
    inode inode_table[MAX_INDOE_NUM];    //索引节点表
    dir_item blk_buf[BLOCK_SIZE/sizeof(dir_item)];
    superBlock = (sp_block *)malloc(sizeof(sp_block));
    open_disk();
    spB_read(superBlock);
    welcome();
    if(superBlock->magic_num != FSID){
        //新开辟一个文件系统
        printf("Creating a new file system ... ...\n");
        superBlock->magic_num = FSID;
        //超级块1块 根目录1块 inode_table 32块
        superBlock->free_block_count = 4096 - 34;
        superBlock->free_inode_count = 1024 - 1;
        superBlock->dir_inode_count = 1;
        memset(superBlock->block_map, 0, sizeof(superBlock->block_map));
        memset(superBlock->inode_map, 0, sizeof(superBlock->inode_map));
        superBlock->block_map[0] = ~0;
        superBlock->block_map[1] = 0xc0000000;
        superBlock->inode_map[0] = 0x80000000;
        spB_write(superBlock);
        //第一个inode以及对应的block
        memset(inode_table, 0, sizeof(inode_table));
        inode_table[0].block_point[0] = 33;
        inode_table[0].file_type = TYPE_FOLD;
        inode_table[0].size = 0;
        inode_table[0].link = 0;
        memset(blk_buf, 0, sizeof(blk_buf));
        blk_buf[0].inode_id=0;
        blk_write(33, (char *)blk_buf);
        it_pull(inode_table);
    }
    return ; 
}

void welcome(){
    printf("\nWelcome to the Youth edition of ext2 file system!(powered by 180110704)\n");
    printf("Some principles you may follow:\n");
    printf("\t1.Commands supported\n");
    printf("\t\t(1) ls\n");
    printf("\t\t(2) touch [filename](no more than 121 chars)\n");
    printf("\t\t(3) mkdir [dirname]\n");
    printf("\t\t(4) cp [srcpath] [destpath]\n");
    printf("\t2. In one directory,at most 6 file or directory is permmitted.\n");
    printf("\t3. In this file system, 6KB is the max size of one sigle file.\n");
    printf("Now you can enjoy your time!\n\n");
}
//读superBlock
void spB_read(sp_block *superBlock){
    char buf[BLOCK_SIZE];
    char *p;
    blk_read(0, buf);
    p = (char *)superBlock;
    memcpy(p, buf, sizeof(sp_block));
}

//写超级块
void spB_write(sp_block *superBlock){
    char buf[BLOCK_SIZE];
    char *p;
    blk_read(0, buf);
    p = (char *)superBlock;
    memcpy(buf, p, sizeof(sp_block));
    blk_write(0, buf);
}

//读数据块，注意到磁盘块是数据块的一半
void blk_read(int blk_ind, char* buf){
    int disk_ind = 2*blk_ind;
    disk_read_block(disk_ind, buf);
    disk_read_block(disk_ind+1, buf+DEVICE_BLOCK_SIZE);
}

//写数据块，注意到磁盘块是数据块的一半
void blk_write(int blk_ind, char* buf){
    int disk_ind = 2*blk_ind;
    disk_write_block(disk_ind, buf);
    disk_write_block(disk_ind+1, buf+DEVICE_BLOCK_SIZE);
}


//从disk读入数据到inode_table
void it_get(inode* it){
    for(int i=1; i<32; i++){
        blk_read(i, (char*)&(it[32*(i-1)]));
    }
}

//把inode_table的数据写入到disk
void it_pull(inode* it){
    for(int i=1; i<32; i++){
        blk_write(i, (char*)&(it[32*(i-1)]));
    }
}

//寻找空闲的inode
int find_free_inode(){
    sp_block* superBlock;
    superBlock = (sp_block*)malloc(sizeof(sp_block));
    spB_read(superBlock);
    uint32_t tmp, bit, res_ind=-1;
    for(int i=0; i<32; i++){
        tmp = superBlock->inode_map[i];
        for(int j=0; j<32; j++){
            bit = tmp & ((uint32_t)1 << (31-j));
            if(0 == bit){
                res_ind = 32*i+j;
                superBlock->inode_map[i] = tmp | ((uint32_t)1 << (31-j));
                break;  //找到了一个
            } 
        }
        if(0 == bit)
            break;
    }
    superBlock->free_inode_count--;
    spB_write(superBlock);
    return res_ind;
}

//寻找空闲块
int find_free_block(){
    sp_block* superBlock;
    superBlock = (sp_block*)malloc(sizeof(sp_block));
    spB_read(superBlock);
    uint32_t tmp, bit, res_ind=-1;
    for(int i=0; i<128; i++){
        tmp = superBlock->block_map[i];
        for(int j=0; j<32; j++){
            bit = tmp & ((uint32_t)1 << (31-j));
            if(0 == bit){
                res_ind = 32*i+j;
                superBlock->block_map[i] = tmp | ((uint32_t)1 << (31-j));
                break;  //找到了一个
            } 
        }
        if(0 == bit)
            break;
    }
    superBlock->free_block_count--;
    spB_write(superBlock);
    return res_ind;
}

//ls命令，列出指定目录下所有的文件（含目录）
void ls(char *otherinfo){
    char* dirstr[MAX_FILE_NUM];
    p_split(dirstr, otherinfo, '/');
    int id = get_inode_id(dirstr), blk_ind, i, j;
    if(id<0){
        printf("error path!\n");
        return ;
    }
    inode it[MAX_INDOE_NUM], root;
    dir_item blk_buf[BLOCK_SIZE/sizeof(dir_item)];
    it_get(it);
    root = it[id];
    for(i=0; i<MAX_FILE_NUM; i++){
        blk_ind = root.block_point[i];
        if(blk_ind>0){
            memset(blk_buf, 0, BLOCK_SIZE);
            blk_read(blk_ind, (char*)blk_buf);
            for(j=0; j<(BLOCK_SIZE/sizeof(dir_item)); j++){
                printf("%s\t", blk_buf[j].name);
                if(blk_buf[j].type == TYPE_FOLD)
                    printf("dir\n");
                else
                    printf("file\n");
            }
        }
    }
}

void p_split(char* dirstr[MAX_FILE_NUM], char *otherinfo, char ch){
    int i, j, n = strlen(otherinfo);
    for(j=0; j<MAX_FILE_NUM; j++){
        dirstr[j]=0;
    }
    j = 0;
    dirstr[j] = otherinfo;
    for(i=0; i<n; i++){
        if(otherinfo[i]==ch){
            otherinfo[i]=0;
            dirstr[++j] = otherinfo+i+1;
        }
    }
}

//查找otherinfo对应dirent的inode
int get_inode_id(char* dirstr[MAX_FILE_NUM]){
    int i, j, k=0, id=0, flag=0, blk_ind;
    char* p;
    inode it[MAX_INDOE_NUM], root;
    dir_item blk_buf[BLOCK_SIZE/sizeof(dir_item)];
    it_get(it);
    while(dirstr[k]){
        root = it[id];
        p = dirstr[k];
        flag = 0;
        for(i=0; i<MAX_FILE_NUM; i++){
            blk_ind = root.block_point[i];
            if(blk_ind>0){
                memset(blk_buf, 0, BLOCK_SIZE);
                blk_read(blk_ind, (char*)blk_buf);
                for(j=0; j<(BLOCK_SIZE/sizeof(dir_item)); j++){
                    if(blk_buf[j].type == TYPE_FOLD && (strcmp(blk_buf[j].name, p) == 0)){
                        if(blk_buf[j].valid){
                            id = blk_buf[j].inode_id;
                            flag = 1;
                        }
                    }
                }
            }
        }
        if(!flag)//说明当前目录下无指定项
            return -1;
        k++;
    }
    return id;
}

//mkdir命令创建一个目录
void mkdir(char* otherinfo){
    char* dirstr[MAX_FILE_NUM];
    char* cd_path[MAX_FILE_NUM];
    p_split(dirstr, otherinfo, '/');
    int id = get_inode_id(dirstr);
    if(id > 0){
        printf("directory already exist!");
        return ;
    }
    int i = 0;
    inode it[MAX_INDOE_NUM], root;
    dir_item blk_buf[BLOCK_SIZE/sizeof(dir_item)];
    it_get(it);
    while(dirstr[i]){
        cd_path[i] = dirstr[i];
        cd_path[i+1] = 0; 
        ++i;
        id = get_inode_id(cd_path);
        if(id>0)continue;

    }
}