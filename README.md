# USTC TinyC 编译系统闯关实验

每一关可以看master分支里的每次commit提交；再具体的每关卡的分析先🕊了，有空来填吧，要复习信息论了QwQ

## 关卡一 Tiny C初上手

这是一个示例代码分析题

`TinyC.c`过于简单的测试文件输入模块

```c
#include "stdio.h"

extern void SyntaxAnalysis();

FILE *sFile;
char name[12];

int main(int argc, char* argv[])
{
	scanf("%s",name);
    sFile=fopen( name,"rt");
	SyntaxAnalysis();
	fclose(sFile);

//Free all memories

	return 0;
}
```

`constvar.h`该头文件定义了需要用到的结构体、联合体、宏和状态转换表等

```c
//#define AnaTypeLex	1
//#define AnaTypeSyn	1
#define MAXTOKENLEN	256

typedef union {
	int number;
	char *str;
} TOKENVAL;

typedef struct {
	int token;
	TOKENVAL tokenVal;
} TERMINAL;

typedef union {
	int intval;
	char charval;
} EXPVALUE;

typedef struct expValue{
	int type;
	EXPVALUE val;
} EXPVAL;

typedef struct idNode{
	char name[MAXTOKENLEN];
	int type;
	EXPVALUE val;
	struct idNode *next;
} IDTABLE;

//词法分析DFA转换表
static int LexTable[6][8]=
   {{   1, 201, 204,   2,   3,   4,   5, 205},
	{ 101, 101, 101, 101, 101, 101, 101, 101},
	{ 102, 102, 202, 203, 102, 102, 102, 102},
	{ 103, 103, 103, 103, 103,   4, 103, 103},
	{ 104, 104, 104, 104, 104,   4, 104, 104},
	{ 105, 105, 105, 105, 105,   5,   5, 105}};


//用于词法分析输出，及语法分析
#define ERR			-1
#define SYN_NUM		1		// int整数
#define SYN_ID		2		// id
#define SYN_LT		11		// <
#define SYN_GT		12		// >
#define SYN_LE		13		// <=
#define SYN_GE		14		// >=
#define SYN_EQ		15		// ==
#define SYN_NE		16		// !=
#define SYN_ADD		17		// +
#define SYN_SUB		18		// -
#define SYN_MUL		19		// *
#define SYN_DIV		20		// /
#define SYN_PAREN_L	21		// (
#define SYN_PAREN_R	22		// )
#define SYN_BRACE_L	23		// {
#define SYN_BRACE_R	24		// }
#define SYN_COMMA	25		// ,
#define SYN_SEMIC	26		// ;
#define SYN_SET		27		// =
#define SYN_AND		51		// &&
#define SYN_OR		52		// ||
#define SYN_NOT		53		// !
#define SYN_TRUE	54		// TRUE
#define SYN_FALSE	55		// FALSE
#define SYN_INT		56		// int
#define SYN_CHAR	57		// char
#define SYN_IF		58		// if
#define SYN_ELSE	59		// else
#define SYN_WHILE	60		// while
#define SYN_SHOW	61		// show

//用于符号表中类型
#define ID_FUN		1		// 函数类型
#define ID_INT		2		// int类型
#define ID_CHAR		3		// char类型
```

`LexicalAnalysis.c`词法分析

```c
#include "stdio.h"
#include "constvar.h"

#define LEX_RELOOP	0
#define LEX_DELIM	1
#define LEX_MUL		2
#define LEX_DIV		3
#define LEX_ADDMIN	4
#define LEX_DIGIT	5
#define LEX_LETTER_	6
#define LEX_SYMBOL	7

static char ReadAChar();
static int FoundRELOOP();
static int STR2INT();
static int FoundKeyword();
static int strcompare(char *sstr, char*tstr);

extern FILE *sFile;
static char prebuf=0;	//buffer to store the pre-read character
static char tokenStr[MAXTOKENLEN];	//token buffer
static int tokenLen;

TERMINAL nextToken()
{
	TERMINAL token;
	int state=0;
	char c,d;
	token.token=ERR;
	token.tokenVal.number=0;
	tokenLen=0;
	for (c=ReadAChar(sFile);c!=0;c=ReadAChar(sFile))
	{	tokenStr[tokenLen++]=c;
		if (tokenLen>=MAXTOKENLEN)
		{	printf("Token is too long!\n");
			break;
		}
		if (feof(sFile))
			state=LexTable[state][LEX_DELIM];
		else if (c=='<' || c=='>' || c=='=' || c=='!' || c=='&' || c=='|')
			state=LexTable[state][LEX_RELOOP];
		else if (c==' ' || c=='\t' || c=='\n')
			state=LexTable[state][LEX_DELIM];
		else if (c=='*')
			state=LexTable[state][LEX_MUL];
		else if (c=='/')
			state=LexTable[state][LEX_DIV];
		else if (c=='+' || c=='-')
			state=LexTable[state][LEX_ADDMIN];
		else if (c>='0' && c<='9')
			state=LexTable[state][LEX_DIGIT];
		else if ((c>='a' && c<='z')||(c>='A' && c<='Z')||(c=='_'))
			state=LexTable[state][LEX_LETTER_];
		else if (c=='(' || c==')' || c=='{' || c=='}' || c==',' || c==';')
			state=LexTable[state][LEX_SYMBOL];
		else
		{	printf("Unknown symbol: %c\n",c);
			break;
		}
		if (state<100) continue;
		if (state>100 && state<200)
		{	prebuf=c;
			tokenLen--;
		}
		switch (state)
		{	case 101: token.token=FoundRELOOP();
					  break;
			case 102: token.token=SYN_DIV;
					  break;
			case 103: if (tokenStr[0]=='+') token.token=SYN_ADD;
					  else token.token=SYN_SUB;
					  break;
			case 104: token.token=SYN_NUM;
					  token.tokenVal.number=STR2INT();
					  break;
			case 105: tokenStr[tokenLen]='\0';
					  token.token=FoundKeyword();
					  token.tokenVal.str=tokenStr;
					  break;
			case 201: if (feof(sFile))
					  {//	  printf("Meet file end!\n");
						  token.token=-1;
						  break;
					  }
					  state=0; tokenLen=0;
					  continue;
			case 202: c=ReadAChar(sFile);
					  while (!feof(sFile) && ((d=ReadAChar(sFile))!='/' || c!='*'))
						  c=d;
					  state=0; tokenLen=0;
					  continue;
			case 203: while ((c=ReadAChar(sFile))!='\n' && (!feof(sFile)));
					  state=0; tokenLen=0;
					  continue;
			case 204: token.token=SYN_MUL;
					  break;
			case 205: if (tokenStr[0]=='(') token.token=SYN_PAREN_L;
					  else if (tokenStr[0]==')') token.token=SYN_PAREN_R;
					  else if (tokenStr[0]=='{') token.token=SYN_BRACE_L;
					  else if (tokenStr[0]=='}') token.token=SYN_BRACE_R;
					  else if (tokenStr[0]==',') token.token=SYN_COMMA;
					  else if (tokenStr[0]==';') token.token=SYN_SEMIC;
					  break;
			default: break;
		}
		break;
	}
	return(token);
}

void renewLex()
{
	prebuf=0;
}

static char ReadAChar(FILE *sFile)
{
	char c;
	if (prebuf!=0)
	{
		c=prebuf;
		prebuf=0;
	}
	else if (!feof(sFile))
		c=fgetc(sFile);
	else
		c=0;
	return(c);
}

static int FoundRELOOP()
{
	if (tokenStr[0]=='<' && tokenStr[1]!='=') return(SYN_LT);
	else if (tokenStr[0]=='<' && tokenStr[1]=='=') { prebuf=0; return(SYN_LE); }
	else if (tokenStr[0]=='>' && tokenStr[1]!='=') return(SYN_GT);
	else if (tokenStr[0]=='>' && tokenStr[1]=='=') { prebuf=0; return(SYN_GE); }
	else if (tokenStr[0]=='=' && tokenStr[1]!='=') return(SYN_SET);
	else if (tokenStr[0]=='=' && tokenStr[1]=='=') return(SYN_EQ);
	else if (tokenStr[0]=='!' && tokenStr[1]!='=') return(SYN_NOT);
	else if (tokenStr[0]=='!' && tokenStr[1]=='=') return(SYN_NE);
	else if (tokenStr[0]=='&' && tokenStr[1]=='&') return(SYN_AND);
	else if (tokenStr[0]=='|' && tokenStr[1]=='|') return(SYN_OR);
	else return(ERR);
}

static int STR2INT()
{
	int i,s=0;
	for (i=0;i<tokenLen;i++)
		s=s*10+tokenStr[i]-'0';
	return(s);
}

static int FoundKeyword()
{
	if (strcompare(tokenStr,"TRUE")) return(SYN_TRUE);
	if (strcompare(tokenStr,"FALSE")) return(SYN_FALSE);
	if (strcompare(tokenStr,"int")) return(SYN_INT);
	if (strcompare(tokenStr,"char")) return(SYN_CHAR);
	if (strcompare(tokenStr,"if")) return(SYN_IF);
	if (strcompare(tokenStr,"else")) return(SYN_ELSE);
	if (strcompare(tokenStr,"while")) return(SYN_WHILE);
	if (strcompare(tokenStr,"show")) return(SYN_SHOW);
	return(SYN_ID);
}

static int strcompare(char *sstr, char*tstr)
{
	while (*sstr==*tstr && *sstr!='\0') { sstr++; tstr++; }
	if (*sstr=='\0' && *tstr=='\0')	return(1);
	else return(0);
}
```

