//
//  IndexManager.cpp
//  Minisql
//
//  Created by xuyuhao on 14/11/14.
//  Copyright (c) 2014年 xuyuhao. All rights reserved.
//

#include "IndexManager.h"
#include <iostream>
#include "API.h"
#include "IndexInfo.h"
#include <vector>//用到迭代器
using namespace std;


/**
 *  Constructor Function: create the existing index by the index files.//构造函数
 *
 *  @param API*
 */
IndexManager::IndexManager(API *m_api)//API在API.h和API.cpp里
{
    api = m_api;
    vector<IndexInfo> allIndexInfo;
    api->allIndexAddressInfoGet(&allIndexInfo);//所有索引文件的名字赋值给向量allIndexInfo
    for(vector<IndexInfo>::iterator i = allIndexInfo.begin();i != allIndexInfo.end();i ++)//第一个索引信息，到最后一个
    {
        createIndex(i->indexName, i->type);//每一个创建索引，初始化一颗新的b+树，将具体的索引文件名和b+关联起来
    }
}


/**
 *  Destructor Function: write the dirty indexs back to the disk.
 *
 */
IndexManager::~IndexManager()
{
    //write back to the disk
    for(intMap::iterator itInt = indexIntMap.begin();itInt != indexIntMap.end(); itInt ++)
    {
        if(itInt->second)//若第二个值value不为0，则写灰磁盘，再删除
        {
            itInt -> second->writtenbackToDiskAll();
            delete itInt->second;
        }
    }
    for(stringMap::iterator itString = indexStringMap.begin();itString != indexStringMap.end(); itString ++)
    {
        if(itString->second)
        {
            itString ->second-> writtenbackToDiskAll();
            delete itString->second;
        }
    }
    for(floatMap::iterator itFloat = indexFloatMap.begin();itFloat != indexFloatMap.end(); itFloat ++)
    {
        if(itFloat->second)
        {
            itFloat ->second-> writtenbackToDiskAll();
            delete itFloat->second;
        }
    }
}


/**
 * Create index on the specific type.
 * If there exists the index before, read data from file path and then rebuild the b+ tree.
 *
 * @param string the file path
 * @param int type
 *
 * @return void
 *
 */
void IndexManager::createIndex(string filePath,int type)
{
    int keySize = getKeySize(type);//返回type类型的所占字节数，
    int degree = getDegree(type);//B+分叉度数
    if(type == TYPE_INT)
    {
        BPlusTree<int> *tree = new BPlusTree<int>(filePath,keySize,degree);//初始化一棵int型B+树
        indexIntMap.insert(intMap::value_type(filePath, tree));
    }
    else if(type == TYPE_FLOAT)
    {
        BPlusTree<float> *tree = new BPlusTree<float>(filePath,keySize,degree);
        indexFloatMap.insert(floatMap::value_type(filePath, tree));
    }
    else // string
    {
        BPlusTree<string> *tree = new BPlusTree<string>(filePath,keySize,degree);
        indexStringMap.insert(stringMap::value_type(filePath, tree));
    }

}

/**
 * Drop the specific index.
 *
 * @param
 *
 * @return void
 *
 */
void IndexManager::dropIndex(string filePath,int type)
{
    if(type == TYPE_INT)
    {
        intMap::iterator itInt = indexIntMap.find(filePath);//关联器自己组成序列吗？
		//Find 返回一个迭代器，指定的排序关键字的第一个元素等于键。如果没有这样的元素存在，则迭代器等于 end()。
        if(itInt == indexIntMap.end())
        {
            cout << "Error:in drop index, no index " << filePath <<" exits" << endl;
            return;
        }
        else
        {
            delete itInt->second;//删除b+树
            indexIntMap.erase(itInt);//擦除迭代器
        }
    }
    else if(type == TYPE_FLOAT)
    {
        floatMap::iterator itFloat = indexFloatMap.find(filePath);
        if(itFloat == indexFloatMap.end())
        {
            cout << "Error:in drop index, no index " << filePath <<" exits" << endl;
            return;
        }
        else
        {
            delete itFloat->second;
            indexFloatMap.erase(itFloat);
        }
    }
    else // string
    {
        stringMap::iterator itString = indexStringMap.find(filePath);
        if(itString == indexStringMap.end())
        {
            cout << "Error:in drop index, no index " << filePath <<" exits" << endl;
            return;
        }
        else
        {
            delete itString->second;
            indexStringMap.erase(itString);
        }
    }

}

/**
 * Search the b+ tree by the key and return the value of that key.
 * If the key is not in the index, return -1.
 *
 * @param string the file path.
 * @param string key.
 * @param int type
 *
 * @return offsetNumber(int型）
 *
 */
