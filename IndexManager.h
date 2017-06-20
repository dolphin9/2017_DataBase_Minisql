//
//  IndexManager.h
//  Minisql
//  Description: Provide functions to manage the indexs.
//
//  Created by 陈泓宇 on 20/6/2017.
//  Copyright (c) 2017年 陈泓宇. All rights reserved.
//

#ifndef __Minisql__IndexManager__
#define __Minisql__IndexManager__

#include <stdio.h>
#include <map>
#include <string>
#include <sstream>
#include "Attribute.h"
#include "BPlusTree.h"

class API;

class IndexManager{
private:
    typedef map<string,BPlusTree<int> *> intMap;//第一个索引文件名，第二个B+树
	//索引值为int类型B+树的映射，Map是STL的一个关联容器，它提供一对一，第一个可以称为关键字，每个关键字只能在map中出现一次，第二个可能称为该关键字的值
    typedef map<string,BPlusTree<string> *> stringMap;
    typedef map<string,BPlusTree<float> *> floatMap;

    int static const TYPE_FLOAT = Attribute::TYPE_FLOAT;//常量初始化
    int static const TYPE_INT = Attribute::TYPE_INT;
    // other values mean the size of the char.Eg, 4 means char(4);

    API *api; // to call the functions of API

    intMap indexIntMap;//自定义类型int型映射
    stringMap indexStringMap;
    floatMap indexFloatMap;
    struct keyTmp{
        int intTmp;
        float floatTmp;
        string stringTmp;
    }; // the struct to help to convert the inputed string to specfied type
    struct keyTmp kt;

    int getDegree(int type);//获得B+树的分叉度数

    int getKeySize(int type);//返回类型为type的类型大小所占字节数sizeof

    void setKey(int type,string key);//将字符串根据type类型转化为相应类型的基本数据类型数据


public:
    IndexManager(API *api);//构造函数，一构造就将所有应该有的索引建立好了
    ~IndexManager();//析构函数， write the dirty indexs back to the disk.

    void createIndex(string filePath,int type);// Create index on the specific type.* If there exists the index before, read data from file path and then rebuild the b+ tree.

    void dropIndex(string filePath,int type);

    offsetNumber searchIndex(string filePath,string key,int type);//Search the b+ tree by the key and return the value of that key./offsetNumber int 型

    void insertIndex(string filePath,string key,offsetNumber blockOffset,int type);// Insert the key and value to the b+ tree.

    void deleteIndexByKey(string filePath,string key,int type);
};



#endif /* defined(__Minisql__IndexManager__) */