`SyntaxAnalysis.c`语法分析

```c
#include "stdio.h"
#include "stdlib.h"
#include "constvar.h"

extern TERMINAL nextToken();
extern void renewLex();
static int match (int t);
static int strcompare(char *sstr, char *tstr);	//比较两个串
static IDTABLE* InstallID();		//在符号表中为curtoken_str建立一个条目
static IDTABLE* LookupID();			//在符号表中查找curtoken_str
static void FreeExit();
static int cast2int(EXPVAL exp);		//将exp的值转换为int类型
static char cast2char(EXPVAL exp);		//将exp的值转换为char类型
static int Prod_FUNC();
static int Prod_S();
static int Prod_D();
static int Prod_L(int type);
static int Prod_T();
static int Prod_A();
static int Prod_B();
static int Prod_B1(int bval);
static int Prod_TB();
static int Prod_TB1(int bval);
static int Prod_FB();
static EXPVAL Prod_E();
static EXPVAL Prod_E1(EXPVAL val);
static EXPVAL Prod_TE();
static EXPVAL Prod_TE1(EXPVAL val);
static EXPVAL Prod_F();

extern FILE *sFile;
static TERMINAL lookahead;
static int curtoken_num;
static char curtoken_str[MAXTOKENLEN];
static IDTABLE *IDTHead=NULL;
static int run_status=1;	//0；程序不执行；1:程序正常执行；2:跳过当前结构后继续执行

void SyntaxAnalysis()
{
#if defined(AnaTypeLex)
//testing lexical analysis
	TERMINAL token;
	token=nextToken();
	while (token.token!=ERR)
	{	if (token.token==SYN_NUM)
			printf("LEX: %d,%d\n",token.token,token.tokenVal.number);
		else if (token.token==SYN_ID)
			printf("LEX: %d,%s\n",token.token,token.tokenVal.str);
		else
			printf("LEX: %d\n",token.token);
		token=nextToken();
	}
#else
//syntax analysis
	lookahead=nextToken();
	if (Prod_FUNC()==ERR)
		printf("PROGRAM HALT!\n");
	FreeExit();

#endif
}

static int match (int t)
{
	char *p,*q;
	if (lookahead.token == t)
	{	if (t==SYN_NUM)
			curtoken_num=lookahead.tokenVal.number;
		else if (t==SYN_ID)
			for (p=lookahead.tokenVal.str,q=curtoken_str;(*q=*p)!='\0';p++,q++);
		lookahead = nextToken( );
	}
	else
		FreeExit();
	return(0);
}

static int strcompare(char *sstr, char *tstr)
{
	while (*sstr==*tstr && *sstr!='\0') { sstr++; tstr++; }
	if (*sstr=='\0' && *tstr=='\0')	return(0);
	else return(ERR);
}

static IDTABLE* InstallID()
{
	IDTABLE *p,*q;
	char *a,*b;
	p=IDTHead; q=NULL;
	while (p!=NULL && strcompare(curtoken_str,p->name)==ERR)
	{
		q=p;
		p=p->next;
	}
	if (p!=NULL)
		return(NULL);
	else
	{
		p=(IDTABLE*)malloc(sizeof(IDTABLE));
		if (q==NULL)
			IDTHead=p;
		else
			q->next=p;
		p->next=NULL;
		for (a=curtoken_str,b=p->name;(*b=*a)!='\0';a++,b++);
		return(p);
	}
}

static IDTABLE* LookupID()
{
	IDTABLE *p;
	p=IDTHead;
	while (p!=NULL && strcompare(curtoken_str,p->name)==ERR)
		p=p->next;
	if (p==NULL)
		return(NULL);
	else
		return(p);
}

static void FreeExit()
{
	IDTABLE *p,*q;
	//释放链表空间
	p=IDTHead;
	while ((q=p)!=NULL)
	{	p=p->next;
		#if defined(AnaTypeSyn)
		printf("NAME:%s, TYPE:%d, ",q->name,q->type);
		if (q->type==ID_INT)
			printf("VALUE:%d\n",q->val.intval);
		else if (q->type==ID_CHAR)
			printf("VALUE:%c\n",q->val.charval);
		else
			printf("\n");
		#endif
		free(q);
	}
	exit(0);
}

static int cast2int(EXPVAL exp)
{
	if (exp.type==ID_INT)
		return(exp.val.intval);
	else if (exp.type==ID_CHAR)
		return((int)(exp.val.charval));
}

static char cast2char(EXPVAL exp)
{
	if (exp.type==ID_INT)
		return((char)(exp.val.intval));
	else if (exp.type==ID_CHAR)
		return(exp.val.charval);
}

static int Prod_FUNC()
{
	IDTABLE *p;
	match(SYN_ID);
	if (strcompare(curtoken_str,"main")==ERR) FreeExit();
	p=InstallID();
	p->type=ID_FUN;
	#if defined(AnaTypeSyn)
	printf("SYN: FUNC-->main() {S}\n");
	#endif
	match(SYN_PAREN_L);
	match(SYN_PAREN_R);
	match(SYN_BRACE_L);
	Prod_S();
	match(SYN_BRACE_R);
	return(0);
}

static int Prod_S()
{
	long file_index;
	EXPVAL exp;
	int bval;
	if (lookahead.token==SYN_INT || lookahead.token==SYN_CHAR)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->D S\n");
		#endif
		Prod_D();
		Prod_S();
	}
	else if (lookahead.token==SYN_ID)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->A S\n");
		#endif
		Prod_A();
		Prod_S();
	}
	else if (lookahead.token==SYN_SHOW)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->show(E); S\n");
		#endif
		match(SYN_SHOW);
		match(SYN_PAREN_L);
		exp=Prod_E();
		match(SYN_PAREN_R);
		match(SYN_SEMIC);
		if (run_status==1)
			if (exp.type==ID_INT)
				printf("%d",exp.val.intval);
			else if (exp.type==ID_CHAR)
				printf("%c",exp.val.charval);
		Prod_S();
	}
	else if (lookahead.token==SYN_IF)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->if (B) {S} [else {S}] S");
		#endif
		match(SYN_IF);
		match(SYN_PAREN_L);
		bval=Prod_B();
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) run_status=2;
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		if (lookahead.token==SYN_ELSE)
		{
			match(SYN_ELSE);
			if (run_status==1) run_status=2;
			else if (run_status==2) run_status=1;
			match(SYN_BRACE_L);
			Prod_S();
			match(SYN_BRACE_R);
			if (run_status==2) run_status=1;
		}
		Prod_S();
	}
	else if (lookahead.token==SYN_WHILE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->while(B) {S} S\n");
		#endif
		match(SYN_WHILE);
		file_index=ftell(sFile)-6;
		match(SYN_PAREN_L);
		bval=Prod_B();
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) run_status=2;
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		if (run_status==1)
		{	fseek(sFile,file_index,SEEK_SET);
			renewLex();
		}
		else if (run_status==2)
			run_status=1;
		Prod_S();
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S--> \n");
		#endif
	}
	return(0);
}

static int Prod_D()
{
	int type;
	IDTABLE *p;
	EXPVAL exp;
	#if defined(AnaTypeSyn)
	printf("SYN: D-->T id [=E] L;\n");
	#endif
	type=Prod_T();
	match(SYN_ID);
	p=InstallID();
	p->type=type;
	if (lookahead.token==SYN_SET)
	{
		match(SYN_SET);
		exp=Prod_E();
		if (run_status==1)
		{	if (type==ID_INT)
				p->val.intval=cast2int(exp);
			else if (type==ID_CHAR)
				p->val.charval=cast2char(exp);
		}
	}
	Prod_L(type);
	match(SYN_SEMIC);
	return(0);
}

static int Prod_L(int type)
{
	IDTABLE *p;
	EXPVAL exp;
	if (lookahead.token==SYN_COMMA)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: L-->, id [=E] L\n");
		#endif
		match(SYN_COMMA);
		match(SYN_ID);
		p=InstallID();
		p->type=type;
		if (lookahead.token==SYN_SET)
		{
			match(SYN_SET);
			exp=Prod_E();
			if (run_status==1)
			{	if (type==ID_INT)
					p->val.intval=cast2int(exp);
				else if (type==ID_CHAR)
					p->val.charval=cast2char(exp);
			}
		}
		Prod_L(type);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: L--> \n");
		#endif
	}
	return(0);
}

static int Prod_T()
{
	if (lookahead.token==SYN_INT)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: T-->int\n");
		#endif
		match(SYN_INT);
		return(ID_INT);
	}
	else if (lookahead.token==SYN_CHAR)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: T-->char\n");
		#endif
		match(SYN_CHAR);
		return(ID_CHAR);
	}
	else
		FreeExit();
	return(0);
}

static int Prod_A()
{
	IDTABLE *p;
	EXPVAL exp;
	#if defined(AnaTypeSyn)
	printf("SYN: A-->id=E;\n");
	#endif
	match(SYN_ID);
	p=LookupID();
	match(SYN_SET);
	exp=Prod_E();
	match(SYN_SEMIC);
	if (run_status==1)
	{	if (p->type==ID_INT)
			p->val.intval=cast2int(exp);
		else if (p->type==ID_CHAR)
			p->val.charval=cast2char(exp);
	}
	return(0);
}

static int Prod_B()
{
	int bval1,bval2;
	#if defined(AnaTypeSyn)
	printf("SYN: B-->id=TB B1\n");
	#endif
	bval1=Prod_TB();
	bval2=Prod_B1(bval1);
	return(bval2);
}

static int Prod_B1(int bval1)
{
	int bval2;
	if (lookahead.token==SYN_OR)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: B1-->|| B1\n");
		#endif
		match(SYN_OR);
		bval2=Prod_TB();
		bval1=(run_status==1 && (bval1==1 || bval2==1)) ? 1 : 0;
		bval2=Prod_B1(bval1);
		return(bval2);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: B1--> \n");
		#endif
		return(bval1);
	}
}

static int Prod_TB()
{
	int bval1,bval2;
	#if defined(AnaTypeSyn)
	printf("SYN: TB-->FB TB1\n");
	#endif
	bval1=Prod_FB();
	bval2=Prod_TB1(bval1);
	return(bval2);
}

static int Prod_TB1(int bval1)
{
	int bval2;
	if (lookahead.token==SYN_AND)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TB1-->&& TB1\n");
		#endif
		match(SYN_AND);
		bval1=Prod_FB();
		bval1=(run_status==1 && (bval1==1 && bval2==1)) ? 1 : 0;
		bval2=Prod_TB1(bval1);
		return(bval2);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TB1--> \n");
		#endif
		return(bval1);
	}
}

static int Prod_FB()
{
	int bval;
	EXPVAL val1,val2;
	int ival1,ival2;
	if (lookahead.token==SYN_NOT)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: FB-->!B\n");
		#endif
		match(SYN_NOT);
		bval=Prod_B();
		return(run_status==1 ? 1-bval : 0);
	}
	else if (lookahead.token==SYN_TRUE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: FB-->TRUE\n");
		#endif
		match(SYN_TRUE);
		return(run_status==1 ? 1 : 0);
	}
	else if (lookahead.token==SYN_FALSE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: FB-->FALSE\n");
		#endif
		match(SYN_FALSE);
		return(run_status==1 ? 0 : 0);
	}
	else if (lookahead.token==SYN_ID || lookahead.token==SYN_NUM || lookahead.token==SYN_PAREN_L)
	{
		val1=Prod_E();
		if (run_status==1) ival1=cast2int(val1);
		if (lookahead.token==SYN_LT)
		{	
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E<E\n");
			#endif
			match(SYN_LT);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1<ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_LE)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E<=E\n");
			#endif
			match(SYN_LE);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1<=ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_GT)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E>E\n");
			#endif
			match(SYN_GT);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1>ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_GE)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E>=E\n");
			#endif
			match(SYN_GE);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1>=ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_EQ)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E==E\n");
			#endif
			match(SYN_EQ);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1==ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_NE)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E!=E\n");
			#endif
			match(SYN_NE);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1!=ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else
		{
			if (run_status==1)
				return(ival1!=0 ? 1 : 0);
			else
				return(0);
		}

	}
	else
	{	FreeExit();
		return(0);
	}
}

static EXPVAL Prod_E()
{
	EXPVAL val1,val2;
	#if defined(AnaTypeSyn)
	printf("SYN: E-->TE E1\n");
	#endif
	val1=Prod_TE();
	val2=Prod_E1(val1);
	return(val2);
}

static EXPVAL Prod_E1(EXPVAL val1)
{
	EXPVAL val2,val;
	int i1,i2;
	char c1,c2;
	if (lookahead.token==SYN_ADD)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1-->+TE E1\n");
		#endif
		match(SYN_ADD);
		val2=Prod_TE();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1+i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1+c2;
			}
		val=Prod_E1(val);
	}
	else if (lookahead.token==SYN_SUB)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1-->-TE E1\n");
		#endif
		match(SYN_SUB);
		val2=Prod_TE();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1-i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1-c2;
			}
		val=Prod_E1(val);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1--> \n");
		#endif
		val=val1;
	}
	return(val);
}

static EXPVAL Prod_TE()
{
	EXPVAL val1,val2;
	#if defined(AnaTypeSyn)
	printf("SYN: TE-->F TE1\n");
	#endif
	val1=Prod_F();
	val2=Prod_TE1(val1);
	return(val2);
}

static EXPVAL Prod_TE1(EXPVAL val1)
{
	EXPVAL val2,val;
	int i1,i2;
	char c1,c2;
	if (lookahead.token==SYN_MUL)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TE1-->*F TE1\n");
		#endif
		match(SYN_MUL);
		val2=Prod_F();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1*i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1*c2;
			}
		val=Prod_TE1(val);
	}
	else if (lookahead.token==SYN_DIV)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TE1-->/F TE1\n");
		#endif
		match(SYN_DIV);
		val2=Prod_F();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1/i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1/c2;
			}
		val=Prod_TE1(val);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TE1--> \n");
		#endif
		val=val1;
	}
	return(val);
}

static EXPVAL Prod_F()
{
	EXPVAL val;
	static IDTABLE *p;
	if (lookahead.token==SYN_NUM)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->num\n");
		#endif
		match(SYN_NUM);
		val.type=ID_INT;
		val.val.intval=curtoken_num;
	}
	else if (lookahead.token==SYN_ID)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->id\n");
		#endif
		match(SYN_ID);
		p=LookupID();
		val.type=p->type;
		val.val=p->val;
	}
	else if (lookahead.token==SYN_PAREN_L)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->(B)\n");
		#endif
		match(SYN_PAREN_L);
		val=Prod_E();
		match(SYN_PAREN_R);
	}
	else
		FreeExit();
	return(val);
}

```

