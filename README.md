### 源文件一般格式
源代码必须是UTF-8编码，换行符为LF
``` pascal
program example 	# 主程序定义
const begin 	    # 常量定义，定义时必需赋初值
	real pi := 3.14;	# 圆周率
end
var begin			# 变量定义，定义时不能赋初值	
	integer a;
	integer b;
	char c;
	bool d;
	bool e;
	real r;		# 圆的半径
	real area;  # 圆的面积
end
begin	# 程序开始		
	a := 1;						#赋值语句
	b := a;						#赋值语句
	area := pi*r*r;				#算数表达式
	d := area*3.0 > 10.0+pi; 	#关系表达式,关系运算符左右都是算数表达式
	#简单逻辑表达式,只支持一个'and'或'or'，暂不支持'not'
	e := (area > 10.4) and (a = b);     
	d := e or true;

	# if-else语句，不支持嵌套
	if((area > 10.4) and (a = b)) then
	begin
		c := 'Y';
		d := d and (a <> b);
	end
	else begin
		c := 'N';
	end

	# while-do语句，不支持嵌套
	while(d = false) do
	begin
		r := pi+(3.14-r*area);
		a := a + b*a;
	end

end   # 程序结束

```

### 已经实现的特性
- 支持常量定义、变量定义、赋值语句、算术表达式、关系表达式、逻辑表达式和if-else语句、while-do语句

- 主程序的执行代码部分必须用begin和end包裹，if-else、while-do必须用begin和end包裹

- 支持单行注释，以 '#' 开始

- 常量支持integer,real,char,bool类型

- 变量支持integer,real,char,bool类型