offsetNumber IndexManager::searchIndex(string filePath,string key,int type)//失败返回-1，否则返回位置offsetNumber
{
    setKey(type, key);

    if(type == TYPE_INT)
    {
        intMap::iterator itInt = indexIntMap.find(filePath);
        if(itInt == indexIntMap.end())//序列最后一个说明没有找到
        {
            cout << "Error:in search index, no index " << filePath <<" exits" << endl;
            return -1;
        }
        else
        {
            return itInt->second->search(kt.intTmp);
        }
    }
    else if(type == TYPE_FLOAT)
    {
        floatMap::iterator itFloat = indexFloatMap.find(filePath);
        if(itFloat == indexFloatMap.end())
        {
            cout << "Error:in search index, no index " << filePath <<" exits" << endl;
            return -1;
        }
        else
        {
            return itFloat->second->search(kt.floatTmp);

        }
    }
    else // string
    {
        stringMap::iterator itString = indexStringMap.find(filePath);
        if(itString == indexStringMap.end())
        {
            cout << "Error:in search index, no index " << filePath <<" exits" << endl;
            return -1;
        }
        else
        {
            return itString->second->search(key);
        }
    }
}

/**
 * Insert the key and value to the b+ tree.
 *
 * @param string the file path
 * @param string key
 * @param offsetNumber the block offset number
 * @param int type
 *
 * @return void
 *
 */
void IndexManager::insertIndex(string filePath,string key,offsetNumber blockOffset,int type)//filePath-索引文件名字，先找到对应的关联器，通过第二个值b+树修改增加索引
{
    setKey(type, key);//将string转化为具体基本数值类型

    if(type == TYPE_INT)
    {
        intMap::iterator itInt = indexIntMap.find(filePath);
        if(itInt == indexIntMap.end())
        {
            cout << "Error:in search index, no index " << filePath <<" exits" << endl;
            return;
        }
        else
        {
            itInt->second->insertKey(kt.intTmp,blockOffset);//kt是转到具体类型的结构体，含有要插入的值
        }
    }
    else if(type == TYPE_FLOAT)
    {
        floatMap::iterator itFloat = indexFloatMap.find(filePath);
        if(itFloat == indexFloatMap.end())
        {
            cout << "Error:in search index, no index " << filePath <<" exits" << endl;
            return;
        }
        else
        {
            itFloat->second->insertKey(kt.floatTmp,blockOffset);

        }
    }
    else // string
    {
        stringMap::iterator itString = indexStringMap.find(filePath);
        if(itString == indexStringMap.end())
        {
            cout << "Error:in search index, no index " << filePath <<" exits" << endl;
            return;
        }
        else
        {
            itString->second->insertKey(key,blockOffset);
        }
    }
}

/**
 * Delete the key and its value
 *
 * @param string file path
 * @param int type
 *
 * @return void
 *
 */
void IndexManager::deleteIndexByKey(string filePath,string key,int type)//先找到对应关联器，第二个值b+树进行删除操作
{
    setKey(type, key);//先转化一下

    if(type == TYPE_INT)
    {
        intMap::iterator itInt = indexIntMap.find(filePath);
        if(itInt == indexIntMap.end())
        {
            cout << "Error:in search index, no index " << filePath <<" exits" << endl;
            return;
        }
        else
        {
            itInt->second->deleteKey(kt.intTmp);
        }
    }
    else if(type == TYPE_FLOAT)
    {
        floatMap::iterator itFloat = indexFloatMap.find(filePath);
        if(itFloat == indexFloatMap.end())
        {
            cout << "Error:in search index, no index " << filePath <<" exits" << endl;
            return;
        }
        else
        {
            itFloat->second->deleteKey(kt.floatTmp);

        }
    }
    else // string
    {
        stringMap::iterator itString = indexStringMap.find(filePath);
        if(itString == indexStringMap.end())
        {
            cout << "Error:in search index, no index " << filePath <<" exits" << endl;
            return;
        }
        else
        {
            itString->second->deleteKey(key);
        }
    }
}

/**
 * Get the degree by the type.
 * The tree node size equals to the block size.
 *
 * @param int type
 *
 * @return int the degree
 *
 */
int IndexManager::getDegree(int type)//根据属性类型计算b+树分叉度数
{
    int degree = bm.getBlockSize()/(getKeySize(type)+sizeof(offsetNumber));
    if(degree %2 == 0) degree -= 1;
    return degree;
}

/**
 * Get the key size by the type
 *
 * @param int type
 *
 * @return int the key size
 *
 */
int IndexManager::getKeySize(int type)//获得类型大小，占字节数多少
{
    if(type == TYPE_FLOAT)
        return sizeof(float);
    else if(type == TYPE_INT)
        return sizeof(int);
    else if(type > 0)
        return type + 1;//"\0"
    else
    {
        cout << "ERROR: in getKeySize: invalid type" << endl;
        return -100;
    }
}

/**
 * Get the key of its type by the inputed string.
 * Users input string whatever type the key is.
 *
 * @param int type
 * @param string key
 *
 * @return void
 *
 */
void IndexManager::setKey(int type,string key)//将输入的key转化到对应类型，存储在kt结构里
{
    stringstream ss;
    ss << key;
    if(type == this->TYPE_INT)
        ss >> this->kt.intTmp;
    else if(type == this->TYPE_FLOAT)
        ss >> this->kt.floatTmp;
    else if(type > 0)
        ss >> this->kt.stringTmp;
    else
        cout << "Error: in getKey: invalid type" << endl;

}