## 关卡二 完善表达式计算

> 测试现有的TinyC能否成功解释`i=i+1`，`i=1+1`和`i=1+i`。
>
> 对照文法，思考代码的错误可能出现在哪里。 你也可以进一步测试`i=1+i+1`，`i=i*2`和`i=2*i`等语句来帮助你找到代码出错的地方。

LexicalAnalysis.c

```c
#include "stdio.h"
#include "constvar.h"

#define LEX_RELOOP	0
#define LEX_DELIM	1
#define LEX_MUL		2
#define LEX_DIV		3
#define LEX_ADDMIN	4
#define LEX_DIGIT	5
#define LEX_LETTER_	6
#define LEX_SYMBOL	7

static char ReadAChar();
static int FoundRELOOP();
static int STR2INT();
static int FoundKeyword();
static int strcompare(char *sstr, char*tstr);

extern FILE *sFile;
static char prebuf=0;	//buffer to store the pre-read character
static char tokenStr[MAXTOKENLEN];	//token buffer
static int tokenLen;

TERMINAL nextToken()
{
	TERMINAL token;
	int state=0;
	char c,d;
	token.token=ERR;
	token.tokenVal.number=0;
	tokenLen=0;
	for (c=ReadAChar(sFile);c!=0;c=ReadAChar(sFile))
	{	tokenStr[tokenLen++]=c;
		if (tokenLen>=MAXTOKENLEN)
		{	printf("Token is too long!\n");
			break;
		}
		if (feof(sFile))
			state=LexTable[state][LEX_DELIM];
		else if (c=='<' || c=='>' || c=='=' || c=='!' || c=='&' || c=='|')
			state=LexTable[state][LEX_RELOOP];
		else if (c==' ' || c=='\t' || c=='\n')
			state=LexTable[state][LEX_DELIM];
		else if (c=='*')
			state=LexTable[state][LEX_MUL];
		else if (c=='/')
			state=LexTable[state][LEX_DIV];
		else if (c=='+' || c=='-')
			state=LexTable[state][LEX_ADDMIN];
		else if (c>='0' && c<='9')
			state=LexTable[state][LEX_DIGIT];
		else if ((c>='a' && c<='z')||(c>='A' && c<='Z')||(c=='_'))
			state=LexTable[state][LEX_LETTER_];
		else if (c=='(' || c==')' || c=='{' || c=='}' || c==',' || c==';')
			state=LexTable[state][LEX_SYMBOL];
		else
		{	printf("Unknown symbol: %c\n",c);
			break;
		}
		if (state<100) continue;
		if (state>100 && state<200)
		{	prebuf=c;
			tokenLen--;
		}
		switch (state)
		{	case 101: token.token=FoundRELOOP();
					  break;
			case 102: token.token=SYN_DIV;
					  break;
			case 103: if (tokenStr[0]=='+') token.token=SYN_ADD;
					  else token.token=SYN_SUB;
					  break;
			case 104: token.token=SYN_NUM;
					  token.tokenVal.number=STR2INT();
					  break;
			case 105: tokenStr[tokenLen]='\0';
					  token.token=FoundKeyword();
					  token.tokenVal.str=tokenStr;
					  break;
			case 201: if (feof(sFile))
					  {//	  printf("Meet file end!\n");
						  token.token=-1;
						  break;
					  }
					  state=0; tokenLen=0;
					  continue;
			case 202: c=ReadAChar(sFile);
					  while (!feof(sFile) && ((d=ReadAChar(sFile))!='/' || c!='*'))
						  c=d;
					  state=0; tokenLen=0;
					  continue;
			case 203: while ((c=ReadAChar(sFile))!='\n' && (!feof(sFile)));
					  state=0; tokenLen=0;
					  continue;
			case 204: token.token=SYN_MUL;
					  break;
			case 205: if (tokenStr[0]=='(') token.token=SYN_PAREN_L;
					  else if (tokenStr[0]==')') token.token=SYN_PAREN_R;
					  else if (tokenStr[0]=='{') token.token=SYN_BRACE_L;
					  else if (tokenStr[0]=='}') token.token=SYN_BRACE_R;
					  else if (tokenStr[0]==',') token.token=SYN_COMMA;
					  else if (tokenStr[0]==';') token.token=SYN_SEMIC;
					  break;
			default: break;
		}
		break;
	}
	return(token);
}

void renewLex()
{
	prebuf=0;
}

static char ReadAChar(FILE *sFile)
{
	char c;
	if (prebuf!=0)
	{
		c=prebuf;
		prebuf=0;
	}
	else if (!feof(sFile))
		c=fgetc(sFile);
	else
		c=0;
	return(c);
}

static int FoundRELOOP()
{
	if (tokenStr[0]=='<' && tokenStr[1]!='=') return(SYN_LT);
	else if (tokenStr[0]=='<' && tokenStr[1]=='=') { prebuf=0; return(SYN_LE); }
	else if (tokenStr[0]=='>' && tokenStr[1]!='=') return(SYN_GT);
	else if (tokenStr[0]=='>' && tokenStr[1]=='=') { prebuf=0; return(SYN_GE); }
	else if (tokenStr[0]=='=' && tokenStr[1]!='=') return(SYN_SET);
	else if (tokenStr[0]=='=' && tokenStr[1]=='=') return(SYN_EQ);
	else if (tokenStr[0]=='!' && tokenStr[1]!='=') return(SYN_NOT);
	else if (tokenStr[0]=='!' && tokenStr[1]=='=') return(SYN_NE);
	else if (tokenStr[0]=='&' && tokenStr[1]=='&') return(SYN_AND);
	else if (tokenStr[0]=='|' && tokenStr[1]=='|') return(SYN_OR);
	else return(ERR);
}

static int STR2INT()
{
	int i,s=0;
	for (i=0;i<tokenLen;i++)
		s=s*10+tokenStr[i]-'0';
	return(s);
}

static int FoundKeyword()
{
	if (strcompare(tokenStr,"TRUE")) return(SYN_TRUE);
	if (strcompare(tokenStr,"FALSE")) return(SYN_FALSE);
	if (strcompare(tokenStr,"int")) return(SYN_INT);
	if (strcompare(tokenStr,"char")) return(SYN_CHAR);
	if (strcompare(tokenStr,"if")) return(SYN_IF);
	if (strcompare(tokenStr,"else")) return(SYN_ELSE);
	if (strcompare(tokenStr,"while")) return(SYN_WHILE);
	if (strcompare(tokenStr,"show")) return(SYN_SHOW);
	return(SYN_ID);
}

static int strcompare(char *sstr, char*tstr)
{
	while (*sstr==*tstr && *sstr!='\0') { sstr++; tstr++; }
	if (*sstr=='\0' && *tstr=='\0')	return(1);
	else return(0);
}

```

