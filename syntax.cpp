#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string>
#include<exception>
#include<sstream>
#include"define.h"
#include"syntax.h"
#include"semantic.h"

using namespace std;

SyntaxAnalyzer::SyntaxAnalyzer()
{
    this->pos = 0;
}

void SyntaxAnalyzer::getTokens(LexicalAnalyzer& lex,TableManager& tmg) //调用词法分析器，将源代码转换为Token序列
{
    Token token;
    while(lex.fileEnd() == false)//文件未结束则继续
    {
        if(lex.oneToken(tmg,token)) //Token有效则接受
            this->tokenlist.push_back(token);
    }
}

void SyntaxAnalyzer::printTokens(TableManager& tmg)//打印所有Token信息
{
    cout<<"-------------------Syntax Analyze Result----------------------"<<endl;
    cout<<"--------------------------------------------------------------"<<endl;
    for(auto i: this->tokenlist)
    {
        if(i.type == KEYWORD)
        {
            cout<<"keyword:       "<<tmg.at(i).name<<endl;
        }
        else if(i.type == DELIMITER)
        {
            cout<<"delimiter:     "<<tmg.at(i).name<<endl;
        }
        else if(i.type == IDENTIFIER)
        {
            cout<<"identifier:    "<<tmg.at(i).name<<endl;
        }
        else
        {
            cout<<"constant:      "<<tmg.tab_constant.ele.at(i.idx).value<<endl;
        }
    }
    cout<<"--------------------------------------------------------------"<<endl;

}

Token SyntaxAnalyzer::fetchToken()//每次获取一个Token
{
    try
    {
        Token t = this->tokenlist.at(this->pos++);
        return t;
    }
    catch(const std::exception& e)
    {
        errorHandle(SYNTAX_ANALYZER,"Program is incomplete");
    }
    
}

bool SyntaxAnalyzer::nameInsert(string name)//若标识符已经存在，则插入失败，返回false，否则返回true
{
    set<string>::iterator iter;
    iter = this->names.find(name);
    if(iter == this->names.end())
    {
        this->names.insert(name);
        return true;
    }
    return false;
}

bool SyntaxAnalyzer::isEnd() //当前Token是否是最后一个
{
    return (this->pos == this->tokenlist.size());
}

void SyntaxAnalyzer::assignStatement(Token& t,TableManager& tmg,SemanticAnalyzer& sem)//赋值语句分析
{
    Index  cat = tmg.tab_main.ele.at(t.idx).cat;
    if(cat == V)//普通局部变量
    {
        Token left = t;  //赋值符号左边Token的序号
        t = this->fetchToken();
        if(!(t.type==DELIMITER && tmg.at(t).name==":="))
            errorHandle(SYNTAX_ANALYZER,"Invalid statement");

        // 保存赋值符号右边的Token
        vector<Token> right;
        t = this->fetchToken();
        while(!(t.type==DELIMITER && tmg.at(t).name==";"))
        {
            right.push_back(t);
            t = this->fetchToken();
        }
        t = this->fetchToken(); //丢掉 ';', 读取下一个Token

        // 分析right中的Token
        if(right.size() == 0)//出错
            errorHandle(SYNTAX_ANALYZER,"Illegal assign statement");
        else if(right.size() == 1)//常数、常量、变量赋值
            sem.Assign(left,right.back(),tmg);
        else//表达式赋值
        {
            Token right_result;
            if(this->isLogic(tmg,right) == true)//逻辑表达式，含有and,or,not
                right_result = this->logic(tmg,sem,right);
            else if(this->isRelation(tmg,right) == true)//关系表达式，含有关系运算符
                right_result = this->relation(tmg,sem,right);
            else //算术表达式
                right_result = this->expression(tmg,sem,right);
                
            sem.Assign(left,right_result,tmg);
        }
    }
    else if(cat == C)//常量
    {
        char info[MINLENGTH] = "";
        sprintf(info,"Constant can't be assigned");
        errorHandle(SYNTAX_ANALYZER,info);
    }
    else//其他
    {
        char info[MINLENGTH] = "";
        sprintf(info,"Undefined variable is invalid");
        errorHandle(SYNTAX_ANALYZER,info);  
    }
}


