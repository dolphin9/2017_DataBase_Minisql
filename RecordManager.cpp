//
//  RecordManager.cpp
//  minisql
//
//  Created by 邓永辉 on 14/11/5.
//  Copyright (c) 2014年 邓永辉. All rights reserved.
//

#include <iostream>
#include "RecordManager.h"
#include <cstring>
#include "API.h"

/**
 *
 * create a table
 * @param tableName: name of table
 */
int RecordManager::tableCreate(string tableName)//放table的空文件创建，1表示成功，0表示失败
{
    string tableFileName = tableFileNameGet(tableName);//设置创建后放table的table's file name叫做“TABLE_FILE_**" ，**：tableName

    FILE *fp;
    fp = fopen(tableFileName.c_str(), "w+");//创建一个放表的文件
	//tableFileName，string类型，tableFileName.c_str()指向tableFileName字符串内容，
    if (fp == NULL)
    {
        return 0;
    }
    fclose(fp);
    return 1;
}

/**
 *
 * drop a table
 * @param tableName: name of table
 */
int RecordManager::tableDrop(string tableName)//删除表：删除buffermanager的file node and its block node?,删除磁盘文件，成功返回1，失败返回0
{
    string tableFileName = tableFileNameGet(tableName);
    bm.delete_fileNode(tableFileName.c_str());//BufferManager bm，delete the file node and its block node
    if (remove(tableFileName.c_str()))//remove函数删除指定文件,int  remove(const char *_Filename);参数Filename为指定的要删除的文件名，成功则返回0，失败则返回-1
    {
        return 0;
    }
    return 1;
}

/**
 *
 * create a index
 * @param indexName: name of index
 */
int RecordManager::indexCreate(string indexName)////放index的空文件创建，1表示成功，0表示失败
{
    string indexFileName = indexFileNameGet(indexName);//INDEX_FILE_"+indexName

    FILE *fp;
    fp = fopen(indexFileName.c_str(), "w+");
    if (fp == NULL)
    {
        return 0;
    }
    fclose(fp);
    return 1;
}

/**
 *
 * drop a index
 * @param indexName: name of index
 */
int RecordManager::indexDrop(string indexName)
{
    string indexFileName = indexFileNameGet(indexName);
    bm.delete_fileNode(indexFileName.c_str());
    if (remove(indexFileName.c_str()))
    {
        return 0;
    }
    return 1;
}

/**
 *
 * insert a record to table
 * @param tableName: name of table
 * @param record: value of record
 * @param recordSize: size of the record
 * @return the position of block in the file(-1 represent error)
 */
int RecordManager::recordInsert(string tableName,char* record, int recordSize)//找tableName对应文件有足够空间的block的对应地址，拷贝要插入的内容，成功return the position of block in the file，-1 represent error
{
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());//获得名叫tableName的文件节点
    blockNode *btmp = bm.getBlockHead(ftmp);
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (bm.get_usingSize(*btmp) <= bm.getBlockSize() - recordSize)
        {

            char* addressBegin;
            addressBegin = bm.get_content(*btmp) + bm.get_usingSize(*btmp);//块节点开始指针+已使用的
            memcpy(addressBegin, record, recordSize);//
            bm.set_usingSize(*btmp, bm.get_usingSize(*btmp) + recordSize);//更新该块节点已使用的空间
            bm.set_dirty(*btmp);//标记为脏节点，要重新写入文件
            return btmp->offsetNum;
        }
        else
        {
            btmp = bm.getNextBlock(ftmp, btmp);
        }
    }

    return -1;
}

/**
 *
 * print all record of a table meet requirement
 * @param tableName: name of table
 * @param attributeNameVector: the attribute list
 * @param conditionVector: the conditions list
 * @return int: the number of the record meet requirements(-1 represent error)
 */
int RecordManager::recordAllShow(string tableName, vector<string>* attributeNameVector,  vector<Condition>* conditionVector)
{
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);
    int count = 0;//count为所有块的recordblocknum之和
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (btmp->ifbottom)//如果文件最后一块
        {
            int recordBlockNum = recordBlockShow(tableName,attributeNameVector, conditionVector, btmp);//btmp-块节点
            count += recordBlockNum;
            return count;
        }
        else//否则块节点要连接到下一个
        {
            int recordBlockNum = recordBlockShow(tableName, attributeNameVector, conditionVector, btmp);
            count += recordBlockNum;
            btmp = bm.getNextBlock(ftmp, btmp);
        }
    }

    return -1;
}