SyntaxAnalysis.c

```c
#include "stdio.h"
#include "stdlib.h"
#include "constvar.h"

extern TERMINAL nextToken();
extern void renewLex();
static int match (int t);
static int strcompare(char *sstr, char *tstr);	//比较两个串
static IDTABLE* InstallID();		//在符号表中为curtoken_str建立一个条目
static IDTABLE* LookupID();			//在符号表中查找curtoken_str
static void FreeExit();
static int cast2int(EXPVAL exp);		//将exp的值转换为int类型
static char cast2char(EXPVAL exp);		//将exp的值转换为char类型
static int Prod_FUNC();
static int Prod_S();
static int Prod_D();
static int Prod_L(int type);
static int Prod_T();
static int Prod_A();
static int Prod_B();
static int Prod_B1(int bval);
static int Prod_TB();
static int Prod_TB1(int bval);
static int Prod_FB();
static EXPVAL Prod_E();
static EXPVAL Prod_E1(EXPVAL val);
static EXPVAL Prod_TE();
static EXPVAL Prod_TE1(EXPVAL val);
static EXPVAL Prod_F();

extern FILE *sFile;
static TERMINAL lookahead;
static int curtoken_num;
static char curtoken_str[MAXTOKENLEN];
static IDTABLE *IDTHead=NULL;
static int run_status=1;	//0；程序不执行；1:程序正常执行；2:跳过当前结构后继续执行

void SyntaxAnalysis()
{
#if defined(AnaTypeLex)
//testing lexical analysis
	TERMINAL token;
	token=nextToken();
	while (token.token!=ERR)
	{	if (token.token==SYN_NUM)
			printf("LEX: %d,%d\n",token.token,token.tokenVal.number);
		else if (token.token==SYN_ID)
			printf("LEX: %d,%s\n",token.token,token.tokenVal.str);
		else
			printf("LEX: %d\n",token.token);
		token=nextToken();
	}
#else
//syntax analysis
	lookahead=nextToken();
	if (Prod_FUNC()==ERR)
		printf("PROGRAM HALT!\n");
	FreeExit();

#endif
}

static int match (int t)
{
	char *p,*q;
	if (lookahead.token == t)
	{	if (t==SYN_NUM)
			curtoken_num=lookahead.tokenVal.number;
		else if (t==SYN_ID)
			for (p=lookahead.tokenVal.str,q=curtoken_str;(*q=*p)!='\0';p++,q++);
		lookahead = nextToken( );
	}
	else
		FreeExit();
	return(0);
}

static int strcompare(char *sstr, char *tstr)
{
	while (*sstr==*tstr && *sstr!='\0') { sstr++; tstr++; }
	if (*sstr=='\0' && *tstr=='\0')	return(0);
	else return(ERR);
}

static IDTABLE* InstallID()
{
	IDTABLE *p,*q;
	char *a,*b;
	p=IDTHead; q=NULL;
	while (p!=NULL && strcompare(curtoken_str,p->name)==ERR)
	{
		q=p;
		p=p->next;
	}
	if (p!=NULL)
		return(NULL);
	else
	{
		p=(IDTABLE*)malloc(sizeof(IDTABLE));
		if (q==NULL)
			IDTHead=p;
		else
			q->next=p;
		p->next=NULL;
		for (a=curtoken_str,b=p->name;(*b=*a)!='\0';a++,b++);
		return(p);
	}
}

static IDTABLE* LookupID()
{
	IDTABLE *p;
	p=IDTHead;
	while (p!=NULL && strcompare(curtoken_str,p->name)==ERR)
		p=p->next;
	if (p==NULL)
		return(NULL);
	else
		return(p);
}

static void FreeExit()
{
	IDTABLE *p,*q;
	//释放链表空间
	p=IDTHead;
	while ((q=p)!=NULL)
	{	p=p->next;
		#if defined(AnaTypeSyn)
		printf("NAME:%s, TYPE:%d, ",q->name,q->type);
		if (q->type==ID_INT)
			printf("VALUE:%d\n",q->val.intval);
		else if (q->type==ID_CHAR)
			printf("VALUE:%c\n",q->val.charval);
		else
			printf("\n");
		#endif
		free(q);
	}
	exit(0);
}

static int cast2int(EXPVAL exp)
{
	if (exp.type==ID_INT)
		return(exp.val.intval);
	else if (exp.type==ID_CHAR)
		return((int)(exp.val.charval));
}

static char cast2char(EXPVAL exp)
{
	if (exp.type==ID_INT)
		return((char)(exp.val.intval));
	else if (exp.type==ID_CHAR)
		return(exp.val.charval);
}

static int Prod_FUNC()
{
	IDTABLE *p;
	match(SYN_ID);
	if (strcompare(curtoken_str,"main")==ERR) FreeExit();
	p=InstallID();
	p->type=ID_FUN;
	#if defined(AnaTypeSyn)
	printf("SYN: FUNC-->main() {S}\n");
	#endif
	match(SYN_PAREN_L);
	match(SYN_PAREN_R);
	match(SYN_BRACE_L);
	Prod_S();
	match(SYN_BRACE_R);
	return(0);
}

static int Prod_S()
{
	long file_index;
	EXPVAL exp;
	int bval;
	if (lookahead.token==SYN_INT || lookahead.token==SYN_CHAR)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->D S\n");
		#endif
		Prod_D();
		Prod_S();
	}
	else if (lookahead.token==SYN_ID)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->A S\n");
		#endif
		Prod_A();
		Prod_S();
	}
	else if (lookahead.token==SYN_SHOW)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->show(E); S\n");
		#endif
		match(SYN_SHOW);
		match(SYN_PAREN_L);
		exp=Prod_E();
		match(SYN_PAREN_R);
		match(SYN_SEMIC);
		if (run_status==1)
			if (exp.type==ID_INT)
				printf("%d",exp.val.intval);
			else if (exp.type==ID_CHAR)
				printf("%c",exp.val.charval);
		Prod_S();
	}
	else if (lookahead.token==SYN_IF)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->if (B) {S} [else {S}] S");
		#endif
		match(SYN_IF);
		match(SYN_PAREN_L);
		bval=Prod_B();
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) run_status=2;
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		if (lookahead.token==SYN_ELSE)
		{
			match(SYN_ELSE);
			if (run_status==1) run_status=2;
			else if (run_status==2) run_status=1;
			match(SYN_BRACE_L);
			Prod_S();
			match(SYN_BRACE_R);
			if (run_status==2) run_status=1;
		}
		Prod_S();
	}
	else if (lookahead.token==SYN_WHILE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->while(B) {S} S\n");
		#endif
		match(SYN_WHILE);
		file_index=ftell(sFile)-6;
		match(SYN_PAREN_L);
		bval=Prod_B();
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) run_status=2;
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		if (run_status==1)
		{	fseek(sFile,file_index,SEEK_SET);
			renewLex();
		}
		else if (run_status==2)
			run_status=1;
		Prod_S();
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S--> \n");
		#endif
	}
	return(0);
}

static int Prod_D()
{
	int type;
	IDTABLE *p;
	EXPVAL exp;
	#if defined(AnaTypeSyn)
	printf("SYN: D-->T id [=E] L;\n");
	#endif
	type=Prod_T();
	match(SYN_ID);
	p=InstallID();
	p->type=type;
	if (lookahead.token==SYN_SET)
	{
		match(SYN_SET);
		exp=Prod_E();
		if (run_status==1)
		{	if (type==ID_INT)
				p->val.intval=cast2int(exp);
			else if (type==ID_CHAR)
				p->val.charval=cast2char(exp);
		}
	}
	Prod_L(type);
	match(SYN_SEMIC);
	return(0);
}

static int Prod_L(int type)
{
	IDTABLE *p;
	EXPVAL exp;
	if (lookahead.token==SYN_COMMA)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: L-->, id [=E] L\n");
		#endif
		match(SYN_COMMA);
		match(SYN_ID);
		p=InstallID();
		p->type=type;
		if (lookahead.token==SYN_SET)
		{
			match(SYN_SET);
			exp=Prod_E();
			if (run_status==1)
			{	if (type==ID_INT)
					p->val.intval=cast2int(exp);
				else if (type==ID_CHAR)
					p->val.charval=cast2char(exp);
			}
		}
		Prod_L(type);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: L--> \n");
		#endif
	}
	return(0);
}

static int Prod_T()
{
	if (lookahead.token==SYN_INT)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: T-->int\n");
		#endif
		match(SYN_INT);
		return(ID_INT);
	}
	else if (lookahead.token==SYN_CHAR)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: T-->char\n");
		#endif
		match(SYN_CHAR);
		return(ID_CHAR);
	}
	else
		FreeExit();
	return(0);
}

static int Prod_A()
{
	IDTABLE *p;
	EXPVAL exp;
	#if defined(AnaTypeSyn)
	printf("SYN: A-->id=E;\n");
	#endif
	match(SYN_ID);
	p=LookupID();
	match(SYN_SET);
	exp=Prod_E();
	match(SYN_SEMIC);
	if (run_status==1)
	{	if (p->type==ID_INT)
			p->val.intval=cast2int(exp);
		else if (p->type==ID_CHAR)
			p->val.charval=cast2char(exp);
	}
	return(0);
}

static int Prod_B()
{
	int bval1,bval2;
	#if defined(AnaTypeSyn)
	printf("SYN: B-->id=TB B1\n");
	#endif
	bval1=Prod_TB();
	bval2=Prod_B1(bval1);
	return(bval2);
}

static int Prod_B1(int bval1)
{
	int bval2;
	if (lookahead.token==SYN_OR)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: B1-->|| B1\n");
		#endif
		match(SYN_OR);
		bval2=Prod_TB();
		bval1=(run_status==1 && (bval1==1 || bval2==1)) ? 1 : 0;
		bval2=Prod_B1(bval1);
		return(bval2);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: B1--> \n");
		#endif
		return(bval1);
	}
}

static int Prod_TB()
{
	int bval1,bval2;
	#if defined(AnaTypeSyn)
	printf("SYN: TB-->FB TB1\n");
	#endif
	bval1=Prod_FB();
	bval2=Prod_TB1(bval1);
	return(bval2);
}

static int Prod_TB1(int bval1)
{
	int bval2;
	if (lookahead.token==SYN_AND)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TB1-->&& TB1\n");
		#endif
		match(SYN_AND);
		bval1=Prod_FB();
		bval1=(run_status==1 && (bval1==1 && bval2==1)) ? 1 : 0;
		bval2=Prod_TB1(bval1);
		return(bval2);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TB1--> \n");
		#endif
		return(bval1);
	}
}

static int Prod_FB()
{
	int bval;
	EXPVAL val1,val2;
	int ival1,ival2;
	if (lookahead.token==SYN_NOT)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: FB-->!B\n");
		#endif
		match(SYN_NOT);
		bval=Prod_B();
		return(run_status==1 ? 1-bval : 0);
	}
	else if (lookahead.token==SYN_TRUE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: FB-->TRUE\n");
		#endif
		match(SYN_TRUE);
		return(run_status==1 ? 1 : 0);
	}
	else if (lookahead.token==SYN_FALSE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: FB-->FALSE\n");
		#endif
		match(SYN_FALSE);
		return(run_status==1 ? 0 : 0);
	}
	else if (lookahead.token==SYN_ID || lookahead.token==SYN_NUM || lookahead.token==SYN_PAREN_L)
	{
		val1=Prod_E();
		if (run_status==1) ival1=cast2int(val1);
		if (lookahead.token==SYN_LT)
		{	
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E<E\n");
			#endif
			match(SYN_LT);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1<ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_LE)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E<=E\n");
			#endif
			match(SYN_LE);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1<=ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_GT)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E>E\n");
			#endif
			match(SYN_GT);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1>ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_GE)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E>=E\n");
			#endif
			match(SYN_GE);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1>=ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_EQ)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E==E\n");
			#endif
			match(SYN_EQ);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1==ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else if (lookahead.token==SYN_NE)
		{
			#if defined(AnaTypeSyn)
			printf("SYN: FB-->E!=E\n");
			#endif
			match(SYN_NE);
			val2=Prod_E();
			if (run_status==1)
			{	ival2=cast2int(val2);
				return(ival1!=ival2 ? 1 : 0);
			}
			else
				return(0);
		}
		else
		{
			if (run_status==1)
				return(ival1!=0 ? 1 : 0);
			else
				return(0);
		}

	}
	else
	{	FreeExit();
		return(0);
	}
}

static EXPVAL Prod_E()
{
	EXPVAL val1,val2;
	#if defined(AnaTypeSyn)
	printf("SYN: E-->TE E1\n");
	#endif
	val1=Prod_TE();
	val2=Prod_E1(val1);
	return(val2);
}

static EXPVAL Prod_E1(EXPVAL val1)
{
	EXPVAL val2,val;
	int i1,i2;
	char c1,c2;
	if (lookahead.token==SYN_ADD)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1-->+TE E1\n");
		#endif
		match(SYN_ADD);
		val2=Prod_TE();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1+i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1+c2;
			}
		val=Prod_E1(val);
	}
	else if (lookahead.token==SYN_SUB)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1-->-TE E1\n");
		#endif
		match(SYN_SUB);
		val2=Prod_TE();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1-i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1-c2;
			}
		val=Prod_E1(val);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: E1--> \n");
		#endif
		val=val1;
	}
	return(val);
}

static EXPVAL Prod_TE()
{
	EXPVAL val1,val2;
	#if defined(AnaTypeSyn)
	printf("SYN: TE-->F TE1\n");
	#endif
	val1=Prod_F();
	val2=Prod_TE1(val1);
	return(val2);
}

static EXPVAL Prod_TE1(EXPVAL val1)
{
	EXPVAL val2,val;
	int i1,i2;
	char c1,c2;
	if (lookahead.token==SYN_MUL)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TE1-->*F TE1\n");
		#endif
		match(SYN_MUL);
		val2=Prod_F();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1*i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1*c2;
			}
		val=Prod_TE1(val);
	}
	else if (lookahead.token==SYN_DIV)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TE1-->/F TE1\n");
		#endif
		match(SYN_DIV);
		val2=Prod_F();
		if (run_status==1)
			if (val1.type==ID_INT || val2.type==ID_INT)
			{
				val.type=ID_INT;
				i1=cast2int(val1);
				i2=cast2int(val2);
				val.val.intval=i1/i2;
			}
			else
			{
				val.type=ID_CHAR;
				c1=cast2char(val1);
				c2=cast2char(val2);
				val.val.charval=c1/c2;
			}
		val=Prod_TE1(val);
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: TE1--> \n");
		#endif
		val=val1;
	}
	return(val);
}

static EXPVAL Prod_F()
{
	EXPVAL val;
	static IDTABLE *p;
	if (lookahead.token==SYN_NUM)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->num\n");
		#endif
		match(SYN_NUM);
		val.type=ID_INT;
		val.val.intval=curtoken_num;
	}
	else if (lookahead.token==SYN_ID)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->id\n");
		#endif
		match(SYN_ID);
		p=LookupID();
		val.type=p->type;
		val.val=p->val;
	}
	else if (lookahead.token==SYN_PAREN_L)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: F-->(B)\n");
		#endif
		match(SYN_PAREN_L);
		val=Prod_E();
		match(SYN_PAREN_R);
	}
	else
		FreeExit();
	return(val);
}

```

