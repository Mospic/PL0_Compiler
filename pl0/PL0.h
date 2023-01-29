#ifndef PL0_H
#define PL0_H





#include <stdio.h>

#define NRW        13     // number of reserved words
#define TXMAX      2000    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       18     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      2000    // size of code array

#define MAXSYM     43     // maximum number of symbols  

#define STACKSIZE  4000   // maximum storage

#define MAXDIM 10	      // maximum dimensions of array

enum symtype
{
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,		/*10*/
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_PERIOD,
	SYM_BECOMES,
    SYM_BEGIN,		/*20*/
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	SYM_CALL,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,	//以下为添加的symtype,共12个
	SYM_AND,		/*30*/
	SYM_OR,
	SYM_NOT,
	SYM_LSPAREN,
	SYM_RSPAREN,
	SYM_ARRAY,
	SYM_READ,
	SYM_WRITE,
	SYM_LBRACE, 
	SYM_RBRACE
};

enum idtype			//添加了ID_ARRAY的符号表类型
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE, ID_ARRAY
};

/*
 *opcode在原有8条指令的基础上，额外添加了7条：
 *JPNC:满足条件则跳转；
 *LDA :加载数组值到栈顶；
 *STA :存储数组值到偏移地址上；
 *RDA :读取数组指令；
 *WTA :打印数组指令；
 *READ:读取变量值；
 *WRITE:打印变量值。
 */
enum opcode			
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC, JPNC, LDA, STA, RDA, WTA, READ, WRITE
};

/*
 *oprcode在原有13条指令的基础上，额外添加了5条：
 *OPR_NOT:对栈顶值取非；
 *OPR_WEN:打印空格指令；
 *OPR_WRITE:打印栈顶常数指令；
 *OPR_ADDPLUS: 进行+=运算；
 *OPR_SUBPLUS: 进行-=运算。
 */
enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_NOT, OPR_WEN, OPR_WRITE,
};


typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
char* err_msg[] =
{
/*  0 */    "",
/*  1 */    "Found ':=' when expecting '='.",
/*  2 */    "There must be a number to follow '='.",
/*  3 */    "There must be an '=' to follow the identifier.",
/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
/*  5 */    "Missing ',' or ';'.",
/*  6 */    "Incorrect procedure name.",
/*  7 */    "Statement expected.",
/*  8 */    "Follow the statement is an incorrect symbol.",
/*  9 */    "'.' expected.",
/* 10 */    "';' expected.",
/* 11 */    "Undeclared identifier.",
/* 12 */    "Illegal assignment.",
/* 13 */    "':=' expected.",
/* 14 */    "There must be an identifier to follow the 'call'.",
/* 15 */    "A constant or variable can not be called.",
/* 16 */    "'then' expected.",
/* 17 */    "';' or 'end' expected.",
/* 18 */    "'do' expected.",
/* 19 */    "Incorrect symbol.",
/* 20 */    "Relative operators expected.",
/* 21 */    "Procedure identifier can not be in an expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by a factor.",
/* 24 */    "The symbol can not be as the beginning of an expression.",
/* 25 */    "The number is too great.",
/* 26 */    "expected ')' or '('",
/* 27 */    "can't break",
/* 28 */    "Missing ']'.",
/* 29 */    "Wrong Initialization of array",
/* 30 */    "",
/* 31 */    "",
/* 32 */    "There are too many levels."
};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;
int  cx = 0;         // index of current instruction to be generated.
int  level = 0;
int  tx = 0;		//符号表索引
int  ax = 0;		//数组符号表索引

int  ox = 0;		//声明变量顺序调整的代码数

int  order_adj = 0; //声明是否需要计数
int  offset = 0;
//int  offset_begin = 0;
int  backfill = 0;//是否需要回填
int  brace_num = 0;

/*
	注：在一个语句可能嵌套多个条件语句 比如 for（；；） begin if（）then statement end；
	这样for的条件，可能会与if的条件相重合 故真、假值链用多维数组，以避免回填时跳到错误指令
	用 condition_level 记录当前条件嵌套层数
	true_count[]、false_count[]表示当前层要跳到真、假出口的指令数目
*/
int true_out[4][10] = { 0 }; //短路计算 真值链 
int false_out[4][10] = { 0 };//短路计算 假值链
int true_count[4] = { 0 };
int false_count[4] = { 0 };
int condition_level = 0;
int mid_cx;

char line[80];

instruction code[CXMAX];
instruction initial_code[CXMAX];

char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while", "read","print"
};

/*
 *wsym数组增加以上关键字对应的symtype类型
 */
int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE, SYM_READ, SYM_WRITE
};

/*
*ssym数组增加了SYM_AND,SYM_OR,SYM_NOT,
*SYM_LSPAREN,SYM_RSPAREN,SYM_MOD共6个symtype类型
*/
int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON, SYM_LSPAREN, SYM_RSPAREN, SYM_LBRACE, SYM_RBRACE 
};

/*
*csym数组增加以上ssym数组对应symtype类型的符号
*/
char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';', '[', ']', '{' , '}'
};

/*
*mnemonic数组增加7条指令
*/
#define MAXINS   15
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "JPNC",
	"LDA", "STA", "RDA", "WTA", "READ", "WRITE"
};

typedef struct
{
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
} comtab;

//comtab table[TXMAX];


/*
 *mask结构添加value变量，将原comtab table[TXMAX]注释掉，
 *直接用mask结构来构建符号表。
*/
typedef struct
{
	char  name[MAXIDLEN + 1];
	int   kind;
	int   value;
	short level;
	short address;
} mask;
mask table[TXMAX];	


/*
 *添加数组结构
*/
typedef struct
{
	char name[MAXIDLEN + 1];	//数组变量名称
	int  sum;					//数组
	int  n;						//数组总维数
	int  dim[MAXDIM];			//数组对应维数的存储空间
	int  size[MAXDIM];			//数组对应维数的地址偏移量大小
	int  addr;					//数组首地址
}arr;

arr array_t, array_table[TXMAX];	//利用数组结构构建数组符号表


FILE* infile;
#endif // 
// EOF PL0.h
