//
// Created by Clàudia Peiró Vidal on 13/4/18.

// 1 Files in this filesystem use extents (INCOMPAT_EXTENTS). --> ext4
// 1 Has a journal (COMPAT_HAS_JOURNAL). --> ext3
// 0 Has a journal (COMPAT_HAS_JOURNAL). --> ext2
//


#include "filesystem.h"

#define EXT_MAGIC_NUMBER_OFFSET         0x38
#define EXT_MAGIC_SEQUENCE              0xef53
#define EXT_HAS_JOURNAL                 0x5C
#define EXT_USE_EXTENTS                 0x60
#define MASK_JOURNAL                    0x4
#define MASK_EXTENTS                    0x40


void chooseExt (FILE* file) {
    unsigned long extents;
    long aux;

    fseek(file, EXT_USE_EXTENTS + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&extents, sizeof(extents), 1, file);
    aux = MASK_EXTENTS & extents;

    if (aux > 0) {
        readExt(file);
    } else {

        fseek(file, EXT_HAS_JOURNAL + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
        fread(&extents, sizeof(extents), 1, file);
        aux = MASK_JOURNAL & extents;

        if (aux > 0) {
            printf("File System not recognized (EXT3)");
        } else {
            printf("File System not recognized (EXT2)");
        }
    }
}

void chooseFilesystem (char * pathFile) {
    FILE* file;

    unsigned short magicSignature;

    file = fopen(pathFile, "rb");

    if (file == NULL) {
        printf(MSG_ERR_FITXER);
    } else {
        //Possible ext?
        fseek(file, EXT_MAGIC_NUMBER_OFFSET + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
        fread(&magicSignature, sizeof(magicSignature), 1, file);

        if (magicSignature == EXT_MAGIC_SEQUENCE) {             //it will be a ext
            chooseExt(file);
        }

        fclose(file);
    }

}

