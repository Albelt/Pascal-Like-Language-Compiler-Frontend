#include<iostream>
#include<fstream>
#include<sstream>
#include<stdlib.h>
#include<string>
#include<vector>
#include"define.h"

using namespace std;

/* 类型表相关函数 */
TypeTable::TypeTable()//初始化类型表，填入基本类型
{
    TypeTableItem item;
    item.type_point = NULL;
    for(int i=0;i<=6;i++)
    {
        item.type_code = i;
        this->ele.push_back(item);
    }
}


/* 数组信息表相关函数 */
ArrayTable::ArrayTable()
{

}


/* 结构体信息表相关函数 */
StructTable::StructTable()
{

}


/* 常量表相关函数 */
ConstantTable::ConstantTable()
{

}

int ConstantTable::seekConstant(const string& value,TypeCode code)//查表
{
    Index size = this->ele.size();
    for(Index i=0;i<size;i++)
    {
        if(this->ele.at(i).type_code==code && this->ele.at(i).value == value)
            return i;
    }
    return -1;
}

int ConstantTable::insertConstant(const string& value,TypeCode code) //添加一项并返回索引
{
    ConstantTableItem item;
    item.value = value;
    item.type_code = code;
    this->ele.push_back(item);
    return this->ele.size() - 1;
}


/* 长度表相关函数 */
LengthTable::LengthTable()
{

}


/* 总表相关函数 */
MainTable::MainTable()
{
    this->end1 = this->end2 = this->end3 = this->end4 = 0;
    this->mid = 0;
}

void MainTable::LoadFile(const char* keyword_table,const char* delimiter_table)//主表加载关键字和界符
{
    char buffer[MIDLENGTH];
    MainTableItem item;
    //读keyword.table
    ifstream in(keyword_table);
    if(in.is_open() == false)
        errorHandle(TABLE_MANAGER,"Can't open keyword_table, please check check sys/keyword.table");
    while(in.eof() == false)
    {
        in.getline(buffer,256);
        item.name = buffer;
        this->ele.push_back(item);
        this->end1++;
    }
    this->end4 = this->end3 = this->end2 = this->end1;
    in.close();
    //读delimiter.table
    ifstream in2(delimiter_table);
    if(in2.is_open() == false)
        errorHandle(TABLE_MANAGER,"Can't open delimiter_table, please check sys/delimiter.table");
     while(in2.eof() == false)
    {
        in2.getline(buffer,256);
        item.name = buffer;
        this->ele.push_back(item);
        this->end2++;
    }
    this->end4 =  this->end3 = this->end2;
    in2.close();
}

int MainTable::seekKeyword(const string& name) //查关键字
{
    for(Index i=0;i<this->end1;i++)
    {
        if(this->ele.at(i).name == name)
            return i;
    }
    return -1;
}

int MainTable::seekDelimiter(const string& name)//查界符
{
    for(Index i=this->end1;i<this->end2;i++)
    {
        if(this->ele.at(i).name == name)
            return i;
    }
    return -1;
}

int MainTable::seekIdentifier(const string& name)//查标识符
{
    for(Index i=this->end2;i<this->end3;i++)
    {
        if(this->ele.at(i).name == name)
            return i;
    }
    return -1;
}

Index MainTable::insertIdentifier(const string& name)//添加标识符(给词法分析器提供的接口)
{
    MainTableItem item;
    item.name = name;
    //type,cat,address还不能确定
    item.cat = N;   //cat初始化为0
    this->ele.push_back(item);
    this->end3++;
    this->end4 = this->end3;
    return this->end3 - 1;
}


/* 表管理器相关函数 */
TableManager::TableManager(const char* keyword_table,const char* delimiter_table)
{
    this->tab_main.LoadFile(keyword_table,delimiter_table);
}

int TableManager:: seek(const string& item,TokenCode code1,TypeCode code2) //Token查表,TypeCode只对常数有效
{
    if(code1 == KEYWORD)
        return this->tab_main.seekKeyword(item);
    else if(code1 == DELIMITER)
        return this->tab_main.seekDelimiter(item);
    else if(code1 == IDENTIFIER)
        return this->tab_main.seekIdentifier(item);
    else if(code1 == CONSTANT)
        return this->tab_constant.seekConstant(item,code2);
    else
        errorHandle(TABLE_MANAGER,"Seek Error! Token Code is invalid!");
}

int TableManager::insert(const string& item,TokenCode code1,TypeCode code2)//在表中插入新的Token，返回插入位置,TypeCode只对常数有效
{
    if(code1 == IDENTIFIER)
        return this->tab_main.insertIdentifier(item);
    else if(code1 == CONSTANT)
        return this->tab_constant.insertConstant(item,code2);
    else
    {
        char buf[MINLENGTH] = "";
        sprintf(buf,"Insert Error, token code == %d is invalid",code1);
        errorHandle(TABLE_MANAGER,buf);
    }
}

MainTableItem TableManager::at(Token token) //给定主表索引，返回表项
{
    MainTableItem item;
    if(token.type != CONSTANT)
        item = this->tab_main.ele[token.idx];
    else
        item.name = this->tab_constant.ele[token.idx].value;
    return item;
}

Token TableManager::newT(TypeCode type)//申请一个中间变量并且在主表中注册
{
    // 申请中间变量
    Token token;
    token.type = IDENTIFIER;
    token.idx = this->tab_main.end4;

    // 在主表中注册
    MainTableItem item;
    stringstream tmp;
    tmp<<"T"<<this->tab_main.mid++;
    tmp>>item.name;
    item.cat = M;
    item.type = type;
    this->tab_main.ele.push_back(item);
    this->tab_main.end4++;

    return token;
}

string TableManager::getId(Token& token)//返回类型和在表中的索引
{
    stringstream tmp;
    string result;
    if(token.type==IDENTIFIER)//标识符
    {
        tmp<<"I("<<token.idx - this->tab_main.end2<<")";
        tmp>>result;
    }
    else//常数
    {
        tmp<<"C("<<token.idx<<")";
        tmp>>result;
    }
    return result;
}

TypeCode TableManager::getType(Token& token)//返回变量、常量或常数的数据类型
{
    if(token.type==CONSTANT)//常数
        return this->tab_constant.ele.at(token.idx).type_code;
    else//变量和常量
        return this->at(token).type;
}

unsigned int TableManager::getSize(MainTableItem& item)//给定item,根据数据类型返回所占空间大小
{
    if(item.cat!=C && item.cat!=V && item.cat!=M)
        return 0;

    switch(item.type)
    {
    case INTEGER:
        return 4;
    case REAL:
        return 8;
    case CHAR:
        return 1;
    case BOOL:
        return 1;
    default:
        return 0;
    }
}

/* 其他函数 */
void errorHandle(CompilerPart c,const char* info) //出错处理函数
{
    switch(c)
    {
    case LEXICAL_ANALYZER:
        cerr<<"[Lexical Analyzer]: "<<info<<" !"<<endl;
        break;
    case SYNTAX_ANALYZER:
        cerr<<"[Syntax Analyzer]: "<<info<<" !"<<endl;
        break;
    case SEMANTIC_ANALYZER:
        cerr<<"[Semantic Analyzer]: "<<info<<" !"<<endl;
        break;
    case TABLE_MANAGER:
        cerr<<"[Table Manager]: "<<info<<" !"<<endl;
        break;        
    default:
        break;
    }

    cerr<<"Press 'Enter' to exit..."<<endl;
    getchar();
    exit(1);
}