constvar.h

```c
//#define AnaTypeLex	1
//#define AnaTypeSyn	1
#define MAXTOKENLEN	256

typedef union {
	int number;
	char *str;
} TOKENVAL;

typedef struct {
	int token;
	TOKENVAL tokenVal;
} TERMINAL;

typedef union {
	int intval;
	char charval;
} EXPVALUE;

typedef struct expValue{
	int type;
	EXPVALUE val;
} EXPVAL;

typedef struct idNode{
	char name[MAXTOKENLEN];
	int type;
	EXPVALUE val;
	struct idNode *next;
} IDTABLE;

//词法分析DFA转换表
static int LexTable[6][8]=
   {{   1, 201, 204,   2,   3,   4,   5, 205},
	{ 101, 101, 101, 101, 101, 101, 101, 101},
	{ 102, 102, 202, 203, 102, 102, 102, 102},
	{ 103, 103, 103, 103, 103,   4, 103, 103},
	{ 104, 104, 104, 104, 104,   4, 104, 104},
	{ 105, 105, 105, 105, 105,   5,   5, 105}};


//用于词法分析输出，及语法分析
#define ERR			-1
#define SYN_NUM		1		// int整数
#define SYN_ID		2		// id
#define SYN_LT		11		// <
#define SYN_GT		12		// >
#define SYN_LE		13		// <=
#define SYN_GE		14		// >=
#define SYN_EQ		15		// ==
#define SYN_NE		16		// !=
#define SYN_ADD		17		// +
#define SYN_SUB		18		// -
#define SYN_MUL		19		// *
#define SYN_DIV		20		// /
#define SYN_PAREN_L	21		// (
#define SYN_PAREN_R	22		// )
#define SYN_BRACE_L	23		// {
#define SYN_BRACE_R	24		// }
#define SYN_COMMA	25		// ,
#define SYN_SEMIC	26		// ;
#define SYN_SET		27		// =
#define SYN_AND		51		// &&
#define SYN_OR		52		// ||
#define SYN_NOT		53		// !
#define SYN_TRUE	54		// TRUE
#define SYN_FALSE	55		// FALSE
#define SYN_INT		56		// int
#define SYN_CHAR	57		// char
#define SYN_IF		58		// if
#define SYN_ELSE	59		// else
#define SYN_WHILE	60		// while
#define SYN_SHOW	61		// show

//用于符号表中类型
#define ID_FUN		1		// 函数类型
#define ID_INT		2		// int类型
#define ID_CHAR		3		// char类型

```

