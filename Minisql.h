//
//  Minisql.h
//  Minisql
//
//  Created by 陈泓宇 on 20/6/2017.
//  Copyright (c) 2014年 陈泓宇. All rights reserved.
//

#ifndef Minisql_Minisql_h
#define Minisql_Minisql_h
#include <stdio.h>
#include <time.h>
#include <iostream>
using namespace std;

struct blockNode
{
    int offsetNum; // the offset number in the block list
    bool pin;  // the flag that this block is locked是否锁定
    bool ifbottom; // flag that this is the end of the file node是否文件节点底部
    char* fileName; // the file which the block node belongs to
    friend class BufferManager;

private:
    char *address; // the content address
    blockNode * preBlock;
    blockNode * nextBlock;
    bool reference; // the LRU replacement flag ，LRU——最近最少使用置换算法
    bool dirty; // the flag that this block is dirty, which needs to written back to the disk later脏块
    size_t using_size; // the byte size that the block have used. The total size of the block is BLOCK_SIZE . This value is stored in the block head.

};

struct fileNode
{
    char *fileName;
    bool pin; // the flag that this file is locked
    blockNode *blockHead;//文件的第一个块节点
    fileNode * nextFile;
    fileNode * preFile;
};

extern clock_t start;
extern void print();//main.cpp,输出时间间隔

#define MAX_FILE_NUM 40
#define MAX_BLOCK_NUM 300
#define MAX_FILE_NAME 100

#endif