/**
 *
 * print record of a table in a block
 * @param tableName: name of table
 * @param attributeNameVector: the attribute list
 * @param conditionVector: the conditions list
 * @param blockOffset: the block's offsetNum
 * @return int: the number of the record meet requirements in the block(-1 represent error)
 */
int RecordManager::recordBlockShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector, int blockOffset)//显示特定偏移的块节点的所有符合记录
{
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode* block = bm.getBlockByOffset(ftmp, blockOffset);//获得偏移为blockOffset的块节点
    if (block == NULL)
    {
        return -1;
    }
    else
    {
        return  recordBlockShow(tableName, attributeNameVector, conditionVector, block);
    }
}

/**
 *
 * print record of a table in a block
 * @param tableName: name of table
 * @param attributeNameVector: the attribute list
 * @param conditionVector: the conditions list
 * @param block: search record in the block
 * @return int: the number of the record meet requirements in the block(-1 represent error)
 */
int RecordManager::recordBlockShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector, blockNode* block)//显示文件单块的符合条件的记录，返回记录数目，-1 represent error
{

    //if block is null, return -1
    if (block == NULL)
    {
        return -1;
    }

    int count = 0;

    char* recordBegin = bm.get_content(*block);//开始等于blockBegin
    vector<Attribute> attributeVector;
    int recordSize = api->recordSizeGet(tableName);//获得一条记录的大小

    api->attributeGet(tableName, &attributeVector);//属性向量里赋值标的属性
    char* blockBegin = bm.get_content(*block);//
    size_t usingSize = bm.get_usingSize(*block);

    while (recordBegin - blockBegin  < usingSize)//每次recordBegin会移到下一条记录
    {
        //if the recordBegin point to a record

        if(recordConditionFit(recordBegin, recordSize, &attributeVector, conditionVector))//是否满足条件
        {
            count ++;
            recordPrint(recordBegin, recordSize, &attributeVector, attributeNameVector);//attributeVector含一个表的所有属性，attributeNameVector，用户输进去的属性列
            printf("\n");
        }

        recordBegin += recordSize;
    }

    return count;
}

/**
 *
 * find the number of all record of a table meet requirement
 * @param tableName: name of table
 * @param conditionVector: the conditions list
 * @return int: the number of the record meet requirements(-1 represent error)
 */
int RecordManager::recordAllFind(string tableName, vector<Condition>* conditionVector)
{
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);
    int count = 0;
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (btmp->ifbottom)
        {
            int recordBlockNum = recordBlockFind(tableName, conditionVector, btmp);
            count += recordBlockNum;
            return count;
        }
        else
        {
            int recordBlockNum = recordBlockFind(tableName, conditionVector, btmp);
            count += recordBlockNum;
            btmp = bm.getNextBlock(ftmp, btmp);
        }
    }

    return -1;
}

/**
 *
 * find the number of record of a table in a block
 * @param tableName: name of table
 * @param block: search record in the block
 * @param conditionVector: the conditions list
 * @return int: the number of the record meet requirements in the block(-1 represent error)
 */
int RecordManager::recordBlockFind(string tableName, vector<Condition>* conditionVector, blockNode* block)
{
    //if block is null, return -1
    if (block == NULL)
    {
        return -1;
    }
    int count = 0;

    char* recordBegin = bm.get_content(*block);
    vector<Attribute> attributeVector;
    int recordSize = api->recordSizeGet(tableName);

    api->attributeGet(tableName, &attributeVector);

    while (recordBegin - bm.get_content(*block)  < bm.get_usingSize(*block))
    {
        //if the recordBegin point to a record

        if(recordConditionFit(recordBegin, recordSize, &attributeVector, conditionVector))
        {
            count++;
        }

        recordBegin += recordSize;

    }

    return count;
}

/**
 *
 * delete all record of a table meet requirement
 * @param tableName: name of table
 * @param conditionVector: the conditions list
 * @return int: the number of the record meet requirements(-1 represent error)
 */
int RecordManager::recordAllDelete(string tableName, vector<Condition>* conditionVector)
{
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    int count = 0;
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (btmp->ifbottom)
        {
            int recordBlockNum = recordBlockDelete(tableName, conditionVector, btmp);
            count += recordBlockNum;
            return count;
        }
        else
        {
            int recordBlockNum = recordBlockDelete(tableName, conditionVector, btmp);
            count += recordBlockNum;
            btmp = bm.getNextBlock(ftmp, btmp);
        }
    }

    return -1;
}