TinyC.c

```c
#include "stdio.h"

extern void SyntaxAnalysis();

FILE *sFile;
char name[12];

int main(int argc, char* argv[])
{
	scanf("%s",name);
    sFile=fopen( name,"rt");
	SyntaxAnalysis();
	fclose(sFile);

//Free all memories

	return 0;
}

```

测试集

```
main()
{ int i;
  i=1; 
  show(i);
}

main()
{ int i=2;
  i=4+i/2-1; 
  show(i);
}

main()
{ int i,j;
  i=2*2+1;
  j=3; 
  show(j*2-i);
}
```

发现是到接收状态103的状态转化表有问题

> DFA转换表中各状态含义：
>
> - 0: reloop
> - 1: delim
> - 2: 乘法
> - 3: 除法
> - 4: 加减
> - 5: 数字
> - 6: letter_
> - 7: symbol
>
> 10*表示接收状态，但多读了一个有用符号，需要缓存：
>
> - 101: 逻辑符号reloop
> - 102: 除法
> - 103: +和-
> - 104: 数字
> - 105: symbol
>
> 20*表示接收状态，没有多读有用符号：
>
> - 201: 文件末尾
> - 202: 注释
> - 203: 换行
> - 204: 乘法
> - 205: 括号

修改前

```
static int LexTable[6][8]=
   {{   1, 201, 204,   2,   3,   4,   5, 205},
	{ 101, 101, 101, 101, 101, 101, 101, 101},
	{ 102, 102, 202, 203, 102, 102, 102, 102},
	{ 103, 103, 103, 103, 103,   4, 103, 103},
	{ 104, 104, 104, 104, 104,   4, 104, 104},
	{ 105, 105, 105, 105, 105,   5,   5, 105}};
```

