#include<iostream>
#include<string>
#include<sstream>
#include<stack>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"define.h"
#include"semantic.h"

using namespace std;

SemanticAnalyzer::SemanticAnalyzer()
{

}
   

bool SemanticAnalyzer::typeMatch(Token& a,Token& b,TableManager& tmg)//检查两个Token的数据类型是否相同
{
    TypeCode type_a,type_b;

    if(a.type==CONSTANT)
        type_a = tmg.tab_constant.ele.at(a.idx).type_code;
    else
        type_a = tmg.at(a).type;

    if(b.type==CONSTANT)
        type_b = tmg.tab_constant.ele.at(b.idx).type_code;
    else
        type_b = tmg.at(b).type;    

    return (type_a == type_b);

}

void SemanticAnalyzer::MainProgram(Index idx,TableManager& tmg)
{
    // 生成四元式
    Quaternary quat;
    quat.op = "program";
    char tmp[MINLENGTH] = "";
    sprintf(tmp,"I(%d)",idx-tmg.tab_main.end2);
    quat.src1 = tmp;
    quat.src2 = "_";
    quat.target = "_";
    this->quatlist.push_back(quat);

    // 填内存分配表
    MemTableItem item;
    item.stype = M_PROGRAM;
    item.size  = 0;
    item.offset= 0;
    item.idx   = 0;
    tmg.tab_mem.ele.push_back(item);
}

void SemanticAnalyzer::Const(TableManager& tmg,Index idx_main,Index idx_const,TypeCode code)
{
    // 填常量表
    tmg.tab_main.ele.at(idx_main).type = code;
    tmg.tab_main.ele.at(idx_main).cat = C;
    tmg.tab_main.ele.at(idx_main).address = idx_const;
}

void SemanticAnalyzer::Var(TableManager& tmg,TypeCode code,Index idx_main)
{
    // 填主表
    tmg.tab_main.ele.at(idx_main).type = code;
    tmg.tab_main.ele.at(idx_main).cat = V;
}


void SemanticAnalyzer::Assign(Token& left,Token& right,TableManager& tmg)
{
    if(right.type == CONSTANT)//常数
    {
        if(this->typeMatch(left,right,tmg) == false)
            errorHandle(SEMANTIC_ANALYZER,"Data type dismatching in assign statement");
        Quaternary quat;
        quat.op = ":=";
        stringstream tmp;
        tmp<<"C("<<right.idx<<")";
        tmp>>quat.src1;
        quat.src2 = "_";
        tmp.clear();
        tmp<<"I("<<left.idx - tmg.tab_main.end2<<")";
        tmp>>quat.target;
        this->quatlist.push_back(quat);
    }
    else if(right.type==IDENTIFIER && tmg.at(right).cat==C)//常量
    {
        if(this->typeMatch(left,right,tmg) == false)
            errorHandle(SEMANTIC_ANALYZER,"Data type dismatching in assign statement");
        Quaternary quat;
        quat.op = ":=";
        stringstream tmp;
        tmp<<"I("<<right.idx - tmg.tab_main.end2<<")";
        tmp>>quat.src1;
        quat.src2 = "_";
        tmp.clear();
        tmp<<"I("<<left.idx - tmg.tab_main.end2<<")";
        tmp>>quat.target;
        this->quatlist.push_back(quat);        
    }
    else if(right.type==IDENTIFIER && (tmg.at(right).cat==V || tmg.at(right).cat==M))//用户变量或中间变量
    {
        if(this->typeMatch(left,right,tmg) == false)
            errorHandle(SEMANTIC_ANALYZER,"Data type dismatching in assign statement");
        Quaternary quat;
        quat.op = ":=";
        stringstream tmp;
        tmp<<"I("<<right.idx - tmg.tab_main.end2 <<")";
        tmp>>quat.src1;
        quat.src2 = "_";
        tmp.clear();
        tmp<<"I("<<left.idx - tmg.tab_main.end2<<")";
        tmp>>quat.target;
        this->quatlist.push_back(quat);
    }
    else 
        errorHandle(SEMANTIC_ANALYZER,"Invalid assign statement");
}

