/*
Plantilla guiada para el scanner de Flex.

Objetivo para el alumnado:
1) Completar cada bloque en este mismo orden.
2) Mantener los nombres de macros y tokens tal cual se indican.
3) Al finalizar, el archivo debe comportarse como el scanner final esperado.
*/

/*
SECCIÓN 1: Opciones globales de Flex
- Agrega la opción para NO requerir yywrap.
- Agrega la opción para llevar conteo automático de líneas.

Pista: la línea esperada usa dos opciones en una sola directiva %option.
*/

/*
SECCIÓN 2: Código C embebido
- Abre el bloque %{ ... %}.
- Incluye el header del scanner con #include "scanner.h".
- Cierra el bloque.
*/

/*
SECCIÓN 3: Estados léxicos
- Declara un estado exclusivo llamado COMMENT para comentarios de bloque.
*/

/*
SECCIÓN 4: Macros (definiciones de regex reutilizables)
- ID: identificadores tipo C (empieza con letra o _, luego letras/dígitos/_).
- INT: uno o más dígitos.
- FLOAT: número con punto opcionalmente con exponente, o entero con exponente.
- WS: espacios en blanco (espacio, tab, retorno de carro y salto de línea).
*/

/*
SECCIÓN 5: Inicio de reglas
- Coloca el separador %% para comenzar reglas.
*/

/*
SECCIÓN 6: Comentarios
- Regla para iniciar comentario de bloque al leer "/*" (cambiar a estado COMMENT).
- En COMMENT, regla para "*/" (volver a INITIAL).
- En COMMENT, consumir \n sin acción (el conteo de línea se actualiza solo).
- En COMMENT, consumir cualquier otro carácter y descartarlo.
- En COMMENT, si llega EOF sin cerrar, devolver TOK_ERROR.
- Regla para comentario de línea: "//" hasta antes de \n, sin devolver token.
*/

/*
SECCIÓN 7: Espacios en blanco
- Regla usando {WS} para ignorar whitespace.
*/

/*
SECCIÓN 8: Palabras reservadas
- Agrega reglas exactas para:
	int, float, double, char, void,
	if, else, while, for, return, break, continue
- Cada una debe retornar su token TOK_KW_* correspondiente.
*/

/*
SECCIÓN 9: Operadores compuestos (siempre antes de los simples)
- ++, --, +=, -=, *=, /=, %=, ==, !=, <=, >=, &&, ||
- Devuelven los TOK_* correspondientes.
*/

/*
SECCIÓN 10: Operadores simples
- =, <, >, !, +, -, *, /, %
- Devuelven los TOK_* correspondientes.
*/

/*
SECCIÓN 11: Delimitadores
- (, ), {, }, [, ], ,, ;
- Devuelven los TOK_* correspondientes.
*/

/*
SECCIÓN 12: Literales e identificadores (orden importante)
- Primero FLOAT -> TOK_FLOAT_LITERAL.
- Luego INT -> TOK_INT_LITERAL.
- Luego literal de char (admitiendo escapes) -> TOK_CHAR_LITERAL.
- Luego literal de string (admitiendo escapes) -> TOK_STRING_LITERAL.
- Finalmente ID -> TOK_IDENTIFIER.

Nota: si pones ID antes de keywords, las keywords se reconocerán como identificadores.
*/

/*
SECCIÓN 13: Fin de archivo y error genérico
- Regla <<EOF>> que retorna TOK_EOF.
- Regla final con . que retorna TOK_ERROR para cualquier carácter no reconocido.
*/

/*
SECCIÓN 14: Cierre de reglas
- Coloca el separador %% final.
*/
```

%option noyywrap yylineno



%{

#include "scanner.h"

%}



%x COMMENT



ID    [a-zA-Z_][a-zA-Z_0-9]*

INT   [0-9]+

FLOAT ({INT}\.{INT})|({INT}(\.{INT})?[eE][+-]?{INT})

WS    [ \t\r\n]+



%%



"/*"              BEGIN(COMMENT);

<COMMENT>"*/"     BEGIN(INITIAL);

<COMMENT>.        ;

<COMMENT>\n       ;

<COMMENT><<EOF>>  { return TOK_ERROR; }

"//"[^\n]*        ;



{WS}              ;



"int"             { return TOK_KW_INT; }

"float"           { return TOK_KW_FLOAT; }

"double"          { return TOK_KW_DOUBLE; }

"char"            { return TOK_KW_CHAR; }

"void"            { return TOK_KW_VOID; }

"if"              { return TOK_KW_IF; }

"else"            { return TOK_KW_ELSE; }

"while"           { return TOK_KW_WHILE; }

"for"             { return TOK_KW_FOR; }

"return"          { return TOK_KW_RETURN; }

"break"           { return TOK_KW_BREAK; }

"continue"        { return TOK_KW_CONTINUE; }



"++"              { return TOK_INC; }

"--"              { return TOK_DEC; }

"+="              { return TOK_PLUS_ASSIGN; }

"-="              { return TOK_MINUS_ASSIGN; }

"*="              { return TOK_MUL_ASSIGN; }

"/="              { return TOK_DIV_ASSIGN; }

"%="              { return TOK_MOD_ASSIGN; }

"=="              { return TOK_EQ; }

"!="              { return TOK_NEQ; }

"<="              { return TOK_LE; }

">="              { return TOK_GE; }

"&&"              { return TOK_AND; }

"||"              { return TOK_OR; }



"="               { return TOK_ASSIGN; }

"<"               { return TOK_LT; }

">"               { return TOK_GT; }

"!"               { return TOK_NOT; }

"+"               { return TOK_PLUS; }

"-"               { return TOK_MINUS; }

"*"               { return TOK_MUL; }

"/"               { return TOK_DIV; }

"%"               { return TOK_MOD; }



"("               { return TOK_LPAREN; }

")"               { return TOK_RPAREN; }

"{"               { return TOK_LBRACE; }

"}"               { return TOK_RBRACE; }

"["               { return TOK_LBRACKET; }

"]"               { return TOK_RBRACKET; }

","               { return TOK_COMMA; }

";"               { return TOK_SEMICOLON; }



{FLOAT}           { return TOK_FLOAT_LITERAL; }

{INT}             { return TOK_INT_LITERAL; }

'([^'\\]|\\.)'    { return TOK_CHAR_LITERAL; }

\"([^"\\]|\\.)*\" { return TOK_STRING_LITERAL; }

{ID}              { return TOK_IDENTIFIER; }



<<EOF>>           { return TOK_EOF; }

.                 { return TOK_ERROR; }



%%

```
