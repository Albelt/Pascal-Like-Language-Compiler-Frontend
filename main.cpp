#include<iostream>
#include<sstream>
#include<string>
#include<stdio.h>
#include<stdlib.h>
#include"define.h"
#include"lexical.h"
#include"syntax.h"
#include"semantic.h"
#include"output.h"

using namespace std;

int main()
{
    cout<<"****************************|------------------------------------------|********************"<<endl;
    cout<<"****************************| A Pascal-Like Language Compiler Frontend |********************"<<endl;
    cout<<"****************************|------------------------------------------|********************"<<endl;

    char source_file[MIDLENGTH];
    cout<<"Input source file path: ";
    gets(source_file);

    // char source_file[MIDLENGTH] = "src/test.pll";


    cout<<"\nSystem initialization..."<<endl;
    TableManager tmg("sys/keyword.table","sys/delimiter.table");    //初始化符号表
    LexicalAnalyzer lex(source_file);    //初始化词法分析器
    SyntaxAnalyzer syn; //初始化语法分析器
    SemanticAnalyzer sem; //初始化语义分析器

    cout<<"\nStart compiling..."<<endl;
    syn.getTokens(lex,tmg); //进行词法分析
    syn.printTokens(tmg);  //打印Tokens
    syn.mainProgram(tmg,sem); //进行语法分析和语义分析
    
    // 将标识符表、常数表和四元式输出到文件
    string file_name = getName(source_file);
    outIdentifier(tmg,file_name);
    outConstant(tmg,file_name);
    outQuaternary(tmg,file_name,sem);

    cout<<"\nCompilation finished successfully!"<<endl;
    cout<<"****************************|------------------------------------------|********************"<<endl;
    cout<<"****************************|^^^^^^^^^^^^^^^^^ NEU CSE ^^^^^^^^^^^^^^^^|********************"<<endl;
    cout<<"****************************|------------------------------------------|********************"<<endl;
    cout<<"Press 'Enter' to exit"<<endl;
    getchar();
    return 0;
}