Token SemanticAnalyzer::Expression(TableManager& tmg,vector<Token>& tokens)
{
    stack<Token> stack_op;  //算符栈
    stack<Token> stack_arg; //操作数栈
    unsigned int cur = 0;   
    unsigned int len = tokens.size();
    Token token = tokens.at(cur);   //当前Token

    while(cur != len)//读取所有的Token
    {
        token = tokens.at(cur);

        if(token.type==IDENTIFIER || token.type==CONSTANT)//遇到变量、常量或常数
        {
            stack_arg.push(token);
            cur++;
        }
        else//遇到符号
        {
            if(tmg.at(token).name=="(")
            {//直接压栈
                stack_op.push(token);
                cur++;
            }
            else if(tmg.at(token).name==")")
            {   
                //不断出栈运算直到遇到'('
                while(stack_op.empty()==false && tmg.at(stack_op.top()).name!="(")
                {
                    // 获取算符、操作数1、操作数2
                    Token arg1,arg2,op;
                    arg2 = stack_arg.top();
                    stack_arg.pop();
                    arg1 = stack_arg.top();
                    stack_arg.pop();
                    op = stack_op.top();
                    stack_op.pop();
                    // 获取中间变量(运算结果保存与此)
                    TypeCode type;
                    if(arg1.type==IDENTIFIER)
                        type = tmg.at(arg1).type;
                    else 
                        type = tmg.tab_constant.ele.at(arg1.idx).type_code;    
                    Token target = tmg.newT(type);
                    // 运算结果压栈
                    stack_arg.push(target);

                    // 生成四元式
                    Quaternary quat;
                    quat.op = tmg.at(op).name;
                    quat.src1 = tmg.getId(arg1);
                    quat.src2 = tmg.getId(arg2);
                    quat.target = tmg.getId(target);
                    this->quatlist.push_back(quat);
                }

                //丢掉 '('  ')'
                stack_op.pop();
                cur++;
            }
            else if(stack_op.empty())
            {//栈为空，直接压栈
                stack_op.push(token);
                cur++;        
            }
            else
            {
                //若当前运算符优先级高于栈顶运算符优先级则压栈，否则出栈
                if( this->OpPriority(tmg.at(token).name, tmg.at(stack_op.top()).name) )
                {
                    stack_op.push(token);
                    cur++;
                }
                else
                {
                    // 获取算符、操作数1、操作数2
                    Token arg1,arg2,op;
                    arg2 = stack_arg.top();
                    stack_arg.pop();
                    arg1 = stack_arg.top();
                    stack_arg.pop();
                    op = stack_op.top();
                    stack_op.pop();
                    // 获取中间变量(运算结果保存与此)
                    TypeCode type;
                    if(arg1.type==IDENTIFIER)
                        type = tmg.at(arg1).type;
                    else 
                        type = tmg.tab_constant.ele.at(arg1.idx).type_code;    
                    Token target = tmg.newT(type);
                    // 运算结果压栈
                    stack_arg.push(target);

                    // 生成四元式
                    Quaternary quat;
                    quat.op = tmg.at(op).name;
                    quat.src1 = tmg.getId(arg1);
                    quat.src2 = tmg.getId(arg2);
                    quat.target = tmg.getId(target);
                    this->quatlist.push_back(quat);
                }
            }
        }
    }

    while(stack_op.empty() == false)//处理栈中剩余的部分
    {
        // 获取算符、操作数1、操作数2
        Token arg1,arg2,op;
        arg2 = stack_arg.top();
        stack_arg.pop();
        arg1 = stack_arg.top();
        stack_arg.pop();
        op = stack_op.top();
        stack_op.pop();
        // 获取中间变量(运算结果保存与此)
        TypeCode type;
        if(arg1.type==IDENTIFIER)
            type = tmg.at(arg1).type;
        else 
            type = tmg.tab_constant.ele.at(arg1.idx).type_code;    
        Token target = tmg.newT(type);
        // 运算结果压栈
        stack_arg.push(target);

        // 生成四元式
        Quaternary quat;
        quat.op = tmg.at(op).name;
        quat.src1 = tmg.getId(arg1);
        quat.src2 = tmg.getId(arg2);
        quat.target = tmg.getId(target);
        this->quatlist.push_back(quat);        
    }

    // 返回操作数栈顶元素(注意top返回的是对象的引用)
    token.idx = stack_arg.top().idx;
    token.type = stack_arg.top().type;
    return token;
}

