//**************************************************************************
//**
//** token.h
//**
//**************************************************************************

// UM 1999-12-05
//  - modification for 'for' statement

#ifndef __TOKEN_H__
#define __TOKEN_H__

// HEADER FILES ------------------------------------------------------------

#include "common.h"
#include "error.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

typedef enum
{
	TK_NONE,
	TK_EOF,
	TK_IDENTIFIER,		// VALUE: (char *) tk_String
	TK_STRING,		// VALUE: (char *) tk_String
	TK_NUMBER,		// VALUE: (int) tk_Number
	TK_LINESPECIAL,		// VALUE: (int) tk_LineSpecial
	TK_PLUS,		// '+'
	TK_MINUS,		// '-'
	TK_ASTERISK,		// '*'
	TK_SLASH,		// '/'
	TK_PERCENT,		// '%'
	TK_ASSIGN,		// '='
	TK_ADDASSIGN,		// '+='
	TK_SUBASSIGN,		// '-='
	TK_MULASSIGN,		// '*='
	TK_DIVASSIGN,		// '/='
	TK_MODASSIGN,		// '%='
	TK_INC,			// '++'
	TK_DEC,			// '--'
	TK_EQ,			// '=='
	TK_NE,			// '!='
	TK_LT,			// '<'
	TK_GT,			// '>'
	TK_LE,			// '<='
	TK_GE,			// '>='
	TK_LSHIFT,		// '<<'
	TK_RSHIFT,		// '>>'
	TK_ANDLOGICAL,		// '&&'
	TK_ORLOGICAL,		// '||'
	TK_ANDBITWISE,		// '&'
	TK_ORBITWISE,		// '|'
	TK_EORBITWISE,		// '^'
	TK_TILDE,		// '~'
	TK_LPAREN,		// '('
	TK_RPAREN,		// ')'
	TK_LBRACE,		// '{'
	TK_RBRACE,		// '}'
	TK_LBRACKET,		// '['
	TK_RBRACKET,		// ']'
	TK_COLON,		// ':'
	TK_SEMICOLON,		// ';'
	TK_COMMA,		// ','
	TK_PERIOD,		// '.'
	TK_NOT,			// '!'
	TK_NUMBERSIGN,		// '#'
	TK_CPPCOMMENT,		// '//'
	TK_STARTCOMMENT,	// '/*'
	TK_ENDCOMMENT,		// '*/'
	TK_BREAK,		// 'break'
	TK_CASE,		// 'case'
	TK_CONST,		// 'const'
	TK_CONTINUE,		// 'continue'
	TK_DEFAULT,		// 'default'
	TK_DEFINE,		// 'define'
	TK_DO,			// 'do'
	TK_ELSE,		// 'else'
	TK_FOR,			// 'for'
	TK_GOTO,		// 'goto'
	TK_IF,			// 'if'
	TK_INCLUDE,		// 'include'
	TK_INT,			// 'int'
	TK_OPEN,		// 'open'
	TK_PRINT,		// 'print'
	TK_PRINTBOLD,		// 'printbold'
	TK_RESTART,		// 'restart'
	TK_SCRIPT,		// 'script'
	TK_SPECIAL,		// 'special'
	TK_STR,			// 'str'
	TK_SUSPEND,		// 'suspend'
	TK_SWITCH,		// 'switch'
	TK_TERMINATE,		// 'terminate'
	TK_UNTIL,		// 'until'
	TK_VOID,		// 'void'
	TK_WHILE,		// 'while'
	TK_WORLD		// 'world'
} tokenType_t;

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void TK_Init(void);
void TK_OpenSource(char *fileName);
void TK_Include(char *fileName);
void TK_CloseSource(void);
void SetIncludePath(char *name);
tokenType_t TK_NextToken(void);
int TK_NextCharacter(void);
void TK_NextTokenMustBe(tokenType_t token, error_t error);
void TK_TokenMustBe(tokenType_t token, error_t error);
boolean TK_Member(tokenType_t *list);
void TK_Undo(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern tokenType_t tk_Token;
extern int tk_Line;
extern int tk_Number;
extern char *tk_String;
extern int tk_SpecialValue;
extern int tk_SpecialArgCount;
extern char tk_SourceName[MAX_FILE_NAME_LENGTH];
extern int tk_IncludedLines;
extern boolean forNeedsSemicolon;

#endif
