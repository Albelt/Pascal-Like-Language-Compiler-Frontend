#include"output.h"


string getName(char *file_path)
{
    stringstream ss;
    ss<<file_path;
    string s;
    ss>>s;
    int i = s.find_last_of('/');
    int j = s.find('.');
    char *buffer = (char*)s.data();
    char name[MIDLENGTH];
    int k = 0;
    i++;
    for(;i<j;i++)
        name[k++] = buffer[i];
    name[k++] = '\0';
    s = name;
    return name;
}

void outIdentifier(TableManager& tmg,string file_name)//将常量、用户变量、中间变量输出到文件
{
    file_name = "out/" + file_name + ".identifier";
    char *file_path = (char*)file_name.data();
    ofstream out(file_path);
    MainTableItem item;
    unsigned int offset = 0;
    unsigned int size = 0;

    if(out.is_open())
    {
        int i = tmg.tab_main.end2;
        int j = tmg.tab_main.end4;
        for(;i<j;i++)
        {
            item = tmg.tab_main.ele.at(i);
            offset += size;
            out<<item.name<<" "<<offset<<" ";
            size = tmg.getSize(item);
            out<<size<<" ";
            if(item.cat==C)
                out<<item.address<<endl;
            else 
                out<<"_"<<endl;
        }
    }
    else
    {
        cout<<"Fail to save identifier!"<<endl;
        return;
    }

    out.close();
    cout<<"Identifer table was saved in "<<file_name<<endl;
}


void outConstant(TableManager& tmg,string file_name)//将常数输出到文件
{
    file_name = "out/" + file_name + ".constant";
    char *file_path = (char*)file_name.data();
    ofstream out(file_path);
    ConstantTableItem item;
    unsigned int i = 0;
    unsigned int j = tmg.tab_constant.ele.size();

    if(out.is_open())
    {
        for(;i<j;i++)
        {
            item = tmg.tab_constant.ele.at(i);
            switch(item.type_code)
            {
            case INTEGER:
                out<<"integer "<<4<<" ";
                break;
            case REAL:
                out<<"real "<<8<<" ";
                break;
            case CHAR:
                out<<"char "<<1<<" ";
                break;
            case BOOL:
                out<<"bool "<<1<<" ";
                break;
            default:
                out<<"error "<<0<<" ";
            }

            out<<item.value<<endl;
        }
    }
    else
    {
        cout<<"Fail to save constant!"<<endl;
        return;
    }

    out.close();
    cout<<"Constant table was saved in "<<file_name<<endl;
}


void outQuaternary(TableManager& tmg,string file_name,SemanticAnalyzer& sem) //将四元式输出到文件
{
    file_name = "out/" + file_name + ".quaternary";
    char *file_path = (char*)file_name.data();
    ofstream out(file_path);
    
    if(out.is_open())
    {
        for(auto i: sem.quatlist)
        {
            out<<i.op<<" "<<i.src1<<" "<<i.src2<<" "<<i.target<<endl;
        }
    }
    else
    {
        cout<<"Fail to save quaternary!"<<endl;
        return;
    }

    out.close();
    cout<<"Quaternary table was saved in "<<file_name<<endl;        
}