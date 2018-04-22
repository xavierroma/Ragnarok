//
// Created by Clàudia Peiró Vidal on 13/4/18.
//


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "ext.h"

#define DEPTH 0x06
#define EXTENT_TREE 0x28
#define UPPER_BLOCK 0x6
#define LOW_BLOCK 0x8
#define HEADER_SIZE 12
#define LENGTH_DIRECTORY   0x4
#define FILE_NAME 0x8
#define LENGTH_FILE 0x6

#define FILE_NOT_FOUND "Error. File not found.\n"


void showExt(ext4 ext) {
    printf("---- Filesystem Information ----\n\n");

    printf("Filesystem: EXT4\n\n");

    printf("INODE INFO\n");
    printf("Inode Size: %u\n", ext.inode.inodeSize);
    printf("Number of Inodes: %d\n", ext.inode.numberOfInodes);
    printf("First Inode: %d\n", ext.inode.firstInodeM);
    printf("Inodes Group: %d\n", ext.inode.inodesGroup);
    printf("Free Inodes: %d\n\n", ext.inode.freeInodes);

    printf("BLOCK INFO\n");
    printf("Block Size: %d\n", (int)pow(2, (10 + ext.block.blockSize)));
    printf("Reserved Blocks: %d\n", ext.block.reservedBlocks);
    printf("Free Blocks: %d\n", ext.block.freeBlocks);
    printf("Total Blocks: %d\n", ext.block.totalBlocks);
    printf("First BLock: %d\n", ext.block.firstBlock);
    printf("Block group: %d\n", ext.block.blockGroup);
    printf("Frags group: %d\n\n", ext.block.fragsGroup);


    printf("VOLUME INFO\n");
    printf("Volume name: %s\n", ext.volume.volumeName);

    time_t ts = ext.volume.lastCheck;
    printf("Last check: %s", ctime(&ts));

    ts = ext.volume.lastMount;
    printf("Last mount: %s", ctime(&ts));

    ts = ext.volume.lastWritten;
    printf("Last written: %s\n", ctime(&ts));

}

