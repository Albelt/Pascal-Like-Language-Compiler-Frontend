#ifndef __LEXICAL_H__
#define __LEXICAL_H__

class LexicalAnalyzer
{
public:
    LexicalAnalyzer(const char* source_file);    //读入源文件并做相关初始化
    bool oneToken(TableManager& tmg,Token& token);           //获取下一个Token,bool=false表示没有Token可以获取了
    bool fileEnd();                              //判断是否到达文件结尾
private:
    char buffer[MAXLENGTH*8] = "";              //源文件缓冲区
    Index pos;                                  //当前词法分析器扫描到的位置
};

#endif