/**
 *
 * delete record of a table in a block
 * @param tableName: name of table
 * @param conditionVector: the conditions list
 * @param blockOffset: the block's offsetNum
 * @return int: the number of the record meet requirements in the block(-1 represent error)
 */
int RecordManager::recordBlockDelete(string tableName,  vector<Condition>* conditionVector, int blockOffset)
{
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode* block = bm.getBlockByOffset(ftmp, blockOffset);
    if (block == NULL)
    {
        return -1;
    }
    else
    {
        return  recordBlockDelete(tableName, conditionVector, block);
    }
}

/**
 *
 * delete record of a table in a block
 * @param tableName: name of table
 * @param conditionVector: the conditions list
 * @param block: search record in the block
 * @return int: the number of the record meet requirements in the block(-1 represent error)
 */
int RecordManager::recordBlockDelete(string tableName,  vector<Condition>* conditionVector, blockNode* block)
{
    //if block is null, return -1
    if (block == NULL)
    {
        return -1;
    }
    int count = 0;

    char* recordBegin = bm.get_content(*block);
    vector<Attribute> attributeVector;
    int recordSize = api->recordSizeGet(tableName);

    api->attributeGet(tableName, &attributeVector);

    while (recordBegin - bm.get_content(*block) < bm.get_usingSize(*block))
    {
        //if the recordBegin point to a record

        if(recordConditionFit(recordBegin, recordSize, &attributeVector, conditionVector))
        {
            count ++;

            api->recordIndexDelete(recordBegin, recordSize, &attributeVector, block->offsetNum);//删除相应索引值
            int i = 0;
            for (i = 0; i + recordSize + recordBegin - bm.get_content(*block) < bm.get_usingSize(*block); i++)
            {
                recordBegin[i] = recordBegin[i + recordSize];//此时recordbegin后的所有记录向前挪一个
            }
            memset(recordBegin + i, 0, recordSize);//将最后一个置为零
			//void *memset(void *s,int c,size_t n)将已开辟内存空间 s 的首 n 个字节的值设为值 c
            bm.set_usingSize(*block, bm.get_usingSize(*block) - recordSize);
            bm.set_dirty(*block);//标记为脏块
        }
        else
        {
            recordBegin += recordSize;
        }
    }

    return count;
}

/**
 *
 * insert the index of all record of the table
 * @param tableName: name of table
 * @param indexName: name of index
 * @return int: the number of the record meet requirements(-1 represent error)
 */
int RecordManager::indexRecordAllAlreadyInsert(string tableName,string indexName)
{
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);
    int count = 0;
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (btmp->ifbottom)
        {
            int recordBlockNum = indexRecordBlockAlreadyInsert(tableName, indexName, btmp);
            count += recordBlockNum;
            return count;
        }
        else
        {
            int recordBlockNum = indexRecordBlockAlreadyInsert(tableName, indexName, btmp);
            count += recordBlockNum;
            btmp = bm.getNextBlock(ftmp, btmp);
        }
    }

    return -1;
}


/**
 *
 * insert the index of a record of a table in a block
 * @param tableName: name of table
 * @param indexName: name of index
 * @param block: search record in the block
 * @return int: the number of the record meet requirements in the block(-1 represent error)
 */
 int RecordManager::indexRecordBlockAlreadyInsert(string tableName,string indexName,  blockNode* block)//在一个块节点里，表名为**的每条记录检查是否需要建立indexName的索引
{
    //if block is null, return -1
    if (block == NULL)
    {
        return -1;
    }
    int count = 0;

    char* recordBegin = bm.get_content(*block);
    vector<Attribute> attributeVector;
    int recordSize = api->recordSizeGet(tableName);

    api->attributeGet(tableName, &attributeVector);

    int type;
    int typeSize;
    char * contentBegin;

    while (recordBegin - bm.get_content(*block)  < bm.get_usingSize(*block))//一条记录一条记录地找
    {
        contentBegin = recordBegin;
        //if the recordBegin point to a record
        for (int i = 0; i < attributeVector.size(); i++)//对一条记录的每个属性操作
        {
            type = attributeVector[i].type;
            typeSize = api->typeSizeGet(type);

            //find the index  of the record, and insert it to index tree
            if (attributeVector[i].index == indexName)//default value is "", representing no index
            {
                api->indexInsert(indexName, contentBegin, type, block->offsetNum);
                count++;
            }

            contentBegin += typeSize;//下一条属性
        }
        recordBegin += recordSize;//下一条记录
    }

    return count;
}

