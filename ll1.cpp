#include<iostream>
#include<vector>
#include<string>
#include<stack>
#include"define.h"
#include"syntax.h"


/* 算术表达式的LL1分析方法 */
bool SyntaxAnalyzer::E_LL1(TableManager& tmg,vector<Token>& tokens)
{
    unsigned int cur = 0;               //当前分析的Token
    unsigned int len = tokens.size();   //token串的长度
    stack<string> symbol;               //文法符号栈
    string X;                           //当前栈顶的文法符号
    vector<string> result;              //查找LL1分析表得到的生成式
    vector<TypeCode> operand_type;      //保存识别的操作数的类型，用于检查类型是否一致

    // 开始符号压栈
    X = "E";
    symbol.push(X);
    
    while(cur!=len || symbol.empty()==false)
    {
        result.clear();
        X = symbol.top();

        // 栈顶符号是非终结符
        if(X=="E" || X=="E1" || X=="T" || X=="T1" || X=="F")
        {
            Token token;
            if(cur<len)
                token = tokens.at(cur);
            else //当前Token为结束符，用100表示
                token.type = 100;
            if(this->E_lookLL1table(tmg,X,token,result))
            {
                symbol.pop();
                if(result.size() != 0)
                {
                    // 将产生式的文法符号压栈
                    for(auto i: result)
                        symbol.push(i);                    
                }
            }
            else // 查表出错
                return false;
        }

        // 栈顶符号是终结符
        else
        {
            symbol.pop();
            Token token;
            if(cur<len)
                token = tokens.at(cur);
            else 
                return false;
            
            if(X=="I")
            {
                if(token.type==IDENTIFIER && (tmg.at(token).type==INTEGER || tmg.at(token).type==REAL))
                {
                    operand_type.push_back(tmg.at(token).type);
                    cur++;
                }
                else if(token.type==CONSTANT && (tmg.tab_constant.ele.at(token.idx).type_code==INTEGER || tmg.tab_constant.ele.at(token.idx).type_code==REAL))
                {
                    operand_type.push_back(tmg.tab_constant.ele.at(token.idx).type_code);
                    cur++;                    
                }
                else 
                    return false;
            }
            else if(X=="W0")
            {
                if(token.type==DELIMITER && (tmg.at(token).name=="+" || tmg.at(token).name=="-"))
                {
                    cur++;
                }
                else 
                    return false;
            }
            else if(X=="W1")
            {
                if(token.type==DELIMITER && (tmg.at(token).name=="*" || tmg.at(token).name=="/"))
                {
                    cur++;
                }
                else 
                    return false;
            }
            else if(X=="(")
            {
                if(token.type==DELIMITER && tmg.at(token).name=="(")
                {
                    cur++;
                }
                else 
                    return false;
            }
            else if(X==")")
            {
                if(token.type==DELIMITER && tmg.at(token).name==")")
                {
                    cur++;
                }
                else 
                    return false;
            }
            else 
                return false;
        }
    }

    // 检查操作数类型是否一致
    for(int i=0;i<operand_type.size()-1;i++)
    {
        if(operand_type[i] != operand_type[i+1])
            return false;
    }

    // 分析正确，返回
    return true;
}
    
bool SyntaxAnalyzer::E_lookLL1table(TableManager& tmg,string X,Token& token,vector<string>& result)
{
    if(X=="E")
    {
        if(token.type==IDENTIFIER && (tmg.at(token).type==INTEGER || tmg.at(token).type==REAL))
        {//I
            result.push_back("E1");
            result.push_back("T");
            return true;
        }
        else if(token.type==CONSTANT && (tmg.tab_constant.ele.at(token.idx).type_code==INTEGER || tmg.tab_constant.ele.at(token.idx).type_code==REAL))
        {//I
            result.push_back("E1");
            result.push_back("T");
            return true;                  
        }
        else if(token.type==DELIMITER && tmg.at(token).name=="(")
        {
            result.push_back("E1");
            result.push_back("T");
            return true;            
        }
        else
            return false;
    }
    else if(X=="E1")
    {
        if(token.type==DELIMITER && (tmg.at(token).name=="+" || tmg.at(token).name=="-"))
        {//W0
            result.push_back("E1");
            result.push_back("T");
            result.push_back("W0");
            return true;
        }
        else if(token.type==DELIMITER && tmg.at(token).name==")")
        {// )
            return true;
        }
        else if(token.type==100)// #
            return true;
        else 
            return false;

    }
    else if(X=="T")
    {
        if(token.type==IDENTIFIER && (tmg.at(token).type==INTEGER || tmg.at(token).type==REAL))
        {//I
            result.push_back("T1");
            result.push_back("F");
            return true;
        }
        else if(token.type==CONSTANT && (tmg.tab_constant.ele.at(token.idx).type_code==INTEGER || tmg.tab_constant.ele.at(token.idx).type_code==REAL))
        {//I
            result.push_back("T1");
            result.push_back("F");
            return true;                  
        }        
        else if(token.type==DELIMITER && tmg.at(token).name=="(")
        {
            result.push_back("T1");
            result.push_back("F");
            return true;
        }
        else
            return false;

    }
    else if(X=="T1")
    {
        if(token.type==DELIMITER && (tmg.at(token).name=="+" || tmg.at(token).name=="-"))
        {// W0
            return true;
        }
        else if(token.type==DELIMITER && (tmg.at(token).name=="*" || tmg.at(token).name=="/"))
        {
            result.push_back("T1");
            result.push_back("F");
            result.push_back("W1");
            return true;
        }
        else if(token.type==DELIMITER && tmg.at(token).name==")")
        {// W1
            return true;
        }
        else if(token.type==100) // #
            return true;
        else 
            return false;
    }
    else if(X=="F")
    {
        if(token.type==IDENTIFIER && (tmg.at(token).type==INTEGER || tmg.at(token).type==REAL))
        {//I
            result.push_back("I");
            return true;
        }
        else if(token.type==CONSTANT && (tmg.tab_constant.ele.at(token.idx).type_code==INTEGER || tmg.tab_constant.ele.at(token.idx).type_code==REAL))
        {//I
            result.push_back("I");
            return true;                  
        } 
        else if(token.type==DELIMITER && tmg.at(token).name=="(")
        {
            result.push_back(")");
            result.push_back("E");
            result.push_back("(");
            return true;
        }
        else 
            return false;
    }
    else 
        return false;
}