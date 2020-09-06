#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string>
#include<sstream>
#include"define.h"
#include"lexical.h"
using namespace std;

bool LexicalAnalyzer::fileEnd()
{
    return (this->buffer[this->pos] == 0);
}


LexicalAnalyzer::LexicalAnalyzer(const char* source_file)//读入源文件并做相关初始化
{
    //读取源文件到缓冲区
    FILE *fp = NULL;
    if((fp = fopen(source_file,"r")) == NULL)
    {
        char buf[MINLENGTH] = "";
        sprintf(buf,"Can't open source file %s, please check the file path",source_file);
        errorHandle(LEXICAL_ANALYZER,buf);
    }
    char tmp[MAXLENGTH];
    while(fgets(tmp,MAXLENGTH,fp) != NULL)
    {
        strcat(this->buffer,tmp);
    }
    fclose(fp);

    //pos设置初值
    this->pos = 0;
}

bool LexicalAnalyzer::oneToken(TableManager& tmg,Token& token)//获取下一个Token,bool=false表示已经没有Token可以获取了
{
    char c;         //当前字符
    string tmp;    //临时字符串
    int idx;       //临时int，用于接受查表返回值

    c = this->buffer[this->pos];
    while(c)
    {
        tmp.clear();

        /* 过滤掉空格、回车、换行 */
        if(c==32 || c==9 || c==10 || c==13)
        {
            c = this->buffer[++this->pos];
            continue;
        }

        /* 识别关键字或标识符 */
        else if((c>='A'&&c<='Z') || (c>='a'&&c<='z'))
        {
            tmp.push_back(c);
            c = this->buffer[++this->pos];
            while((c>='A'&&c<='Z') || (c>='a'&&c<='z') ||(c>='0'&&c<='9'))
            {
                tmp.push_back(c);
                c = this->buffer[++this->pos];
            }
            if((idx=tmg.seek(tmp,KEYWORD)) != -1)//是关键字
            {
                token.type = KEYWORD;
                token.idx = idx;
                return true;
            }
            if((idx=tmg.seek(tmp,IDENTIFIER)) != -1)//是标识符且在表中存在
            {
                token.type = IDENTIFIER;
                token.idx = idx;
                return true;
            }
            //是标识符但是在表中不存在
            idx = tmg.insert(tmp,IDENTIFIER);
            token.type = IDENTIFIER;
            token.idx = idx;
            return true;
        }

        /* 识别INTEGER或REAL型常数 */
        else if(c>='0' && c<='9')
        {
            tmp.push_back(c);
            c = this->buffer[++this->pos];
            while(c>='0' && c<='9')//识别整数部分
            {
                tmp.push_back(c);
                c = this->buffer[++this->pos];
            }
            //常量为REAL型
            if(c == '.')
            {
                tmp.push_back(c);//识别小数点
                c = this->buffer[++this->pos];
                //识别小数部分
                string decimal;
                while(c>='0' && c<='9')
                {
                    decimal.push_back(c);
                    c = this->buffer[++this->pos];
                }
                if(decimal.size() == 0)//小数部分出错
                    errorHandle(LEXICAL_ANALYZER,"Invilad real constantin source file");
                tmp.append(decimal);//整数部分、小数点、小数部分合并
                if((idx=tmg.seek(tmp,CONSTANT,REAL)) != -1)//在常数表中存在
                {
                    token.type = CONSTANT;
                    token.idx = idx;
                    return true;
                }
                //在常数表中不存在
                idx = tmg.insert(tmp,CONSTANT,REAL);
                token.type = CONSTANT;
                token.idx = idx;
                return true;
            }
            //常量为INTEGER型
            if((idx=tmg.seek(tmp,CONSTANT,INTEGER)) != -1)//在常数表中存在
            {
                token.type = CONSTANT;
                token.idx = idx;
                return true;
            }
            idx = tmg.insert(tmp,CONSTANT,INTEGER);
            token.type = CONSTANT;
            token.idx = idx;
            return true;
        }

        /* 识别CHAR或STRING型常量，不支持转义字符 */
        else if(c=='\'' || c=='"')
        {
            //识别一个字符
            if(c=='\'')
            {
                if(this->buffer[this->pos + 2] != '\'')//错误
                {
                    char buf[MINLENGTH] = "";
                    sprintf(buf,"Invalid character constant: %c%c%c",this->buffer[this->pos],this->buffer[this->pos+1],this->buffer[this->pos+2]);
                    errorHandle(LEXICAL_ANALYZER,buf);
                }
                tmp.push_back(this->buffer[this->pos+1]);
                this->pos += 3;
                c = this->buffer[this->pos];
                if((idx=tmg.seek(tmp,CONSTANT,CHAR)) != -1)//在常数表中
                {
                    token.type = CONSTANT;
                    token.idx = idx;
                    return true;
                }
                //不在常数表中
                idx = tmg.insert(tmp,CONSTANT,CHAR);
                token.type = CONSTANT;
                token.idx = idx;
                return true;
            }

            //识别字符串
            //找到字符串结尾位置
            Index end = this->pos + 1;
            while(this->buffer[end]!='"' && this->buffer[end]!='\n')
                end++;
            //如果一行结束都没有找到双引号，就报错
            if(this->buffer[end] != '"')
                errorHandle(LEXICAL_ANALYZER,"Invalid string constant in source file");
            //提取字符串
            this->pos++;
            while(this->pos != end)
                tmp.push_back(this->buffer[this->pos++]);
            c = this->buffer[++this->pos];
            if((idx=tmg.seek(tmp,CONSTANT,STRING)) != -1)//字符串在常数表中
            {
                token.type = CONSTANT;
                token.idx = idx;
                return true;
            }
            //字符串不在常数表中
            idx = tmg.insert(tmp,CONSTANT,STRING);
            token.type = CONSTANT;
            token.idx = idx;
            return true;
        }

        /* 从'#' 到 '\n' 都是注释内容，跳过 */
        else if(c == '#')
        {
            while(c != '\n')
                c = this->buffer[++this->pos];
            c = this->buffer[++this->pos];    
            continue;
        }
        /* 识别界符和注释符号 */
        else
        {
            token.type = DELIMITER;
            switch (c)
            {
            case ',':
                token.idx = tmg.seek(",",DELIMITER);
                c = this->buffer[++this->pos];
                return true;
            case ';':
                token.idx = tmg.seek(";",DELIMITER);
                c = this->buffer[++this->pos];
                return true;
            case '+':
                token.idx = tmg.seek("+",DELIMITER);
                c = this->buffer[++this->pos];
                return true;
            case '-':
                token.idx = tmg.seek("-",DELIMITER);
                c = this->buffer[++this->pos];
                return true;
            case '*':
                token.idx = tmg.seek("*",DELIMITER);
                c = this->buffer[++this->pos];
                return true;
            case '/':
                token.idx = tmg.seek("/",DELIMITER);
                c = this->buffer[++this->pos];
                return true;
            case '(':
                token.idx = tmg.seek("(",DELIMITER);
                c = this->buffer[++this->pos];
                return true;
            case ')':
                token.idx = tmg.seek(")",DELIMITER);
                c = this->buffer[++this->pos];
                return true;
            case '[':
                token.idx = tmg.seek("[",DELIMITER);
                c = this->buffer[++this->pos];
                return true;
            case ']':
                token.idx = tmg.seek("]",DELIMITER);
                c = this->buffer[++this->pos];
                return true;
            case '=':
                token.idx = tmg.seek("=",DELIMITER);
                c = this->buffer[++this->pos];
                return true;
            case ':':
                c = this->buffer[++this->pos];
                if(c == '=')
                {
                    token.idx = tmg.seek(":=",DELIMITER);
                    c = this->buffer[++this->pos];
                }
                else
                    token.idx = tmg.seek(":",DELIMITER);
                return true;
            case '>':
                c = this->buffer[++this->pos];
                if(c == '=')
                {
                    token.idx = tmg.seek(">=",DELIMITER);
                    c = this->buffer[++this->pos];
                }
                else
                    token.idx = tmg.seek(">",DELIMITER);
                return true;
            case '<':
                c = this->buffer[++this->pos];
                if(c == '=')
                {
                    token.idx = tmg.seek("<=",DELIMITER);
                    c = this->buffer[++this->pos];
                }
                if(c == '>')
                {
                    token.idx = tmg.seek("<>",DELIMITER);
                    c = this->buffer[++this->pos];
                }
                else
                    token.idx = tmg.seek("<",DELIMITER);
                return true;
            default:
                char buf[MINLENGTH] = "";
                sprintf(buf,"Unrecognizable character %c",c);
                errorHandle(LEXICAL_ANALYZER,buf);
           }
        }

    }

    return false;
}