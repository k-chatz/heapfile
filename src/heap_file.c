#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "heap_file.h"

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

HP_ErrorCode HP_Init() {
    return HP_OK;
}

HP_ErrorCode HP_CreateFile(const char *filename) {
    int usedBlocks = 0, dataBlockNumber = 0, fd = 0, characteristic = CHARACTERISTIC;
    char *infoBlockData = NULL;
    BF_Block *infoBlock;
    BF_Block_Init(&infoBlock);
    CALL_BF(BF_CreateFile(filename))
    CALL_BF(BF_OpenFile(filename, &fd))
    CALL_BF(BF_AllocateBlock(fd, infoBlock))
    infoBlockData = BF_Block_GetData(infoBlock);

    memcpy(infoBlockData, &characteristic, sizeof(characteristic));
    memcpy(infoBlockData + sizeof(characteristic), &usedBlocks, sizeof(usedBlocks));
    memcpy(infoBlockData + sizeof(characteristic) + sizeof(usedBlocks), &dataBlockNumber, sizeof(dataBlockNumber));

    BF_Block_SetDirty(infoBlock);
    CALL_BF(BF_UnpinBlock(infoBlock))
    CALL_BF(BF_CloseFile(fd))
    BF_Block_Destroy(&infoBlock);
    return HP_OK;
}

HP_ErrorCode HP_OpenFile(const char *fileName, int *fileDesc) {
    //insert code here
    return HP_OK;
}

HP_ErrorCode HP_CloseFile(int fileDesc) {
    //insert code here
    return HP_OK;
}

HP_ErrorCode HP_InsertEntry(int fileDesc, Record record) {
    //insert code here
    return HP_OK;
}

HP_ErrorCode HP_PrintAllEntries(int fileDesc, char *attrName, void *value) {
    //insert code here
    return HP_OK;
}

HP_ErrorCode HP_GetEntry(int fileDesc, int rowId, Record *record) {
    //insert code here
    return HP_OK;
}
