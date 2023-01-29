// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' '||ch == '\t')
		getch();

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else {
			if (ch == '[')
				sym = SYM_ARRAY;		// symbol is an array
			else
				sym = SYM_IDENTIFIER;   // symbol is an identifier
		}
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		}
		while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == '\n')
	{
		getch();
		getsym();
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_NULL;       // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else if (ch == ']') {		// ��ʾ�������������
		getch();
		sym = SYM_RSPAREN;
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
    if(order_adj == 0)
	{
        code[cx].f = x;
	    code[cx].l = y;
	    code[cx++].a = z;
    }
	else
    {
        initial_code[ox].f = x;
	    initial_code[ox].l = y;
	    initial_code[ox++].a = z;
    }

} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;

	if (! inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while(! inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index

// enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask* mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask*) &table[tx];
		mk->level = level;
		break;
	} // switch
} // enter


    /*
   *������������������ķ��ű���
   */
void array_enter()
{
	int i; 
	ax++;
	array_table[ax] = array_t;
	strcpy(array_table[ax].name, id);
	enter(ID_VARIABLE);
	array_table[ax].addr = tx;							//��¼�����ʼƫ�Ƶ�ַ
	for (i = array_table[ax].sum - 1; i > 0; i--)	//����������д洢�ռ�������ű���
		enter(ID_VARIABLE);
} // array_enter


//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

 /*
  *��λ��ʶ����������ű��е�λ��
  */
int array_position()
{
	int i = 0;
	while (strcmp(array_table[++i].name, id));
	if (i <= ax)
		return i;
	else
		return 0;
} // array_position

/*
    Array��ʼ���ķ����£�
    S -> { L } | num | epsilon
    L -> ST
    T -> ,ST | epsilon
	���������ʷ�:([]|epsilon)([num])*
*/
void Array_S(int position, int dim, int count, int offset_begin)	//dim�ǵ�ǰά��,count�ǵ�ǰ�����������ָ���
{
    void Array_L(int position, int dim, int count, int offset_begin);
    void expression(symset fsys);
    int j;
	int max_count = 1;
    symset set;
    mask *mk;

    if(sym == SYM_LBRACE)
    {
        getsym();
        Array_L(position, dim + 1, 0, offset);
        getsym();
    }  
    else if(sym == SYM_NUMBER)
    {
		{
			if(array_table[position].dim[dim - 1] < count && backfill == 0)
			{
				error(29);
			}
			else if(dim > 1 && array_table[position].size[dim - 2] - offset_begin % array_table[position].size[dim - 2] < count + 1 && backfill == 0)
			{
				error(29);
			}			
			else if(dim > 1 && array_table[position].size[dim - 2] - offset_begin % array_table[position].size[dim - 2] < count + 1 && backfill == 1)
			{
				error(29);
			}
			else
			{
				order_adj = 1;
				gen(LIT, 0, offset++);
				
				set = createset(SYM_RBRACE, SYM_COMMA, SYM_NULL);
				expression(set);
				destroyset(set);
				j = array_table[position].addr;
				mk = (mask*)&table[j];
				if (j)
					gen(STA, level - mk->level, mk->address);		//�洢����ֵ���ض�����ض�ƫ�Ƶ�ַ
				
				brace_num = 0;
				order_adj = 0;
			}
		}
    }
	else if(sym == SYM_RBRACE)		//,ȱʡ
	{
		brace_num = count;
		return ;
	}
    else
    {
        error(29);      
    }
}

void Array_L(int position, int dim, int count, int offset_begin)
{
    void Array_T(int position, int dim, int count, int offset_begin);

    if(sym == SYM_LBRACE || sym == SYM_NUMBER)
    {
        Array_S(position, dim, count, offset_begin);
        Array_T(position, dim, count, offset_begin);
    }
    else
    {
        error(29);      
    }
}

