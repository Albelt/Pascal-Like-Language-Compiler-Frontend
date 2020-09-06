#ifndef __DEFINE_H__
#define __DEFINE_H__

#include<string>
#include<vector>
using namespace std;


/* ************************************************* */
/* *******************全局符号定义*****************  */
/* ************************************************* */

/* 索引类型,从0开始 */
typedef unsigned int Index;

/* 长度(length) */
#define MAXLENGTH   1024
#define MIDLENGTH   256
#define MINLENGTH   64

/* Token种类码(用于Token) */
typedef unsigned int TokenCode;
#define KEYWORD     0
#define DELIMITER   1
#define IDENTIFIER  2
#define CONSTANT    3

/* 标识符种类码(用于填符号表总表) */
typedef unsigned int IdentifierCode;
#define N 0   //无类型(初始化)
#define P 1   //函数(过程)
#define C 2   //常量
#define V 3   //普通局部变量
#define M 4   //中间变量
#define T 5   //结构体
#define D 6   //域名(结构体单元名字)
#define VN 7  //换名形参(传地址)
#define VF 8  //赋值形参(传值)

/* 类型码(用于填类型表) */
typedef unsigned int TypeCode;
#define INTEGER 0
#define REAL    1
#define CHAR    2
#define STRING  3
#define BOOL    4
#define ARRAY   5
#define STRUCT  6

/* 编译器各部分 */
typedef unsigned int CompilerPart;
#define LEXICAL_ANALYZER  0
#define SYNTAX_ANALYZER   1
#define SEMANTIC_ANALYZER 2
#define TABLE_MANAGER     3

/* 运行时刻内存分配表 */
typedef unsigned int MemType;
#define M_PROGRAM   0
#define M_CONSTANT  1
#define M_VARIBLE   2
#define M_MIDVAR    3



/* ************************************************* */
/* ****************各种结构体定义******************  */
/* ************************************************* */

/* Token定义 */
typedef struct
{
    TokenCode type;          //token种类
    Index idx;               //对KEYWORD、DELIMITER、IDENTIFIER而言是在MainTable中的索引，对CONSTANT而言是在ConstantTable中的索引
} Token;

/* 类型表表项 */
typedef struct
{
    TypeCode type_code;         //类型码
    void *type_point;          //类型指针，数组指向数组表，结构体指向结构体表，基本类型此项为空

} TypeTableItem;

/* 数组信息表表项 */
typedef struct
{
    Index low;  //下界
    Index up;   //上界
    Index ctp;  //该维数组的元素类型(在类型表中的索引)
    Index clen; //该维数组的元素长度(以字节为单位)
} ArrayTableItem;

/* 结构体信息表表项 */
typedef struct
{
    string name;         //结构体内单元的名字
    unsigned int off;   //该单元与结构体头部地址的偏移量
    Index tp;            //该单元的类型(在类型表中的索引)
} StructTableItem;

/* 常数表表项 */
typedef struct
{
    TypeCode type_code;     //常数类型，支持INTEGER,REAL,CHAR,STRING
    string value;           //不管是什么类型的常数，统一保存为字符串，方便操作
} ConstantTableItem;

/* 长度表表项 */
// unsigned int型

/* 符号表总表表项 */
typedef struct
{
    string name;            //名字，对关键字、界符而言就是自身，对标识符而言是名字，对常量而言此项为空
    TypeCode type;          //类型，在类型表中的索引，对关键字、界符而言无意义
    IdentifierCode cat;     //标识符种类码，对关键字、界符而言无意义
    Index address;          //函数在函数表中的索引，或常数在常数表中的索引，或结构体和数组在长度表中的索引，对变量、关键字、界符而言无意义
} MainTableItem;


/* 运行时刻内存分配表表项 */
typedef struct 
{
    MemType stype;          //符号类型
    unsigned int size;      //所占空间大小
    unsigned int offset;    //偏移量
    unsigned int idx;       //在常数表中的索引(仅对常量有效)
} MemTableItem;


/* ************************************************* */
/* ***************各种表的定义*********************  */
/* ************************************************* */

/* 类型表 */
class TypeTable
{
public:
    TypeTable();    //初始化类型表，填上基本类型
    vector<TypeTableItem> ele;     //类型表
};

/* 数组信息表 */
class ArrayTable
{
public:
    ArrayTable();
private:
    vector<ArrayTableItem> ele;     //数组表
};

/* 结构体信息表 */
class StructTable
{
public:
    StructTable();
private:
    vector<StructTableItem> ele;    //结构体表
};

/* 常数表：integer,real,char,string型常数均以字符串的形式进行存储和查找 */
class ConstantTable
{
public:
    ConstantTable();
    int seekConstant(const string& value,TypeCode code);       //查表
    int insertConstant(const string& value,TypeCode code);     //添加一项并返回索引
//private:
    vector<ConstantTableItem> ele;    //常数表
};

/* 长度表 */
class LengthTable
{
public:
    LengthTable();
private:
    vector<unsigned int> ele;   //长度表
};


/* 符号表总表(关键字、界符、标识符，不包含常量) */
class MainTable
{
public:
    MainTable();
    void LoadFile(const char* keyword_table,const char* delimiter_table);   //加载关键字和界符

    vector<MainTableItem> ele;      //主表
    Index end1,end2,end3,end4;           //关键字:[0,end1),界符:[end1,end2),标识符:[end2,end3),中间变量:[end3,end4)
    Index mid;      //中间变量的索引

    /* 查表函数，查到返回索引，查不到返回-1 */
    int seekKeyword(const string& name);      //查关键字
    int seekDelimiter(const string& name);    //查界符
    int seekIdentifier(const string& name);   //查标识符
    

    /* 给词法分析器提供的接口 */
    Index insertIdentifier(const string& name);   //向表中加入一个新的标识符并返回索引

};


/* 运行时刻内存分配表表项 */
class MemTable
{
public:
    vector<MemTableItem> ele;
};


/* 所有表的管理器 */
class TableManager
{
public:
    TableManager(const char* keyword_table,const char* delimiter_table); //初始化符号总表
    /* 给词法分析器提供的接口 */
    int seek(const string& item,TokenCode code1,TypeCode code2=0);          //Token查表,TypeCode只对常数有效
    int insert(const string& item,TokenCode code1,TypeCode code2=0);        //在表中插入新的Token，返回插入位置,TypeCode只对常数有效

    /* 给语法分析器提供的接口 */
    MainTableItem at(Token token);    //给定主表索引，返回表项

    /* 给语义分析器提供的接口 */
    Token newT(TypeCode type);  //申请一个中间变量并且在主表中注册
    string getId(Token& token); //返回类型和在表中的索引,如I(10)
    TypeCode getType(Token& token); //返回变量、常量或常数的数据类型

    /* 给文件输出提供的接口 */
    unsigned int getSize(MainTableItem& item); //给定item,根据数据类型返回所占空间大小

// private:
    TypeTable tab_type;
    ArrayTable tab_array;
    StructTable tab_struct;
    ConstantTable tab_constant;
    LengthTable tab_length;
    MainTable tab_main;
    MemTable tab_mem;
};


void errorHandle(CompilerPart c,const char *info);  //出错处理函数

#endif