/**
 *
 * judge if the record meet the requirement
 * @param recordBegin: point to a record
 * @param recordSize: size of the record
 * @param attributeVector: the attribute list of the record
 * @param conditionVector: the conditions
 * @return bool: if the record fit the condition
 */
bool RecordManager::recordConditionFit(char* recordBegin,int recordSize, vector<Attribute>* attributeVector,vector<Condition>* conditionVector)
{
    if (conditionVector == NULL) {
        return true;
    }
    int type;
    string attributeName;
    int typeSize;
    char content[255];

    char *contentBegin = recordBegin;
    for(int i = 0; i < attributeVector->size(); i++)//一个一个属性操作
    {
        type = (*attributeVector)[i].type;//-1、0、正整数
        attributeName = (*attributeVector)[i].name;
        typeSize = api->typeSizeGet(type);

        //init content (when content is string , we can get a string easily)
        memset(content, 0, 255);
        memcpy(content, contentBegin, typeSize);//拷贝从contentBegin开始的地址的一块内容值
        for(int j = 0; j < (*conditionVector).size(); j++)//对每一个属性一个条件一个条件判断
        {
            if ((*conditionVector)[j].attributeName == attributeName)
            {
                //if this attribute need to deal about the condition
                if(!contentConditionFit(content, type, &(*conditionVector)[j]))//利用condition->ifRight(tmp)判断
                {
                    //if this record is not fit the conditon
                    return false;
                }
            }
        }

        contentBegin += typeSize;
    }
    return true;
}

/**
 *
 * print value of record
 * @param recordBegin: point to a record
 * @param recordSize: size of the record
 * @param attributeVector: the attribute list of the record
 * @param attributeVector: the name list of all attribute you want to print
 */
void RecordManager::recordPrint(char* recordBegin, int recordSize, vector<Attribute>* attributeVector, vector<string> *attributeNameVector)
{
    int type;
    string attributeName;
    int typeSize;
    char content[255];

    char *contentBegin = recordBegin;
    for(int i = 0; i < attributeVector->size(); i++)//表的属性遍历，一条条属性来
    {
        type = (*attributeVector)[i].type;
        typeSize = api->typeSizeGet(type);//每个类型固定大小

        //init content (when content is string , we can get a string easily)
        memset(content, 0, 255);

        memcpy(content, contentBegin, typeSize);//获取地址从contentBegin开始的一块内容

        for(int j = 0; j < (*attributeNameVector).size(); j++)//attributeVector含一个表的所有属性，attributeNameVector，用户输进去的属性列
        {
            if ((*attributeNameVector)[j] == (*attributeVector)[i].name)
            {
                contentPrint(content, type);//type是一个不同输出时if函数判断条件
                break;
            }
        }

        contentBegin += typeSize;
    }
}

/**
 *
 * print value of content
 * @param content: point to content
 * @param type: type of content
 */
void RecordManager::contentPrint(char * content, int type)
{
    if (type == Attribute::TYPE_INT)
    {
        //if the content is a int
        int tmp = *((int *) content);   //get content value by point
        printf("%d ", tmp);
    }
    else if (type == Attribute::TYPE_FLOAT)
    {
        //if the content is a float
        float tmp = *((float *) content);   //get content value by point
        printf("%f ", tmp);
    }
    else
    {
        //if the content is a string
        string tmp = content;
        printf("%s ", tmp.c_str());
    }

}

/**
 *
 * judge if the content meet the requirement
 * @param content: point to content
 * @param type: type of content
 * @param condition: condition
 * @return bool: the content if meet
 */
bool RecordManager::contentConditionFit(char* content,int type,Condition* condition)
{
    if (type == Attribute::TYPE_INT)
    {
        //if the content is a int
        int tmp = *((int *) content);   //get content value by point
        return condition->ifRight(tmp);
    }
    else if (type == Attribute::TYPE_FLOAT)
    {
        //if the content is a float
        float tmp = *((float *) content);   //get content value by point
        return condition->ifRight(tmp);
    }
    else
    {
        //if the content is a string
        return condition->ifRight(content);
    }
    return true;
}

/**
 *
 * get a index's file name
 * @param indexName: name of index
 */
string RecordManager::indexFileNameGet(string indexName)
{
    string tmp = "";
    return "INDEX_FILE_"+indexName;
}

/**
 *
 * get a table's file name
 * @param tableName: name of table
 */
string RecordManager::tableFileNameGet(string tableName)
{
    string tmp = "";
    return tmp + "TABLE_FILE_" + tableName;
}
