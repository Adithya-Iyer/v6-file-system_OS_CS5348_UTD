/*
* CS 5348 Project 2 Part 1

* Team Members:
   Raj Mishra (RXM190093)
   Adithya Iyer (ASI200000)
   Yu Chen (YXC190072)
*  Contribution by each team member in terms of methods and functionalities:
       Raj Mishra - chaining(), getFreeBlock(), , main() (collaboration with Adithya), addFreeBlock(), Debugging and Testing of the code (collaboration with Yu Chen)
       Adithya Iyer - openfs(), Final Commenting and Refactoring of the code, main() (collaboration with Raj), quit()
       Yu Chen - initfs(), Debugging and Testing (collaboration with Raj), writeBlockToFS(), writeInodeToFS()
*   How to run and validate:
        # Copy the file to one of the directories in any CS UTD linux machines
        # Execute cc -o mod mod-v6.c
        # Execute ./mod
        # Enter the commands in order for example
            $ openfs abc
            $ initfs 555 15
            $ q
        # Please do a openfs before initfs
        # You would be able to see logs on the screen to validate
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

//superblock struct
typedef struct {
    unsigned int isize;
    unsigned int fsize;
    unsigned int nfree;
    unsigned int free[251];
    char flock;
    char ilock;
    char fmod;
    unsigned int time;
} superblock_type;

//Inode Struct
typedef struct {
    unsigned short flags;
    unsigned short nlinks;
    unsigned int uid;
    unsigned int gid;
    unsigned int size0;
    unsigned int size1;
    unsigned int addr[9];
    unsigned int actime;
    unsigned int modtime;
} inode_type;

typedef struct {
    unsigned int inode;
    char filename[28];
} dir_type;

int chain[256];
int zeros[256];

int fd;
superblock_type superBlock;
inode_type root_inode;
int total_num_inodes;

//Defining size of inode and block as const for better readibility
const int INODESIZE = 64;
const int BLOCKSIZE = 1024;

//This method will write a block to FileSystem
void writeBlockToFS(int bNumber,void *input, int num_bytes){
    lseek(fd,BLOCKSIZE * bNumber,SEEK_SET);
    write(fd,input,num_bytes);
}

//This methof will write Inode to file system
void writeInodeToFS(int iNumber,void * input, int num_bytes){
    lseek(fd,((iNumber-1)*INODESIZE)+(2*BLOCKSIZE),SEEK_SET);
    write(fd,input,num_bytes);
}

void openfs(char* fileName){
    
    //Opening a file with read write persmission and creating in case it is absent
    fd = open(fileName, O_CREAT | O_RDWR, 0644);
    lseek(fd,BLOCKSIZE,SEEK_SET);

    printf("File %s opened with permission O_CREAT, O_RDWR\n",fileName);

    //Checking if the file is already present with super block and inode written
    if(access(fileName,F_OK) == 0){
        struct stat st;
        stat(fileName, &st);
        if(st.st_size >= (2*BLOCKSIZE+INODESIZE)){
            printf("File %s already exists, reading super block and root inode\n",fileName);
            read(fd,&superBlock,BLOCKSIZE);
            read(fd,&root_inode,sizeof(root_inode));
        }
    }
}

//Adding a free block by writing to the filesystem
void addFreeBlock(int bNumber){
    
    lseek(fd,BLOCKSIZE * (bNumber),SEEK_SET);
    write(fd,&zeros,sizeof(zeros));
    
    superBlock.free[superBlock.nfree] = bNumber;
    superBlock.nfree++;
}

int getFreeBlock(){
    //reducing nfree value by 1
    superBlock.nfree--;

    //if no free data block remains
    if(superBlock.free[superBlock.nfree] == 0)
        return -1;

    //if nfree becomes 0 we copy the values from next chain, as per the algorithms taught in class
    if(superBlock.nfree == 0){
        int bNumber = superBlock.free[0];
        lseek(fd,BLOCKSIZE * bNumber,SEEK_SET);
        read(fd,&chain,sizeof(chain));
        int i;
        superBlock.nfree = chain[0]-1;
        for(i=1;i<=250;i++)
            superBlock.free[i-1] = chain[i];
        superBlock.free[superBlock.nfree] = bNumber;
        return bNumber;
    }else{
        return superBlock.free[superBlock.nfree];
    }
}

void chaining(int blocks){

    printf("Initializing chaining of free data blocks in sets of 251\n");

    int total_chains = blocks/251;
    int remaining_blocks = blocks%251;

    int i;

    /*
    if total chains is equal to 1 then we already assigned to the nfree and no chaining is needed
    */

    if(total_chains>1){
        for(i=0;i<total_chains-1;i++){
            int j;
            chain[0] = 251;

            for(j=1;j<=250;j++){
                if(i == total_chains-1 && remaining_blocks == 0 && j == 1){
                    chain[j] = 0;
                }else{
                    chain[j] = (2 + (superBlock.isize)) + ((i+1)*251) + j;
                }
            }

            lseek(fd,((2+superBlock.isize) + (251*i))*BLOCKSIZE,SEEK_SET);
            int kk = 0;

            write(fd,&chain,sizeof(chain));

            for(j=0;j<251;j++){
                lseek(fd,((2+superBlock.isize)+(251*(i+1))+j)*BLOCKSIZE,SEEK_SET);
                write(fd,&zeros,sizeof(zeros));
            }
        }
    }

    printf("Chaining total number of data blocks modulo 251 blocks\n");

    int j;

    //if case handles when we already assigned the blocks to free array
    if(total_chains == 0){
        for(j=0;j<remaining_blocks+2;j++){
            if(j == 0)
                chain[j] = remaining_blocks;
            else
                chain[j] = 0;
        }
        lseek(fd,(2+superBlock.isize)*BLOCKSIZE,SEEK_SET);
    }else{  //else case handles when there are remaining blocks apart from the chained blocks
        for(j=0;j<remaining_blocks+2;j++){
            if(j == 0)
                chain[j] = remaining_blocks;
            else if(j == 1)
                chain[j] = 0;
            else{
                chain[j] = (2 + (superBlock.isize)) + ((total_chains)*251) + (j-2);
            }
        }
        lseek(fd,((2+superBlock.isize) + (251*(total_chains-1)))*BLOCKSIZE,SEEK_SET);
    }
    
    write(fd,&chain,sizeof(chain));

    int k;

    //writing zeroes to the blocks
    for(k=1;k<remaining_blocks+2;k++){
        if(chain[k] == 0)
            continue;
        lseek(fd,(chain[k])*BLOCKSIZE,SEEK_SET);
        write(fd,&zeros,sizeof(zeros));
    }

    printf("Chaining Ends\n");

}