void Array_T(int position, int dim, int count, int offset_begin)
{

    if(sym == SYM_COMMA)
    {
		int i = 0;
        getsym();
        Array_S(position, dim, count + 1, offset_begin);
		//���������offset���ж����ӣ����{x}��array_table[position].size�еĵ�һ��,��ռ�������ռ�
		if(brace_num != array_table[position].size[dim - 1] && brace_num != 0)
		{
			while((offset - 1) % array_table[position].size[i++] != 0);
			offset = offset - 1 + array_table[position].size[--i];
		}
        Array_T(position, dim, count + 1, offset_begin);
    }
    else if(sym == SYM_RBRACE)
    {
		brace_num = count + 1;
        return ;
    }
    else
    {
        error(29);  
    }
}

//////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	} else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

//////////////////////////////////////////////////////////////////////
void vardeclaration()
{
	void expression(symset fsys);
	symset set;
	int i, j, dim = 0;
	mask *mk;
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);
		i = tx;
		getsym();
		if(sym == SYM_EQU)
		{
			getsym();
			order_adj = 1;
			set = createset(SYM_SEMICOLON, SYM_COMMA, SYM_NULL);
			expression(set);
			destroyset(set);
			mk = (mask*) &table[i];
			if (i)
			{
				gen(STO, level - mk->level, mk->address);
			}		
			else
			{
				error(2);			
			}
			order_adj = 0;
		}
	}
	else if (sym == SYM_ARRAY) {			//����������Ϊ����	
        if(ch == '[')
        {
			dim++;
			getch();						//��ȡ'['�����һ���ַ�
            if(ch == ']')
            {
                backfill = 1;
                getsym();
            }
			else
            {
                getsym();						//��ȡ'['��']'֮������֣���ֵnumΪ��ǰά��������ռ�

			    array_t.dim[dim - 1] = num;		//������������ά�������ݴ�С
			    getsym();						//��ȡ']'�����һ���ַ�
            }
        }
		while(ch == '[')
		{
			dim++;
			getch();						//��ȡ'['�����һ���ַ�
			getsym();						//��ȡ'['��']'֮������֣���ֵnumΪ��ǰά��������ռ�

			array_t.dim[dim - 1] = num;		//������������ά�������ݴ�С
			getsym();						//��ȡ']'�����һ���ַ�
		}
        
		array_t.n = dim;					//�������ά��
		array_t.size[dim - 1] = 1;			//������ƫ������СΪ1
		for (i = dim - 1; i > 0; i--)
			array_t.size[i - 1] = array_t.size[i] * array_t.dim[i];		//��������Ԫ�ص�ƫ�Ƶ�ַ
		array_t.sum = array_t.size[0] * array_t.dim[0];					//��������������Ԫ��ռ�ÿռ�
		array_enter();						//����������ű�
        i = ax;
		getsym();
        if(sym == SYM_EQU)
        {
            offset = 0;
            getsym();
            Array_S(i, 0, 0, 0); 
            if(backfill)
            {
                for(j = 1; j < array_t.n; j++)
                {
                    offset = offset / array_t.dim[j];
                }
                array_table[i].dim[0] = offset;
                array_table[i].sum = array_table[i].size[0] * array_table[i].dim[0];
                backfill = 0;
                for (j = array_table[i].sum - 1; j > 0; j--)	//����������д洢�ռ�������ű���
		            enter(ID_VARIABLE);
            }
			else
			{
				if(offset > array_table[i].sum)
				{
					error(29);
				}
			}
        }
	//	else
	//		enter(ID_VARIABLE);
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
void factor(symset fsys)
{
	void expression(symset fsys);
	int i, j;
	int dim = 0;	//���鵱ǰά��
	symset set;
	mask* mk;
	
	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					//mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
					mk = (mask*) &table[i];
					gen(LOD, level - mk->level, mk->address);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				} // switch
			}
			getsym();
		}
		else if (sym == SYM_ARRAY)					//����Ϊ��������
		{
			if (!(i = array_position()))			//��λ��������ڷ��ű��е�λ��
				error(11);
			else {
				j = array_table[i].addr;			//��¼�����׵�ַ
				mk = (mask*) &table[j];
				gen(LIT, 0, 0);						//ͨ���ۼ�����ƫ������ȷ��Ԫ��λ�ã���ʼΪ0
				while (ch == '[') {
					dim++;							//�����������ź�ά��+1��
					getch();
					getsym();						//��ȡ���������е���ֵ
					set = uniteset(createset(SYM_RSPAREN, SYM_NULL), fsys);
					expression(set);				//�����еı��ʽ
					destroyset(set);
					gen(LIT, 0, array_table[i].size[dim - 1]);		//ȡ��ǰά��ƫ�ƴ�С��ջ��
					gen(OPR, 0, OPR_MUL);			//��ά����ֵ���Ը�ά��ƫ�ƴ�С
					gen(OPR, 0, OPR_ADD);			//�ۼӵ���ƫ��
				}
				gen(LDA, level - mk->level, mk->address);			//���ɼ�������ָ���¼����ƫ��
			}
			getsym();
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if(sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{  
			 getsym();
			 factor(fsys);
			 gen(OPR, 0, OPR_NEG);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if
} // factor

//////////////////////////////////////////////////////////////////////
void term(symset fsys)
{
	int mulop;
	symset set;
	//fsys����������ŵ�symtypeֵ
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH)
	{
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else
		{
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
} // term

//////////////////////////////////////////////////////////////////////
void expression(symset fsys)
{
	int addop;
	symset set;
	//fsys����������ŵ�symtypeֵ
	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));
	
	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} // expression

//////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (! inset(sym, relset))
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition

void ex_condition(symset fsys) 
{
	int relop;
	condition_level++;
	true_count[condition_level] = 0; //��λ0
	false_count[condition_level] = 0;//��λ0
	if (sym == SYM_NOT) //������һ����NOT
	{
		getsym();
		condition(fsys);
		gen(OPR, 0, OPR_NOT);
	}
	else
	{
		condition(fsys);
	}
	
	if (sym == SYM_THEN||sym == SYM_SEMICOLON||sym == SYM_DO) //ֻ��һ������,��û��not and or���㡣
	{
		false_out[condition_level][false_count[condition_level]++] = cx;
		gen(JPC, 0, 0);   //����������
		true_out[condition_level][true_count[condition_level]++] = cx;
		gen(JMP, 0, 0);  //��������
		return;
	}
	else if (sym == SYM_AND || sym == SYM_OR || sym == SYM_NOT)
	{
		while (sym == SYM_AND || sym == SYM_OR || sym == SYM_NOT)
		{
			relop = sym;
			getsym();
			switch (relop)
			{
			case SYM_OR:
				true_out[condition_level][true_count[condition_level]++] = cx;
				gen(JPNC, 0, 0); //�������������true �ĳ���,�ȴ�����
				if (sym == SYM_NOT)  //or �ĺ�����not
				{
					getsym();
					condition(fsys);
					gen(OPR, 0, OPR_NOT); //����ֵȡ��
				}
				else
				{
					condition(fsys);
				}
				if (sym == SYM_THEN || sym == SYM_SEMICOLON)
				{
					false_out[condition_level][false_count[condition_level]++] = cx; //����������
					gen(JPC, 0, 0);
					true_out[condition_level][true_count[condition_level]++] = cx;
					gen(JMP, 0, 0);  //��������
					return;
				}
				break;

			case SYM_AND:
				false_out[condition_level][false_count[condition_level]++] = cx;
				gen(JPC, 0, 0);  //����false����
				if (sym == SYM_NOT)  //and ������ not
				{
					getsym();
					condition(fsys);
					gen(OPR, 0, OPR_NOT);
				}
				else
				{
					condition(fsys);
				}
				if (sym == SYM_THEN || sym == SYM_SEMICOLON)
				{
					false_out[condition_level][false_count[condition_level]++] = cx; //����������
					gen(JPC, 0, 0);
					true_out[condition_level][true_count[condition_level]++] = cx;
					gen(JMP, 0, 0);  //��������
					return;
				}
				break;

			/*case SYM_NOT:
				condition(fsys);
				gen(OPR, 0, OPR_NEG);
				if (sym == SYM_THEN)
				{
					false_out[false_count++] = cx;
					gen(JPC, 0, 0);
					return;
				}
				break;*/
			}

		}
		error(16);
	}
	else
	{
		error(16);
	}
}

//////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;
	//for array
	int dim = 0, j;
	mask* mk;
	int k;

	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		//mask* mk;
		if (! (i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_VARIABLE)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		getsym();
		if (sym == SYM_BECOMES)
		{
			getsym();
		}
		else
		{
			error(13); // ':=' expected.
		}
		expression(fsys);
		mk = (mask*) &table[i];
		if (i)
		{
			gen(STO, level - mk->level, mk->address);
		}
	}
	else if (sym == SYM_ARRAY)		//statment�����ʼΪ��������
	{
		if (!(i = array_position()))
			error(11);
		else {
			j = array_table[i].addr;
			mk = (mask*)&table[j];
			gen(LIT, 0, 0);
			while (ch == '[') {
				dim++;
				getch();
				getsym();
				set = uniteset(createset(SYM_RSPAREN, SYM_NULL), fsys);
				expression(set);
				destroyset(set);
				gen(LIT, 0, array_table[i].size[dim - 1]);
				gen(OPR, 0, OPR_MUL);
				gen(OPR, 0, OPR_ADD);
			}
		}
		getsym();
		if (sym == SYM_BECOMES)
			getsym();
		else
			error(13);		// ':=' expected.
		set = uniteset(createset(SYM_RSPAREN, SYM_NULL), fsys);
		expression(set);
		destroyset(set);
		j = array_table[i].addr;
		mk = (mask*)&table[j];
		if (j)
			gen(STA, level - mk->level, mk->address);		//�洢����ֵ���ض�����ض�ƫ�Ƶ�ַ
	}
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (! (i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*) &table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	} 
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].a = cx;	
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		//break_count++; //�Ƿ����break
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO,SYM_AND,SYM_NOT,SYM_OR ,SYM_NULL);
		set = uniteset(set1, fsys);
		//condition(set);
		ex_condition(set);
		destroyset(set1);
		destroyset(set);

		//cx2 = cx;
		//gen(JPC, 0, 0);
		if (sym == SYM_DO)  
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		/*��������ڶ�����while-statement��ǰ��*/
		for (k = 0; k < true_count[condition_level];k++)
			code[true_out[condition_level][k]].a = cx;

		statement(fsys);
		gen(JMP, 0, cx1); //ִ����ѭ������������ǰ���ж�
		
		/*���мٳ��ڶ�����while-statement ����*/
		for (k = 0;k < false_count[condition_level];k++) 
			code[false_out[condition_level][k]].a = cx;
		condition_level--; //��С�������

		//�������break���������ô�
		/*
		if (break_cx[break_count] > 0)
		{
			code[break_cx[break_count]].a = cx;
			break_count--;
		}
		*/
		//code[cx2].a = cx;
	}
	else if (sym == SYM_WRITE) {		//�����ӡ����
		getsym();
		if (sym != SYM_LPAREN)
			error(26);
		do {
			getsym();
			if (sym == SYM_RPAREN) {	//�����ȡ�������ţ���һ���ַ��������������ţ���ʾ�ù���Ϊ��ӡ�س�
				gen(OPR, 0, OPR_WEN);	//����OPR,0,14��ӡ�س�������break�˳�
				break;
			}
			else if (sym == SYM_IDENTIFIER) {	//��ӡ������Ϊconst���������
				if (!(i = position(id)))
					error(11);
			/*	else if (table[i].kind != ID_VARIABLE) {		//ע��ԭ���Ĵ�����ʾ��ʹ��print���̿����������ֵ
					error(12);
					i = 0;
				} */
				mk = (mask *)&table[i];
				if (table[i].kind == ID_CONSTANT) {				//���constant����ֵ
					gen(LIT, 0, table[i].value);
					gen(OPR, 0, OPR_WRITE);						//����OPR,0,15��ӡ����ֵ
				}
				else if (i)										//��Ϊ��������WRITEָ��
					gen(WRITE, level - mk->level, mk->address);	
			}
			else if (sym == SYM_ARRAY) {						//��Ϊ����
				dim = 0;
				if (!(i = array_position()))
					error(11);
				else {
					j = array_table[i].addr;
					mk = (mask *)&table[j];
					gen(LIT, 0, 0);
					while (ch == '[') {
						dim++;
						getch();
						getsym();
						set = uniteset(createset(SYM_RSPAREN, SYM_NULL), fsys);
						expression(set);
						destroyset(set);
						gen(LIT, 0, array_table[i].size[dim - 1]);
						gen(OPR, 0, OPR_MUL);
						gen(OPR, 0, OPR_ADD);
					}
					gen(WTA, level - mk->level, mk->address);	//����WTAָ���ӡ���ڲ㼰ƫ�Ƶ�ַ������ֵ
				}
			}
			else if (sym == SYM_NUMBER) {						//��������֣�ֱ��ȡ����ջ��
				gen(LIT, 0, num);
				gen(OPR, 0, OPR_WRITE);							//����OPR,0,15ָ���ӡ����
			}
			getsym();
		} while (sym == SYM_COMMA);								//���м�Ϊ���ţ��������ӡ��һ����
		if (sym == SYM_RPAREN)
			getsym();
		else
			error(22);
	}
	test(fsys, phi, 19);
} // statement