bool SemanticAnalyzer::OpPriority(string op1,string op2)
{
    // op1: + - * / 
    // op2: + - * / (
    
    if(op1=="+" || op1=="-")
    {
        if(op2=="(")
            return true;
        else
            return false;
    }
    else if(op1=="*" || op2=="/")
    {
        if(op2=="(" || op2=="+" || op2=="-")
            return true;
        else
            return false;
    }
}

Token SemanticAnalyzer::Relation(TableManager& tmg,Token& token_l,Token& token_r,string& rea_symbol)//生成关系表达式的四元式
{
    Quaternary quat;
    quat.op = rea_symbol;
    quat.src1 = tmg.getId(token_l);
    quat.src2 = tmg.getId(token_r);
    Token t = tmg.newT(BOOL);
    quat.target = tmg.getId(t);
    this->quatlist.push_back(quat);
    return t;
}

Token SemanticAnalyzer::Logic(TableManager& tmg,Token& token_l,Token& token_r,string& logic_symbol)//生成逻辑简单逻辑表达式的四元式
{
    Quaternary quat;
    quat.op = logic_symbol;

    if(token_l.type==KEYWORD)
        quat.src1 = tmg.at(token_l).name;
    else 
        quat.src1 = tmg.getId(token_l);
    if(token_r.type==KEYWORD)
        quat.src2 = tmg.at(token_r).name;
    else 
        quat.src2 = tmg.getId(token_r);    

    Token t = tmg.newT(BOOL);
    quat.target = tmg.getId(t);
    this->quatlist.push_back(quat);
    return t;
}


void SemanticAnalyzer::If(Token& token,TableManager& tmg) //生成if四元式
{
    Quaternary quat;
    quat.op = "if";
    quat.src1 = tmg.getId(token);
    quat.src2 = "_";
    quat.target = "";
    this->quatlist.push_back(quat);
}
void SemanticAnalyzer::El()//生成else四元式
{
    Quaternary quat;
    quat.op = "else";
    quat.src1 = "_";
    quat.src2 = "_";
    quat.target = "_";
    this->quatlist.push_back(quat);    
}
void SemanticAnalyzer::Ie() //生成ifend四元式
{
    Quaternary quat;
    quat.op = "ie";
    quat.src1 ="_";
    quat.src2 = "_";
    quat.target = "_";
    this->quatlist.push_back(quat); 
}

void SemanticAnalyzer::Wh()//生成while四元式
{
    Quaternary quat;
    quat.op = "wh";
    quat.src1 = "_";
    quat.src2 = "_";
    quat.target = "_";
    this->quatlist.push_back(quat); 
}

void SemanticAnalyzer::Do(Token& token,TableManager& tmg)//生成do四元式
{
    Quaternary quat;
    quat.op = "do";
    quat.src1 = tmg.getId(token);
    quat.src2 ="_";
    quat.target = "_";
    this->quatlist.push_back(quat);
}

void SemanticAnalyzer::We()//生成while-end四元式
{
    Quaternary quat;
    quat.op = "we";
    quat.src1 = "_";
    quat.src2 = "_";
    quat.target = "_";
    this->quatlist.push_back(quat); 
}