Token SyntaxAnalyzer::expression(TableManager& tmg,SemanticAnalyzer& sem,vector<Token>& tokens)//算术表达式分析
{
    // LL1分析方法
    if(this->E_LL1(tmg,tokens) == false)
        errorHandle(SYNTAX_ANALYZER,"Invalid expression");
    // 语法分析正确，执行语义动作
    Token token = sem.Expression(tmg,tokens);
    return token;
}

Token SyntaxAnalyzer::relation(TableManager& tmg,SemanticAnalyzer& sem,vector<Token>& tokens)//关系表达式分析
{
    vector<Token> l,r;  //关系运算符左右两边的Token
    string rea_symbol; //关系运算符 
    int i = 0;  //关系运算符在tokens中的位置
    Token tmp; 
    for(;i<tokens.size();i++)
    {
        tmp = tokens[i];
        if(tmp.type==DELIMITER &&(tmg.at(tmp).name==">"||tmg.at(tmp).name=="<"||tmg.at(tmp).name=="="||tmg.at(tmp).name==">="||tmg.at(tmp).name=="<="||tmg.at(tmp).name=="<>"))
        {
            rea_symbol = tmg.at(tmp).name;
            break;
        }
        l.push_back(tmp);
    }
    i++;
    for(int j=i;j<tokens.size();j++)
        r.push_back(tokens[j]);
    
    Token token_l,token_r,result;
    if(l.size()==0 || r.size()==0)
        errorHandle(SYNTAX_ANALYZER,"Invalid relation expression");
    //关系运算符左边
    if(l.size()==1)
        token_l = l[0];
    else if(this->E_LL1(tmg,l) == true)
        token_l = sem.Expression(tmg,l);
    else 
        errorHandle(SYNTAX_ANALYZER,"Invalid relation expression");
    //关系运算符右边
    if(r.size()==1)
        token_r = r[0];
    else if(this->E_LL1(tmg,r) == true)
        token_r = sem.Expression(tmg,r);
    else 
        errorHandle(SYNTAX_ANALYZER,"Invalid relation expression");
    
    result  = sem.Relation(tmg,token_l,token_r,rea_symbol);
    return result;
}

bool SyntaxAnalyzer::isRelation(TableManager& tmg,vector<Token>& tokens)//判断一个表达式是不是关系表达式
{
    for(auto i: tokens)
    {
        if(i.type==DELIMITER && (tmg.at(i).name==">"||tmg.at(i).name=="<"||tmg.at(i).name=="="||tmg.at(i).name==">="||tmg.at(i).name=="<="||tmg.at(i).name=="<>"))
            return true;
    }
    return false;    
}

