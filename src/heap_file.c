#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "heap_file.h"

#define CALL_BF(call){ BF_ErrorCode code = call; if (code != BF_OK) {BF_PrintError(code); return HP_ERROR; }}

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

/*
 * Check if block is full.
 * Returns 0/1
 * Assign the current count of records in count variable.*/
int _isFull(char *block, int *count) {
    _getCount(block, count);
    return *count == MAX_BLOCK_RECORDS;
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
    char *infoBlockData = NULL;
    int characteristic = 0, next = 0;
    BF_Block *infoBlock;
    BF_Block_Init(&infoBlock);
    CALL_BF(BF_OpenFile(fileName, fileDesc))
    CALL_BF(BF_GetBlock(*fileDesc, 0, infoBlock));
    infoBlockData = BF_Block_GetData(infoBlock);
    _getCharacteristic(infoBlockData, &characteristic);
    _getNext(infoBlockData, &next);
    if (characteristic != CHARACTERISTIC) {
        return HP_ERROR;
    }
    //CALL_BF(BF_UnpinBlock(infoBlock))
    //BF_Block_Destroy(&infoBlock);
    return HP_OK;
}

HP_ErrorCode HP_CloseFile(int fileDesc) {
    CALL_BF(BF_CloseFile(fileDesc))
    return HP_OK;
}

HP_ErrorCode HP_InsertEntry(int fileDesc, Record record) {
    int usedBlocks = 0, blocks_num = 0, characteristic = 0, count = 0, next = 0;
    char *infoBlockData = NULL, *blockData = NULL, *newBlockData = NULL;
    BF_Block *infoBlock, *block, *newBlock;

    BF_Block_Init(&block);
    BF_Block_Init(&infoBlock);

    CALL_BF(BF_GetBlock(fileDesc, 0, infoBlock));
    infoBlockData = BF_Block_GetData(infoBlock);
    _getCharacteristic(infoBlockData, &characteristic);
    if (characteristic != CHARACTERISTIC) {
        return HP_ERROR;
    }
    _getUsedBlocks(infoBlockData, &usedBlocks);
    _getDataBlockNumber(infoBlockData, &next);

    // If data block not yet exists, then a new one is about to be created.
    if (!next) {
        CALL_BF(BF_AllocateBlock(fileDesc, block));
        blockData = BF_Block_GetData(block);
        _setRecord(blockData, 0, &record);
        _setCount(blockData, 1);
        _setNext(blockData, -1);
        BF_Block_SetDirty(block);
        CALL_BF(BF_UnpinBlock(block));
        /*Update info block data*/
        CALL_BF(BF_GetBlockCounter(fileDesc, &blocks_num));
        _setDataBlockNumber(infoBlockData, blocks_num - 1);
        _setUsedBlocks(infoBlockData, usedBlocks + 1);
        BF_Block_SetDirty(infoBlock);
    } else {
        do {
            CALL_BF(BF_GetBlock(fileDesc, next, block))
            blockData = BF_Block_GetData(block);
            _getNext(blockData, &next);

            if (!_isFull(blockData, &count)) {
                _setRecord(blockData, count, &record);
                _setCount(blockData, count + 1);
                BF_Block_SetDirty(block);
                CALL_BF(BF_UnpinBlock(block));
                break;
            } else {
                if (next > 0) {
                    CALL_BF(BF_UnpinBlock(block));
                    continue;
                }

                BF_Block_Init(&newBlock);
                CALL_BF(BF_AllocateBlock(fileDesc, newBlock));
                newBlockData = BF_Block_GetData(newBlock);
                _setRecord(newBlockData, 0, &record);
                _setCount(newBlockData, 1);
                _setNext(newBlockData, -1);
                BF_Block_SetDirty(newBlock);
                CALL_BF(BF_UnpinBlock(newBlock));
                BF_Block_Destroy(&newBlock);

                CALL_BF(BF_GetBlockCounter(fileDesc, &blocks_num));
                _setNext(blockData, blocks_num - 1);
                BF_Block_SetDirty(block);
                CALL_BF(BF_UnpinBlock(block));

                /*Update info block data*/
                _setUsedBlocks(infoBlockData, usedBlocks + 1);
                BF_Block_SetDirty(infoBlock);
            }
        } while (next > 0);
    }

    CALL_BF(BF_UnpinBlock(infoBlock));
    BF_Block_Destroy(&infoBlock);
    BF_Block_Destroy(&block);
    return HP_OK;
}

HP_ErrorCode HP_PrintAllEntries(int fileDesc, char *attrName, void *value) {
    int count = 0, next = 0, slot = 0;
    char *infoBlockData = NULL, *blockData = NULL;
    Record record;
    BF_Block *infoBlock, *block;
    BF_Block_Init(&block);
    BF_Block_Init(&infoBlock);
    CALL_BF(BF_GetBlock(fileDesc, 0, infoBlock))
    infoBlockData = BF_Block_GetData(infoBlock);
    _getDataBlockNumber(infoBlockData, &next);
    if (next) {
        do {
            CALL_BF(BF_GetBlock(fileDesc, next, block))
            blockData = BF_Block_GetData(block);
            _getNext(blockData, &next);
            _getCount(blockData, &count);
            for (slot = 0; slot < count; slot++) {
                _getRecord(blockData, slot, &record);
                if (strcmp(attrName, "id") == 0) {
                    if (record.id == *(int *) value) {
                        _printRecord(record);
                    }
                } else if (strcmp(attrName, "name") == 0) {
                    if (strcmp(record.name, (char *) value) == 0) {
                        _printRecord(record);
                    }
                } else if (strcmp(attrName, "surname") == 0) {
                    if (strcmp(record.surname, (char *) value) == 0) {
                        _printRecord(record);
                    }
                } else if (strcmp(attrName, "city") == 0) {
                    if (strcmp(record.city, (char *) value) == 0) {
                        _printRecord(record);
                    }
                }
            }
            CALL_BF(BF_UnpinBlock(block));
        } while (next > 0);
    }
    CALL_BF(BF_UnpinBlock(infoBlock));
    BF_Block_Destroy(&infoBlock);
    BF_Block_Destroy(&block);
    return HP_OK;
}

HP_ErrorCode HP_GetEntry(int fileDesc, int rowId, Record *record) {
    int count = 0, next = 0, slot = 0, c = 1;
    char *infoBlockData = NULL, *blockData = NULL;
    Record r;
    BF_Block *infoBlock, *block;
    BF_Block_Init(&block);
    BF_Block_Init(&infoBlock);
    CALL_BF(BF_GetBlock(fileDesc, 0, infoBlock))
    infoBlockData = BF_Block_GetData(infoBlock);
    _getDataBlockNumber(infoBlockData, &next);
    if (next) {
        do {
            CALL_BF(BF_GetBlock(fileDesc, next, block))
            blockData = BF_Block_GetData(block);
            _getNext(blockData, &next);
            _getCount(blockData, &count);
            for (slot = 0; slot < count; slot++) {
                _getRecord(blockData, slot, &r);
                if (c == rowId) {
                    *record = r;
                    CALL_BF(BF_UnpinBlock(block));
                    CALL_BF(BF_UnpinBlock(infoBlock));
                    BF_Block_Destroy(&infoBlock);
                    BF_Block_Destroy(&block);
                    return HP_OK;
                }
                c++;
            }
            CALL_BF(BF_UnpinBlock(block));
        } while (next > 0);
    }
    CALL_BF(BF_UnpinBlock(infoBlock));
    BF_Block_Destroy(&infoBlock);
    BF_Block_Destroy(&block);
    return HP_ERROR;
}
