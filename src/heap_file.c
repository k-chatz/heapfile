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
#define MAX_BLOCK_RECORDS (BF_BLOCK_SIZE - 2 * sizeof(int)) / sizeof(Record)

/***Private functions***/

void _printRecord(Record record) {
    printf("Record: (%d %s %s %s)\n", record.id, record.name, record.surname, record.city);
}

void _getCharacteristic(char *block, int *characteristic) {
    memcpy(characteristic, block, sizeof(int));
}

void _setCharacteristic(char *block, const int characteristic) {
    memcpy(block, &characteristic, sizeof(int));
}

void _getUsedBlocks(char *block, int *usedBlocks) {
    memcpy(usedBlocks, block + sizeof(int), sizeof(int));
}

void _setUsedBlocks(char *block, const int usedBlocks) {
    memcpy(block + sizeof(int), &usedBlocks, sizeof(int));
}

void _getDataBlockNumber(char *block, int *dataBlockNumber) {
    memcpy(dataBlockNumber, block + sizeof(int) * 2, sizeof(int));
}

void _setDataBlockNumber(char *block, const int dataBlockNumber) {
    memcpy(block + sizeof(int) * 2, &dataBlockNumber, sizeof(int));
}

void _getCount(char *block, int *count) {
    memcpy(count, block + BF_BLOCK_SIZE - 2 * sizeof(int), sizeof(int));
}

void _setCount(char *block, const int count) {
    memcpy(block + BF_BLOCK_SIZE - 2 * sizeof(int), &count, sizeof(int));
}

void _getNext(char *block, int *next) {
    memcpy(next, block + BF_BLOCK_SIZE - sizeof(int), sizeof(int));
}

void _setNext(char *block, const int next) {
    memcpy(block + BF_BLOCK_SIZE - sizeof(int), &next, sizeof(int));
}

void _getRecord(char *block, const int offset, Record *record) {
    memcpy(record, block + offset * sizeof(Record), sizeof(Record));
}

void _setRecord(char *block, const int offset, const Record *record) {
    memcpy(block + offset * sizeof(Record), record, sizeof(Record));
}

int _isFull(char *block) {
    int count = 0;
    _getCount(block, &count);
    return count == MAX_BLOCK_RECORDS;
}

/***Public functions***/

HP_ErrorCode HP_Init() {
    return HP_OK;
}

HP_ErrorCode HP_CreateFile(const char *filename) {
    int fd = 0;
    char *infoBlockData = NULL;
    BF_Block *infoBlock;
    BF_Block_Init(&infoBlock);
    CALL_BF(BF_CreateFile(filename))
    CALL_BF(BF_OpenFile(filename, &fd))
    CALL_BF(BF_AllocateBlock(fd, infoBlock))
    infoBlockData = BF_Block_GetData(infoBlock);
    _setCharacteristic(infoBlockData, CHARACTERISTIC);
    _setUsedBlocks(infoBlockData, 0);
    _setDataBlockNumber(infoBlockData, 0);
    _setNext(infoBlockData, -1);
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
    CALL_BF(BF_CloseFile(fileDesc))
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
