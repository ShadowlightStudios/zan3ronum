%{

/* $Id: parser.y,v 1.20 2006/04/16 15:15:46 helly Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "globals.h"
#include "parser.h"
#include "basics.h"

#define YYMALLOC malloc
#define YYFREE free

using namespace re2c;

extern "C"
{
int yylex();
void yyerror(const char*);
}

static re2c::uint accept;
static RegExp *spec;
static Scanner *in = NULL;

/* Bison version 1.875 emits a definition that is not working
 * with several g++ version. Hence we disable it here.
 */
#if defined(__GNUC__)
#define __attribute__(x)
#endif

/* strdup() isn't standard C, so if we don't have it, we'll create our
 * own version
 */
#if !defined(HAVE_STRDUP)
static char* strdup(const char* s)
{
	char* rv = (char*)malloc(strlen(s) + 1);
	if (rv == NULL)
		return NULL;
	strcpy(rv, s);
	return rv;
}
#endif

%}

%start	spec

%union {
    re2c::Symbol	*symbol;
    re2c::RegExp	*regexp;
    re2c::Token 	*token;
    char        	op;
    int         	number;
    re2c::ExtOp 	extop;
    re2c::Str   	*str;
};

%token		CLOSESIZE   CLOSE	ID	CODE	RANGE	STRING
%token		CONFIG	VALUE	NUMBER

%type	<op>		CLOSE
%type	<op>		close
%type	<extop>		CLOSESIZE
%type	<symbol>	ID
%type	<token>		CODE
%type	<regexp>	RANGE	STRING
%type	<regexp>	rule	look	expr	diff	term	factor	primary
%type	<str>		CONFIG	VALUE
%type	<number>	NUMBER

%%

spec	:
		{ accept = 0;
		  spec = NULL; }
	|	spec rule
		{ spec = spec? mkAlt(spec, $2) : $2; }
	|	spec decl
	;

decl	:	ID '=' expr ';'
		{ if($1->re)
		      in->fatal("sym already defined");
		  $1->re = $3; }
	|	ID '=' expr '/'
		{ in->fatal("trailing contexts are not allowed in named definitions"); }
	|	CONFIG '=' VALUE ';'
		{ in->config(*$1, *$3); delete $1; delete $3; }
	|	CONFIG '=' NUMBER ';'
		{ in->config(*$1, $3); delete $1; }
	;

rule	:	expr look CODE
		{ $$ = new RuleOp($1, $2, $3, accept++); }
	;

look	:
		{ $$ = new NullOp; }
	|	'/' expr
		{ $$ = $2; }
	;

expr	:	diff
		{ $$ = $1; }
	|	expr '|' diff
		{ $$ =  mkAlt($1, $3); }
	;

diff	:	term
		{ $$ = $1; }
	|	diff '\\' term
		{ $$ =  mkDiff($1, $3);
		  if(!$$)
		       in->fatal("can only difference char sets");
		}
	;

term	:	factor
		{ $$ = $1; }
	|	term factor
		{ $$ = new CatOp($1, $2); }
	;

factor	:	primary
		{ $$ = $1; }
	|	primary close
		{
		    switch($2){
		    case '*':
			$$ = mkAlt(new CloseOp($1), new NullOp());
			break;
		    case '+':
			$$ = new CloseOp($1);
			break;
		    case '?':
			$$ = mkAlt($1, new NullOp());
			break;
		    }
		}
	|	primary CLOSESIZE
		{
			$$ = new CloseVOp($1, $2.minsize, $2.maxsize);
		}
	;

close	:	CLOSE
		{ $$ = $1; }
	|	close CLOSE
		{ $$ = ($1 == $2) ? $1 : '*'; }
	;

primary	:	ID
		{ if(!$1->re)
		      in->fatal("can't find symbol");
		  $$ = $1->re; }
	|	RANGE
		{ $$ = $1; }
	|	STRING
		{ $$ = $1; }
	|	'(' expr ')'
		{ $$ = $2; }
	;

%%

extern "C" {
void yyerror(const char* s)
{
    in->fatal(s);
}

int yylex(){
    return in ? in->scan() : 0;
}
} // end extern "C"

namespace re2c
{

void parse(Scanner& i, std::ostream& o)
{
	in = &i;

	o << "/* Generated by re2c " PACKAGE_VERSION;
	if (!bNoGenerationDate)
	{
		o << " on ";
		time_t now = time(&now);
		o.write(ctime(&now), 24);
	}
	o << " */\n";
	o << sourceFileInfo;
	
	while(i.echo())
	{
		yyparse();
		if(spec)
		{
			genCode(o, topIndent, spec);
		}
		o << sourceFileInfo;
	}

	RegExp::vFreeList.clear();
	Range::vFreeList.clear();
	Symbol::ClearTable();
	in = NULL;
}

} // end namespace re2c