Token SyntaxAnalyzer::logic(TableManager& tmg,SemanticAnalyzer& sem,vector<Token>& tokens)//简单逻辑表达式
{

    if(this->isLogic(tmg,tokens) == false)//没有'and','or','not'连词
    {
        if(this->isRelation(tmg,tokens) == true)//关系表达式也是逻辑表达式
        {
            return this->relation(tmg,sem,tokens);
        }
        else 
            errorHandle(SYNTAX_ANALYZER,"Invalid logic expression");
    }

    //将逻辑表达式分割成两个部分
    string logic_symbol;
    vector<Token> l,r;  //'and'或'or'符号左右两边的表达式
    Token t;
    int i = 0;
    for(;i<tokens.size();i++)
    {
        t = tokens[i];
        if((t.type==KEYWORD &&(tmg.at(t).name=="and"||tmg.at(t).name=="or")))
        {
            logic_symbol = tmg.at(t).name;
            break;
        }
        l.push_back(t);
    }
    i++;
    for(int j=i;j<tokens.size();j++)
        r.push_back(tokens[j]);
    
    //处理左边
    Token result_left;
    if(l.size()==0)
        errorHandle(SYNTAX_ANALYZER,"Invalid logic expression");
    else if(l.size()==1)
    {
        t = l[0];
        if(t.type==KEYWORD &&(tmg.at(t).name=="true" || tmg.at(t).name=="false"))
        {
            result_left = t;
        }
        else if(t.type==IDENTIFIER && tmg.at(t).type==BOOL)
        {
            result_left = t;
        }
        else 
            errorHandle(SYNTAX_ANALYZER,"Invalid logic expression");
    }
    else if(l.size() >= 5)
    {
        //去掉括号
        l.erase(l.begin());
        l.pop_back();
        // l.erase(l.end());
        //分析关系表达式
        result_left = this->relation(tmg,sem,l);
    }
    else 
        errorHandle(SYNTAX_ANALYZER,"Invalid logic expression");


    //处理右边
    Token result_right;
    if(r.size()==0)
        errorHandle(SYNTAX_ANALYZER,"Invalid logic expression");
    else if(r.size()==1)
    {
        t = r[0];
        if(t.type==KEYWORD &&(tmg.at(t).name=="true" || tmg.at(t).name=="false"))
        {
            result_right = t;
        }
        else if(t.type==IDENTIFIER && tmg.at(t).type==BOOL)
        {
            result_right = t;
        }
        else 
            errorHandle(SYNTAX_ANALYZER,"Invalid logic expression");
    }
    else if(r.size() >= 5)
    {
        //去掉括号
        r.erase(r.begin());
        r.pop_back();
        //分析关系表达式
        result_right = this->relation(tmg,sem,r);
    }
    else 
        errorHandle(SYNTAX_ANALYZER,"Invalid logic expression");


    //左右分析均正确，执行逻辑表达式的语义动作
    Token result = sem.Logic(tmg,result_left,result_right,logic_symbol);
    return result;
}

bool SyntaxAnalyzer::isLogic(TableManager& tmg,vector<Token>& tokens)//判断一个表达式是不是逻辑表达式
{
    for(auto i: tokens)
    {
        if(i.type==KEYWORD && (tmg.at(i).name=="and"||tmg.at(i).name=="or"||tmg.at(i).name=="not"))
            return true;
    }
    return false;
}


void SyntaxAnalyzer::ifStatement(Token& t,TableManager& tmg,SemanticAnalyzer& sem)//if-else语句分析
{
    // 分析逻辑表达式
    vector<Token> tokens;
    t = this->fetchToken();
    while(! (t.type==KEYWORD && tmg.at(t).name=="then"))
    {
        tokens.push_back(t);
        t = this->fetchToken();
    }
    //去掉括号
    tokens.erase(tokens.begin());
    tokens.pop_back();
    Token result = this->logic(tmg,sem,tokens);
    //生成if四元式
    sem.If(result,tmg);

    /* 分析then后面的内容 */  
    t = this->fetchToken();
    if(!(t.type==KEYWORD && tmg.at(t).name=="begin"))
        errorHandle(SYNTAX_ANALYZER,"Invalid if-else statement");     
    t = this->fetchToken();
    while(!(t.type==KEYWORD && tmg.at(t).name=="end"))
    {
        this->assignStatement(t,tmg,sem);
    }
    //生成else四元式
    sem.El();
    /* 分析else后边的内容 */
    t = this->fetchToken();
    if(!(t.type==KEYWORD && tmg.at(t).name=="else"))
        errorHandle(SYNTAX_ANALYZER,"Invalid if-else statement");    
    t = this->fetchToken();
    if(!(t.type==KEYWORD && tmg.at(t).name=="begin"))
        errorHandle(SYNTAX_ANALYZER,"Invalid if-else statement");     
    t = this->fetchToken();
    while(!(t.type==KEYWORD && tmg.at(t).name=="end"))
    {
        this->assignStatement(t,tmg,sem);
    }
    t = this->fetchToken();
   
    //生成ifend四元式
    sem.Ie();
}

