#ifndef __SYNTAX_H__
#define __SYNTAX_H__

#include<set>
#include<string>
#include"define.h"
#include"lexical.h"
#include"semantic.h"

using namespace std;

/* 语法分析器 */
class SyntaxAnalyzer
{
public:
    SyntaxAnalyzer();

    /* 不同语句的语法分析，语法分析过程调用词法分析器，若语法分析正确则生成四元式 */
    void mainProgram(TableManager& tmg,SemanticAnalyzer& sem); //主程序分析

    void assignStatement(Token& t,TableManager& tmg,SemanticAnalyzer& sem); //赋值语句分析
    Token expression(TableManager& tmg,SemanticAnalyzer& sem,vector<Token>& right);  //算术表达式分析
    Token relation(TableManager& tmg,SemanticAnalyzer& sem,vector<Token>& tokens);   //关系表达式分析
    bool isRelation(TableManager& tmg,vector<Token>& tokens);    //判断一个表达式是不是关系表达式
    Token logic(TableManager& tmg,SemanticAnalyzer& sem,vector<Token>& tokens);      //简单逻辑表达式
    bool isLogic(TableManager& tmg,vector<Token>& tokens);   //判断一个表达式是不是逻辑表达式

    void ifStatement(Token& t,TableManager& tmg,SemanticAnalyzer& sem); //if-else语句分析
    void whileStatement(Token& t,TableManager& tmg,SemanticAnalyzer& sem);  //while-do语句分析

    void getTokens(LexicalAnalyzer& lex,TableManager& tmg);//调用词法分析器，将源代码转换为Token序列
    Token fetchToken(); //每次获取一个Token
    void printTokens(TableManager& tmg); //打印所有Token信息
    bool isEnd();   //当前Token是否是最后一个

    bool nameInsert(string name);   //若标识符已经存在，则插入失败，返回false，否则返回true

    /* 算术表达式的LL1分析方法 */
    bool E_LL1(TableManager& tmg,vector<Token>& tokens);
    bool E_lookLL1table(TableManager& tmg,string X,Token& token,vector<string>& result);

private:
    vector<Token> tokenlist;    //保存词法分析结果
    Index pos;  //当前正在识别的token在tokenlist中的位置
    set<string> names;  //所有标识符的名字集合，用于检查标识符是否有重定义
};

#endif
