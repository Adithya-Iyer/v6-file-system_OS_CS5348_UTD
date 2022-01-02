# v6-file-system_OS_CS5348_UTD

[CS5348] Project 2 Part2 - Team 12

Commands implemented as per part 2:
    1. cpin extFile intFile - This function takes in source as extFile from the working directory 
                              and stores it in an internal file represented in v6-file system
    2. cpout intFile extFile - This fucntion takes in source as intFile represented in v6-file system
                               and copies it's contents into an externalFile in the working directory of user
    3. rm v6-File -  This function is used to remove the file represented in the internal v6 file system
    4. mkdir v6Dir - This will let users create a new directory in the v6 file system
    5. cd v6Dir - This functionality lets user to change the working directory to the path specified by v6Dir
    6. q -  saves all changes and quits the program

How to run:

    1. Copy file named mod-v6-part2.c into your working directory in any of UTD linux server
    2. Execute "cc -o mod mod-v6-part2.c" to generate binary object file mod
    3. Execute ./mod
    4. Type in the commands, example set of commands below
        4.1. openfs abc
        4.2. initfs 550 10
        4.3. mkdir /user
        4.4. mkdir /user/abc
        4.5. cd /user/abc
        4.6. cpin extFile.txt intFile.txt
        4.7. cd /
        4.8. cpout /user/abc/intFile.txt extFileNew.txt
        4.9. rm /user/abc/intFile.txt
    5. Logs can be seen on the screen with statuses


[PROJECT DESCRIPTION - PART 1]

University of Texas at Dallas--Computer Science Department
CS 5348 Operating Systems Concepts Fall 2021
Project 2-Part 1

V6 file system is highly restrictive. A modification has been done: Block size is 1024 Bytes, i-node size is 64 Bytes and i-node’s structure and directory entry struc have been modified as well and given below:

typedef struct {
 int isize;
 int fsize;
int nfree;
 unsigned int free[251];
 char flock;
 char ilock;
 char fmod;
 unsigned int time;
} superblock_type; // Block size is 1024 Bytes; only 1023 Bytes are used
superblock_type superBlock;

// i-node Structure
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
} inode_type; //64 Bytes in size

typedef struct {
 unsigned int inode;
 char filename[28];
} dir_type;//32 Bytes long

Flags field has a small change: bits 1, b, c are as before. Bits d and e are to represent if the file is small/medium/long/super long file (00 = small file, 01=medium, 10=long and 11 = super long file). Bit f is for set uid on execution and bit g is for set gid on execution. Other bits remain the same.
If file is small addr[9] has 9 direct block addresses. If file is medium, addr[9] has addresses of 9 single indirect blocks. If file is large, each element of addr[] is address of a double indirect block. If file is super long, each element of addr[] is address of a triple indirect block.

You need to develop a program called mod-v6.c (or mod-v6.cc) that implements the following three commands in C/C++:
1. openfs file_name
      In this case, file_name is the name of the file in the native unix machine (where you are running your program) that represents the disk drive.
2. initfs n1 n2 
      where n1 is the file system size in number of blocks and n2 is the number of blocks devoted to the i-nodes. In this case, set all data blocks free (except for one data block for storing the contents of i-node number 1, representing the root, which has the two entries . and .. All i-nodes except i-node number 1 are (unallocated) set to free. Make sure that all free blocks are accessible from free[] array of the super block. One of the data blocks contains the root directory’s contents (two entries . and ..)
3. q
    Quit the program

Some useful Unix system calls: lseek(), read(), write(), open()
This project must be done in C/C++ only.


[PROJECT DESCRIPTION - PART 2]

University of Texas at Dallas--Computer Science Program
CS 5348 Operating Systems Concepts Fall 2021
Project 2-Part 2

You need to implement the following commands and the commands are additional commands to your program developed in Project 2-Part 1: Keep in mind that all files are small files. No large files at all for this part of the project. Any reference to v6 in this description refers to the modified v6 file system that you are familiar with in Project-2-Part-1.

(a) cpin externalfile v6-file
      Creat a new file called v6-file in the v6 file system and fill the contents of the newly created file with the contents of the externalfile. The file externalfile is a file in the native unix machine, the system where your program is being run.
(b) cpout v6-file externalfile
      If the v6-file exists, create externalfile and make the externalfile's contents equal to v6-file.
(c) rm v6-file
      Delete the file v6-file from the v6 file system. Remove all the data blocks of the file, free the i-node and remove the directory entry.
(d) mkdir v6dir
      create the v6dir. It should have two entries . and ..
(e) cd v6dir
      change working directory of the v6 file system to the v6dir
(f) q
      Save all changes and quit. 

Keep in mind that all file names starting with / are absolute path names and those not starting with / are relative to current working directory. 

For example the following sequence
mkdir /user
mkdir /user/Adi
cd /user/Adi
cpin myfile-in-native-unix f1-in-v6
is possible where f1-in-v6 is created in the directory /user/Adi of the v6 file system