修改后

```
//词法分析DFA转换表
static int LexTable[6][8]=
   {{   1, 201, 204,   2,   3,   4,   5, 205},
	{ 101, 101, 101, 101, 101, 101, 101, 101},
	{ 102, 102, 202, 203, 102, 102, 102, 102},
	{ 103, 103, 103, 103, 103, 103, 103, 103},
	{ 104, 104, 104, 104, 104,   4, 104, 104},
	{ 105, 105, 105, 105, 105,   5,   5, 105}};
```

## 关卡三 完善char处理

> ## TinyC词法
>
> 1. `letter_->a..z | A..Z | _`
> 2. `digit    -> 0..9`
> 3. `reloop-> < | = | > | ! | & | ‘|’`
> 4. `calop-> + | - | * | /`
> 5. `id    ->letter_(letter_ | digit)*`
> 6. `number->digit digit*`
> 7. `delim->blank | tab | newline`
> 8. `ws->  /* | */ | //`
> 9. `symbol->( | ) | , | ; | { | } `
>
> 其中赋值号`=`、非运算符`!`被归入关系符中分析，然后在细分关系符时再筛选出来。关键字归入`id`中分析，然后再通过判断筛选出来。
>
> ## 实验环境
>
> ### 编程语言
>
> 标准C语言
>
> ### 实验文件
>
> #### 可编辑文件
>
> ```
> LexicalAnalysis.c`，`SyntaxAnalysis.c`，`constvar.h`，`test.txt
> ```
>
> #### 不可编辑文件
>
> ```
> Tiny_C.c`，`1.txt`，`2.txt
> ```
>
> ## 实验内容
>
> 程序中，虽然在语义分析部分包含了`char`型数据的处理，但词法分析和语法分析并不完整，请完善代码，使之支持`char`型数据。
>
> ### 注意
>
> - 其余关卡的代码修改不会影响本关卡，你可能需要将部分代码复制过来。
> - `test.txt`和`1.txt`初始内容相同，而`test.txt`开放了编辑权限，方便大家测试代码。
> - 测试时可以修改`constvar.h`中的相关宏定义方便观察代码运行情况，但最终提交时需要将其注释。
> - 如果`1.txt`和`2.txt`的验收成功完成，将`test.txt`中的内容改回默认即可完整完成本次实践的验收。