ext4 readExt4(FILE *file) {
    ext4 ext;

    // READ INODE
    fseek(file, INODE_SIZE + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.inode.inodeSize, sizeof(ext.inode.inodeSize), 1, file);

    fseek(file, EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.inode.numberOfInodes, sizeof(ext.inode.numberOfInodes), 1, file);

    fseek(file, FIRST_INODE + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.inode.firstInodeM, sizeof(ext.inode.firstInodeM), 1, file);

    fseek(file, INODE_GROUP + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.inode.inodesGroup, sizeof(ext.inode.inodesGroup), 1, file);

    fseek(file, FREE_INODES + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.inode.freeInodes, sizeof(ext.inode.freeInodes), 1, file);


    //READ BLOCK
    fseek(file, BLOCK_SIZE + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.blockSize, sizeof(ext.block.blockSize), 1, file);

    fseek(file, RESERVED_BLOCKS + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.reservedBlocks, sizeof(ext.block.reservedBlocks), 1, file);

    fseek(file, FREE_BLOCKS + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.freeBlocks, sizeof(ext.block.freeBlocks), 1, file);

    fseek(file, TOTAL_BLOCKS + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.totalBlocks, sizeof(ext.block.totalBlocks), 1, file);

    fseek(file, FIRST_BLOCK + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.firstBlock, sizeof(ext.block.firstBlock), 1, file);

    fseek(file, BLOCK_GROUP + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.blockGroup, sizeof(ext.block.blockGroup), 1, file);

    fseek(file, FRAGS_GROUP + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.fragsGroup, sizeof(ext.block.fragsGroup), 1, file);


    //READ VOLUME
    fseek(file, VOLUME_NAME + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(ext.volume.volumeName, sizeof(ext.volume.volumeName), 1, file);

    fseek(file, LAST_CHECK + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.volume.lastCheck, sizeof(ext.volume.lastCheck), 1, file);

    fseek(file, LAST_MOUNT + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.volume.lastMount, sizeof(ext.volume.lastMount), 1, file);

    fseek(file, LAST_WRITTEN + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.volume.lastWritten, sizeof(ext.volume.lastWritten), 1, file);

    return ext;
}

unsigned long findBlock(FILE *file, __uint64_t in, int blockSize) {
    __uint16_t depth;
    __uint64_t indexInode;
    __uint16_t upperBlock;
    __uint32_t lowBlock;
    unsigned long posBlock;

    //ens posem a la posició del depth per mirar si es fulla
    //indexInode = posInodeTable * blockSize * manyInode + inodeSize + EXTENT_TREE;
    indexInode = in + EXTENT_TREE;

    fseek(file, indexInode + DEPTH, SEEK_SET); //Depth
    fread(&depth, sizeof(depth), 1, file);

    if (depth == 0) {
        //Upper block
        fseek(file, indexInode + HEADER_SIZE + UPPER_BLOCK, SEEK_SET);
        fread(&upperBlock, sizeof(upperBlock), 1, file);

        //Lower block
        fseek(file, indexInode + HEADER_SIZE + LOW_BLOCK, SEEK_SET);
        fread(&lowBlock, sizeof(lowBlock), 1, file);

        //Block
        posBlock = upperBlock << 32 | lowBlock;
        posBlock *= blockSize;
        return posBlock;



    }
    return 0;
}

unsigned long exploreDirectory(__uint64_t posBlock, FILE *file, char *fileToFind, int *found) {
    __uint16_t  var16;
    __uint32_t  var32;
    char * fileName;
    unsigned char length;

    fseek(file, posBlock + LENGTH_FILE, SEEK_SET);
    fread(&length, sizeof(length), 1, file);

    fileName = (char *) malloc(sizeof(char) * ((int)length + 1));

    fseek(file, posBlock + FILE_NAME, SEEK_SET);
    fread(fileName, sizeof(char) * ((int)length + 1), 1, file);

    fileName[(int)length] = '\0';
    printf("File name %s is not %s\n", fileName, fileToFind);

    if (strcmp(fileName, fileToFind) == 0) {
        *found = 1;
        return posBlock;
    }

    fseek(file, posBlock + LENGTH_DIRECTORY, SEEK_SET);
    fread(&var16, sizeof(var16), 1, file);

    posBlock += var16;

    fseek(file, posBlock, SEEK_SET);
    fread(&var32, sizeof(var32), 1, file);
    return posBlock;
}

int findFileinDirectory(__uint64_t posBlock, FILE *file, char *fileToFind) {

    int found = 0;
    __uint32_t var32;

    posBlock = exploreDirectory(posBlock, file, fileToFind, &found);

    fseek(file, posBlock, SEEK_SET);
    fread(&var32, sizeof(var32), 1, file);

    if (var32 != 0 && found == 0) {
        found = findFileinDirectory(posBlock, file, fileToFind);
    } else {
        printf("Inode result %u\n", var32);
        return found;
    }
    return found;

}

void findNextInode(FILE* file, char * fileToFind,  __uint64_t posInodeTable, int blockSize, __uint16_t inodeSize, __uint64_t posBlock);

void deepSearch(FILE* file, char * fileToFind,  __uint64_t posInodeTable, int blockSize, __uint16_t inodeSize, int howMany) {

    unsigned long  posBlock;
    int found = 0;

    __uint64_t in = posInodeTable * blockSize  + inodeSize * howMany;
    posBlock = findBlock(file, in, blockSize);
    found = findFileinDirectory(posBlock, file, fileToFind);

    if (!found) {
        findNextInode(file, fileToFind,  posInodeTable, blockSize, inodeSize, posBlock);
    }

}

void findNextInode(FILE* file, char * fileToFind,  __uint64_t posInodeTable, int blockSize, __uint16_t inodeSize, __uint64_t posBlock) {
    __uint16_t  var16;
    __uint32_t  var32;
    char * fileName;
    unsigned char length;

    fseek(file, posBlock + LENGTH_FILE, SEEK_SET);
    fread(&length, sizeof(length), 1, file);

    fileName = (char *) malloc(sizeof(char) * ((int)length + 1));

    fseek(file, posBlock + FILE_NAME, SEEK_SET);
    fread(fileName, sizeof(char) * ((int)length + 1), 1, file);

    fileName[(int)length] = '\0';


    if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0) {
        fseek(file, posBlock, SEEK_SET);
        fread(&var32, sizeof(var32), 1, file);
        if (var32 != 0) {
            printf("NextInode %u\n", var32);
            //es troba next Inode a explorar
            deepSearch(file, fileToFind,  posInodeTable, blockSize, inodeSize, var32);
        }

    } else {
        fseek(file, posBlock + LENGTH_DIRECTORY, SEEK_SET);
        fread(&var16, sizeof(var16), 1, file);

        posBlock += var16;

        fseek(file, posBlock, SEEK_SET);
        fread(&var32, sizeof(var32), 1, file);

        if (var32 != 0) {
            findNextInode(file, fileToFind,  posInodeTable, blockSize, inodeSize, posBlock);
        }

    }
}



void explore(FILE* file, char * fileToFind,  __uint64_t posInodeTable, int blockSize, __uint16_t inodeSize, int howMany, int operation) {

    unsigned long  posBlock;
    int found = 0;

    __uint64_t in = posInodeTable * blockSize  + inodeSize * howMany;

    posBlock = findBlock(file, in, blockSize);
    found = findFileinDirectory(posBlock, file, fileToFind);

    if (found == 0) {
        if (operation == 1) {
            printf(FILE_NOT_FOUND);
        } else {
            deepSearch(file, fileToFind,  posInodeTable, blockSize, inodeSize, 1);
        }

    }
}


/**
 * Primer haurem de buscar els group descriptors
 * @param file
 */
void searchExt4(FILE * file, char * fileToFind, int operation) {
    int posGroupDescrip;
    __uint32_t posLowInodeTable;
    __uint32_t posHighInodeTable;
    __uint64_t posInodeTable;
    unsigned long posBlock;

    ext4 ext4Info = readExt4(file);
    int blockSize = (int)pow(2, (10 + ext4Info.block.blockSize));

    //posicio group descriptor
    posGroupDescrip = blockSize == EXT_PADDING_SUPER_BLOCK ? 2*EXT_PADDING_SUPER_BLOCK:blockSize;

    //Lower 32 bits de inode table
    fseek(file, 0x8 + posGroupDescrip, SEEK_SET);
    fread(&posLowInodeTable, sizeof(posLowInodeTable), 1, file);

    //high 32 bits de inode table
    fseek(file, 0x24 + posGroupDescrip, SEEK_SET);
    fread(&posHighInodeTable, sizeof(posHighInodeTable), 1, file);

    posInodeTable = posHighInodeTable << 32 | posLowInodeTable;

    explore(file, fileToFind, posInodeTable, blockSize, ext4Info.inode.inodeSize, 1, operation);


}



