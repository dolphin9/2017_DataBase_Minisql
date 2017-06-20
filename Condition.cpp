//
//  Condition.cpp
//  minisql
//
//  Created by 陈泓宇 on 17/16/21.
//  Copyright (c) 2017年 陈泓宇. All rights reserved.
//

#include "Condition.h"
bool Condition::ifRight(int content)
{
    stringstream ss;//stringstream输入输出操作
    ss << value;
    int myContent;
    ss >> myContent;

    switch (operate)
    {
        case Condition::OPERATOR_EQUAL:
            return content == myContent;
            break;
        case Condition::OPERATOR_NOT_EQUAL:
            return content != myContent;
            break;
        case Condition::OPERATOR_LESS:
            return content < myContent;
            break;
        case Condition::OPERATOR_MORE:
            return content > myContent;
            break;
        case Condition::OPERATOR_LESS_EQUAL:
            return content <= myContent;
            break;
        case Condition::OPERATOR_MORE_EQUAL:
            return content >= myContent;
            break;
        default:
            return true;
            break;
    }
}

bool Condition::ifRight(float content)
{
    stringstream ss;
    ss << value;
    float myContent;
    ss >> myContent;

    switch (operate)
    {
        case Condition::OPERATOR_EQUAL:
            return content == myContent;
            break;
        case Condition::OPERATOR_NOT_EQUAL:
            return content != myContent;
            break;
        case Condition::OPERATOR_LESS:
            return content < myContent;
            break;
        case Condition::OPERATOR_MORE:
            return content > myContent;
            break;
        case Condition::OPERATOR_LESS_EQUAL:
            return content <= myContent;
            break;
        case Condition::OPERATOR_MORE_EQUAL:
            return content >= myContent;
            break;
        default:
            return true;
            break;
    }
}

bool Condition::ifRight(string content)
{
    string myContent = value;
    switch (operate)
    {
        case Condition::OPERATOR_EQUAL:
            return content == myContent;
            break;
        case Condition::OPERATOR_NOT_EQUAL:
            return content != myContent;
            break;
        case Condition::OPERATOR_LESS:
            return content < myContent;
            break;
        case Condition::OPERATOR_MORE:
            return content > myContent;
            break;
        case Condition::OPERATOR_LESS_EQUAL:
            return content <= myContent;
            break;
        case Condition::OPERATOR_MORE_EQUAL:
            return content >= myContent;
            break;
        default:
            return true;
            break;
    }
}

Condition::Condition(string a,string v,int o) {//构造函数
    attributeName = a;
    value = v;
    operate = o;
}
