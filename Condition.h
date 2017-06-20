//
//  Condition.h
//  minisql
//
//  Created by 陈泓宇 on 2017/6/19.
//  Copyright (c) 2017年 陈泓宇. All rights reserved.
//

#ifndef minisql_Condition_h
#define minisql_Condition_h
#include <string>
#include <sstream>
using namespace std;

class Condition//条件
{

public:
    const static int OPERATOR_EQUAL = 0; // "="
    const static int OPERATOR_NOT_EQUAL = 1; // "<>"
    const static int OPERATOR_LESS = 2; // "<"
    const static int OPERATOR_MORE = 3; // ">"
    const static int OPERATOR_LESS_EQUAL = 4; // "<="
    const static int OPERATOR_MORE_EQUAL = 5; // ">="

    Condition(string a,string v,int o);

    string attributeName;//关于哪个属性的查询条件
    string value;           // the value to be compared
    int operate;            // the type to be compared

    bool ifRight(int content);//三种类型的数据比较
    bool ifRight(float content);
    bool ifRight(string content);
};

#endif
