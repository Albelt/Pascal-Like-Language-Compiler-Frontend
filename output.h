#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include"define.h"
#include"semantic.h"

void outIdentifier(TableManager& tmg,string file_name);    //将常量、用户变量、中间变量输出到文件
void outConstant(TableManager& tmg,string file_name);      //将常数输出到文件
void outQuaternary(TableManager& tmg,string file_name,SemanticAnalyzer& sem);  //将四元式输出到文件


string getName(char *file_path);    

#endif