void SyntaxAnalyzer::whileStatement(Token& t,TableManager& tmg,SemanticAnalyzer& sem)//while-do语句分析
{
    // 生成while四元式
    sem.Wh();

    //分析逻辑表达式
    vector<Token> tokens;
    t = this->fetchToken();
    while(!(t.type==KEYWORD && tmg.at(t).name=="do"))
    {
        tokens.push_back(t);
        t = this->fetchToken();
    }
    //去掉括号
    tokens.erase(tokens.begin());
    tokens.pop_back();
    Token result = this->logic(tmg,sem,tokens);

    // 生成do四元式
    sem.Do(result,tmg);


    /* 分析do后面的内容 */
    t = this->fetchToken();
    if(!(t.type==KEYWORD && tmg.at(t).name=="begin"))
        errorHandle(SYNTAX_ANALYZER,"Invalid while-do statement");
    t = this->fetchToken();
    while(!(t.type==KEYWORD && tmg.at(t).name=="end"))
    {
        this->assignStatement(t,tmg,sem);
    }
    t = this->fetchToken();

    //生成while-end四元式
    sem.We();
}


void SyntaxAnalyzer::mainProgram(TableManager& tmg,SemanticAnalyzer& sem)//主程序的语法分析，递归调用其他语法分析函数
{   
    cout<<"------------------Syntax Analyze Result-----------------------"<<endl;
    cout<<"--------------------------------------------------------------"<<endl;

    Token t; 
    /* 分析主程序定义部分 */
    t = this->fetchToken();
    if(!(t.type==KEYWORD && tmg.at(t).name=="program"))
        errorHandle(SYNTAX_ANALYZER,"Invalid main program definition");
    t = this->fetchToken();
    if(t.type != IDENTIFIER)
        errorHandle(SYNTAX_ANALYZER,"Invalid main program definition");
    this->nameInsert(tmg.at(t).name);   //标识符名称加入集合
    sem.MainProgram(t.idx,tmg);             //语法分析成功，执行语义动作
    t = this->fetchToken();


    /* 分析常量定义部分 */
    if(!(t.type==KEYWORD && tmg.at(t).name=="const"))
        errorHandle(SYNTAX_ANALYZER,"Missing constant definition block");
    t = this->fetchToken();
    if(!(t.type==KEYWORD && tmg.at(t).name=="begin"))
        errorHandle(SYNTAX_ANALYZER,"Missing 'begin' in constant definition");
    t = this->fetchToken();
    while (tmg.at(t).name != "end")
    {   
        TypeCode code; //常量类型
        Index idx_main;    //常量在主表中的索引
        Index idx_const;    //常量对应的常数在常数表中的索引

        //识别类型：char,string,integer,real
        if(t.type==KEYWORD)
        {
            if(tmg.at(t).name=="integer") code = INTEGER;
            else if(tmg.at(t).name=="real") code = REAL;
            else if(tmg.at(t).name=="char") code = CHAR;
            else if(tmg.at(t).name=="bool") code = BOOL;
            else errorHandle(SYNTAX_ANALYZER,"Unsupported constant type in constant definition");
        }
        else
            errorHandle(SYNTAX_ANALYZER,"Invalid constant definition");
        t = this->fetchToken();
        
        //识别标识符            
        if(t.type==IDENTIFIER)
        {
            if(this->nameInsert(tmg.at(t).name) == false)
                errorHandle(SYNTAX_ANALYZER,"Indentifier redefinition error");//标识符重定义
            idx_main = t.idx;
        }
        else
            errorHandle(SYNTAX_ANALYZER,"Invalid constant definition");
        t = this->fetchToken();

        //识别 :=
        if(!(t.type==DELIMITER && tmg.at(t).name==":="))
            errorHandle(SYNTAX_ANALYZER,"Invalid constant definition");
        t = this->fetchToken();

        //识别常数
        if(t.type==CONSTANT)
        {
            //检查常数类型是否和定义的类型一致
            if(tmg.tab_constant.ele.at(t.idx).type_code != code)
                errorHandle(SYNTAX_ANALYZER,"Constant's data type doesn't match it's definition");
            else
                idx_const = t.idx;
        }
        else
            errorHandle(SYNTAX_ANALYZER,"Invalid constant definition");
        t = this->fetchToken();

        //识别 ;
        if(!(t.type==DELIMITER && tmg.at(t).name==";"))
            errorHandle(SYNTAX_ANALYZER,"Constant definition missing ';' ");
        t = this->fetchToken();

        //语法分析成功，执行语义动作
        sem.Const(tmg,idx_main,idx_const,code);
    }
    t = this->fetchToken();
    cout<<"const definition analysis successfully!"<<endl;


    /* 分析变量定义部分 */
    if(!(t.type==KEYWORD && tmg.at(t).name=="var"))
        errorHandle(SYNTAX_ANALYZER,"Missing variable definition block");
    t = this->fetchToken();
    if(!(t.type==KEYWORD && tmg.at(t).name=="begin"))
        errorHandle(SYNTAX_ANALYZER,"Missing 'begin' in variable definition");
    t = this->fetchToken();
    while (tmg.at(t).name != "end")
    {   
        TypeCode code;      //变量类型
        Index idx_main;    //变量在主表中的索引

        //识别类型：char,string,integer,real,bool
        if(t.type==KEYWORD)
        {
            if(tmg.at(t).name=="integer") code = INTEGER;
            else if(tmg.at(t).name=="real") code = REAL;
            else if(tmg.at(t).name=="char") code = CHAR;
            else if(tmg.at(t).name=="bool") code = BOOL;
            else errorHandle(SYNTAX_ANALYZER,"Unsupported variable type in variable definition");
        }
        else
            errorHandle(SYNTAX_ANALYZER,"Invalid variable definition");
        t = this->fetchToken();
        
        //识别标识符            
        if(t.type==IDENTIFIER)
        {
            if(this->nameInsert(tmg.at(t).name) == false)
                errorHandle(SYNTAX_ANALYZER,"Indentifier redefinition error");//标识符重定义
            idx_main = t.idx;
        }
        else
            errorHandle(SYNTAX_ANALYZER,"Invalid variable definition");
        t = this->fetchToken();

        //识别 ;
        if(!(t.type==DELIMITER && tmg.at(t).name==";"))
            errorHandle(SYNTAX_ANALYZER,"Variable definition missing ';' ");
        t = this->fetchToken();

        //语法分析成功，执行语义动作：填符号表
        sem.Var(tmg,code,idx_main);
    }
    t = this->fetchToken();
    cout<<"variable definition analysis successfully!"<<endl;


    /* 程序语句分析 */
    if(!(t.type==KEYWORD && tmg.at(t).name=="begin"))
        errorHandle(SYNTAX_ANALYZER,"Missing 'begin' in main body");
    t = this->fetchToken();  
    while(this->isEnd() == false)//没到最后一个Token时继续分析
    {
        // 识别赋值语句和算术表达式(算术表达式必须有赋值)
        if(t.type == IDENTIFIER)
            this->assignStatement(t,tmg,sem);

        // 识别if-else
        else if(t.type==KEYWORD && tmg.at(t).name=="if")
            this->ifStatement(t,tmg,sem);

        // 识别while-do
        else if(t.type==KEYWORD && tmg.at(t).name=="while")
            this->whileStatement(t,tmg,sem);

        // 出错处理
        else
            errorHandle(SYNTAX_ANALYZER,"Unrecognized statement in main body");

    }

    if(!(t.type==KEYWORD && tmg.at(t).name=="end"))//最后一个Token一定是end
        errorHandle(SYNTAX_ANALYZER,"Missing 'end' in main body");   

    cout<<"--------------------------------------------------------------"<<endl;
}