void quit(){
    printf("Received quit command\nClosing File\n");
    close(fd);
    printf("Quitting\n");
    exit(0);
}

void initfs(int totalBlocks,int totalInodeBlocks){
    printf("Initializing the file system\n");
    int totalIsize = 0;
    int total_num_inodes = totalInodeBlocks*16;

    //initializing superblock with appropriate values
    superBlock.isize = totalInodeBlocks;
    superBlock.fsize = totalBlocks;
    superBlock.flock = 'x';
    superBlock.ilock = 'x';
    superBlock.fmod = 'x';
    superBlock.time = (int)time(NULL); //unix epoch time
    superBlock.nfree = 0;

    printf("Writing Super Block to the file system\n");

    writeBlockToFS(1,&superBlock,BLOCKSIZE);

    int currBlockNumber = totalInodeBlocks + 2;
    int i;
    
    printf("Allocating first 251 data blocks free\n");

    for(i = 1;i<=251 && (i<=(totalBlocks-totalInodeBlocks-2));i++){
        addFreeBlock(currBlockNumber);
        currBlockNumber++;
    }

    int idx;
    for(idx = 0;idx<256;idx++)
        zeros[idx] = 0;

    chaining(totalBlocks-2-superBlock.isize);

    int ii;
    int dir_block = getFreeBlock();
    
    if(dir_block == -1){
        printf("No free data block exists\nRelease some blocks and execute again\n");
    }
    
    printf("Initializing Root Inode with first free data block\n");

    //Initializing root Inode with values
    //1(allocated)10(directory)00(small file)1(uid)1(gid)111(rwx for owner)101(rx for group)100(read for everyone)
    root_inode.flags = root_inode.flags | 51180; //using bitwise OR operation
    root_inode.size0 = 0;
    root_inode.size1 = 2*32;
    root_inode.nlinks = 0;
    root_inode.uid = 0;
    root_inode.gid = 0;
    root_inode.addr[0] = dir_block;
    root_inode.addr[1] = 0;
    root_inode.addr[2] = 0;
    root_inode.addr[3] = 0;
    root_inode.addr[4] = 0;
    root_inode.addr[5] = 0;
    root_inode.addr[6] = 0;
    root_inode.addr[7] = 0;
    root_inode.addr[8] = 0;
    root_inode.actime = 0; //access time is not defined since it's never accessed
    root_inode.modtime = (int)time(NULL); //unix epoch time

    printf("Writing Root Inode to the file system\n");

    writeInodeToFS(1,&root_inode,INODESIZE);

    printf("Initializing Root directory\n");

    dir_type directory[2];

    directory[0].inode = 1;
    directory[1].inode = 1;

    directory[0].filename[0] = '.';

    directory[1].filename[0] = '.';
    directory[1].filename[1] = '.';

    printf("Writing Root directory to the file system\n");

    writeBlockToFS(dir_block,&directory,BLOCKSIZE);

    int currInodeNumber;

    printf("Allocating free inode to File System\n");

    for(currInodeNumber = 2;currInodeNumber<=total_num_inodes;currInodeNumber++){
        inode_type temp_inode;
        
        temp_inode.flags = 0;
        temp_inode.size0 = 0;
        temp_inode.size1 = 0;
        temp_inode.nlinks = 0;
        temp_inode.uid = 0;
        temp_inode.gid = 0;
        temp_inode.addr[0] = 0;
        temp_inode.addr[1] = 0;
        temp_inode.addr[2] = 0;
        temp_inode.addr[3] = 0;
        temp_inode.addr[4] = 0;
        temp_inode.addr[5] = 0;
        temp_inode.addr[6] = 0;
        temp_inode.addr[7] = 0;
        temp_inode.addr[8] = 0;
        root_inode.actime = 0;
        root_inode.modtime = 0;

        writeInodeToFS(currInodeNumber,&temp_inode,INODESIZE);
    }

}

void main(){

    while(1){
        printf("###################################\n");
        printf("Input command alongwith arguments\n");
        
        char cmd[256];
        scanf(" %[^\n]s",cmd); //for reading strings with whitespaces
        
        char *token;
        char *first;
        char *second;
        token = strtok(cmd," "); //using strtok to split string based upon delimeter
        
        if(strcmp(token,"openfs") == 0){
            first = strtok(NULL," ");
            openfs(first);
        }else if(strcmp(token,"initfs") == 0){
            first = strtok(NULL," ");
            second = strtok(NULL," ");

            initfs(atoi(first),atoi(second));
        }else if(strcmp(token,"q") == 0){
            quit();
        }else{
            printf("Invalid command\n");
        }
    }

}