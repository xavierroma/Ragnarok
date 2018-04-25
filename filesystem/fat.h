//
// Created by Clàudia Peiró Vidal on 17/4/18.
//

#ifndef RAGNAROK_FAT_H
#define RAGNAROK_FAT_H


#include <stdio.h>

#define SYSTEM_NAME                  0x03
#define SECTOR_SIZE                  0x0B
#define SECTOR_CLUSTER               0x0D
#define RESERVED_SECTORS             0x0E
#define NUMBER_OF_FATS               0x10
#define ROOT_ENTRIES                 0x11
#define NUMBER_FATS_SECTOR           0x24
#define ROOT_FIRST_CLUSTER           0x2C
#define LABEL                        0x47

#define NAME                         0x0D
#define ATTRIBUTES                   0x0B
#define DATE                         0x10
#define NEXT_CLUSTER_LOW             0x14
#define NEXT_CLUSTER_HIGH            0x1A
#define SIZE                         0x1C

typedef struct fat32Information {
    char systemName[11];
    __uint16_t sectorSize;
    unsigned char sectorPerCluster;
    __uint16_t reservedSectors;
    __uint32_t numberOfFats;
    __uint16_t maximumRootEntries;
    __uint32_t rootFirstCluster;
    __uint16_t numberOfSectorsPerFat;
    char label[9];
} fat32;

typedef struct ClusterFat32Information {
    char name[11];
    __uint8_t attributs;
    __uint16_t date;
    __uint32_t nextCluster;
    __uint32_t size;
} clusterData;

fat32 readFat32(FILE *file);

void searchFat32(FILE * file, char * fileToFind, int operation);

#endif //RAGNAROK_FAT_H