void Initialization(int ox0, int block_ox)      //��ʼ��������д�����ٿռ�֮��
{
	int i;
	if(ox0 == block_ox)
		return ;
	else
	{
        for(i = ox0; i < block_ox; i++)
        {
            gen(initial_code[i].f, initial_code[i].l, initial_code[i].a);
        }
	}
}
			
//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	int tx0; // initial table index
	int dx1; // save data allocation index
    int ox0;
	mask* mk;
	int block_dx;
	int block_ox;
	int savedTx;
	symset set1, set;

	dx = 3;
	tx0 = tx;
	block_dx = dx;
	mk = (mask*) &table[tx];
	mk->address = cx;
	gen(JMP, 0, 0);
    ox0 = ox;
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if
		block_dx = dx; //save dx before handling procedure call!
        block_ox = ox;
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}


			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tx;
			dx1 = dx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			dx = dx1;
			level--;

			if (sym == SYM_SEMICOLON)
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, declbegsys));

	code[mk->address].a = cx;//(level == 0)? cx: cx - ox;
	mk->address = cx;
	cx0 = cx;

	gen(INT, 0, block_dx);
    Initialization(ox0, block_ox);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register
	
	int input, output;
	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_NOT:
				stack[top] = !stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			case OPR_WEN:		//��ӡ�س�
				printf("\n");
				break;
			case OPR_WRITE:
				printf("%d\t", stack[top]);		//��ӡ���ֻ���������Ʊ��
				top--;
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			//printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		case JPNC:
			if (stack[top] > 0)
				pc = i.a;
			top--;
			break;
		case READ:					//��ȡ����
			scanf("%d", &input);
			stack[base(stack, b, i.l) + i.a] = input;
			break;
		case WRITE:					//��ӡ����
			printf("%d\t", stack[base(stack, b, i.l) + i.a]);
			break;
		case LDA:					//��������
			stack[top] = stack[base(stack, b, i.l) + i.a + stack[top]];
			break;
		case STA:					//�洢����
			stack[base(stack, b, i.l) + i.a + stack[top - 1]] = stack[top];
			//printf("%d\n", stack[top]);
			top--;
			break;
		case WTA:					//��ӡ����ֵ������Ʊ��
			printf("%d\t", stack[base(stack, b, i.l) + i.a + stack[top]]);
			break;
		case RDA:					//��ȡ�������ֵ
			scanf("%d", &input);
			stack[base(stack, b, i.l) + i.a + stack[top]] = input;
			break;
		default:
			fprintf(stderr, "Runtime error: unexpected instruction.\n");
		} // switch
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
int main ()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	
	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_ARRAY, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
	getchar();
	getchar();
	return 0;
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