```
//1.txt
main()
{ char ch='A';
  show(ch);
}

//2.txt
main()
{ char ch;
  ch='\\';
  show(ch);
}

//test.txt
main()
{ char ch='A';
  show(ch);
}
```

## 关卡四 逻辑表达式运算

> ## 实验内容
>
> 在表达式计算中，允许**逻辑表达式**参与算术表达式计算，例如求` a+(b>1)`的值。
>
> ### 注意
>
> - 其余关卡的代码修改不会影响本关卡，你可能需要将部分代码复制过来。
> - `test.txt`和`1.txt`初始内容相同，而`test.txt`开放了编辑权限，方便大家测试代码。
> - 测试时可以修改`constvar.h`中的相关宏定义方便观察代码运行情况，但最终提交时需要将其注释。
> - 如果`1.txt`和`2.txt`的验收成功完成，将`test.txt`中的内容改回默认即可完整完成本次实践的验收。

```
//1.txt
main()
{ int a,b;
  b=1;
  a=3-(b>=1);
  show(a);
}

//2.txt
main()
{ int a,b;
  b=5;
  a=(3*b>11)+(b>9||b<=6);
  show(a);
}

```

需要修改文法`F-->(E)`为`F-->id | num | char | (B)`

## 关卡五 添加continue语句

>  实验内容
>
> ### Step1
>
> 测试原有的`while`语句能否正常运行，修改你发现的错误。
>
> ### Step2
>
> 为Tiny C添加循环控制语句`continue`。
>
> ### 注意
>
> - 其余关卡的代码修改不会影响本关卡，你可能需要将部分代码复制过来。
> - `test.txt`和`1.txt`初始内容相同，而`test.txt`开放了编辑权限，方便大家测试代码。
> - 测试时可以修改`constvar.h`中的相关宏定义方便观察代码运行情况，但最终提交时需要将其注释。
> - 如果`1.txt`和`2.txt`的验收成功完成，将`test.txt`中的内容改回默认即可完整完成本次实践的验收。

```
//1.txt 19
main()
{ int i=1,j=1;
  while(i<6){
     j=j+i;
     if(j==7){
       continue;
        }
    i=i+1;
  }
  show(j);
}

//2.txt 26
main()
{ int i=1,j=1;
  while(i<6&&j<25){
     j=j+i;
     if(i==4||j==7){
       continue;
        }
    i=i+1;
  }
  show(j);
}

```

添加文法`S-->continue;`

果然，if功能也是错的；修改`Prod_S()`中SYN_IF项，用局部变量存储if语句前run_status的状态

对于`2.txt`，`&&`运算也是错的，修改 `bval2=Prod_FB();`
