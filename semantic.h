#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

#include<string>
#include<vector>
using namespace std;


/* 四元式定义 */
typedef struct
{
    string op;      //算符
    string src1;    //源操作数1
    string src2;    //源操作数2
    string target;  //结果对象
} Quaternary;


/* 语义分析器 */
class SemanticAnalyzer
{
public:
    SemanticAnalyzer();
    vector<Quaternary> quatlist;    //生成的四元式序列

    /* 语义动作：生成四元式、填表 */
    void MainProgram(Index idx,TableManager& tmg);  
    void Const(TableManager& tmg,Index idx_main,Index idx_const,TypeCode code);  
    void Var(TableManager& tmg,TypeCode code,Index idx_main);  
    void Assign(Token& left,Token& right,TableManager& tmg);  //生成赋值四元式
    
    Token Expression(TableManager& tmg,vector<Token>& tokens);   //给定算术表达式tokens，返回最终结果单元的Token
    bool OpPriority(string op1,string op2); //若op1 > op2，返回TRUE，否则返回FALSE

    Token Relation(TableManager& tmg,Token& token_l,Token& token_r,string& rea_symbol);  //生成关系表达式的四元式

    Token Logic(TableManager& tmg,Token& token_l,Token& token_r,string& logic_symbol);//生成逻辑简单逻辑表达式的四元式

    void If(Token& token,TableManager& tmg);  //生成if四元式
    void El();  //生成else四元式
    void Ie();  //生成ifend四元式
    
    void Wh();  //生成while四元式
    void Do(Token& token,TableManager& tmg);  //生成do四元式
    void We();  //生成while-end四元式

    bool typeMatch(Token& a,Token& b,TableManager& tmg);      //检查两个Token的数据类型是否相同

};

#endif
