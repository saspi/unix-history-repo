typedef union {long itype; tree ttype; enum tree_code code;
	char *filename; int lineno; } YYSTYPE;
#define	IDENTIFIER	258
#define	TYPENAME	259
#define	SCSPEC	260
#define	TYPESPEC	261
#define	TYPE_QUAL	262
#define	CONSTANT	263
#define	STRING	264
#define	ELLIPSIS	265
#define	SIZEOF	266
#define	ENUM	267
#define	STRUCT	268
#define	UNION	269
#define	IF	270
#define	ELSE	271
#define	WHILE	272
#define	DO	273
#define	FOR	274
#define	SWITCH	275
#define	CASE	276
#define	DEFAULT	277
#define	BREAK	278
#define	CONTINUE	279
#define	RETURN	280
#define	GOTO	281
#define	ASM_KEYWORD	282
#define	TYPEOF	283
#define	ALIGNOF	284
#define	ALIGN	285
#define	ATTRIBUTE	286
#define	EXTENSION	287
#define	LABEL	288
#define	ASSIGN	289
#define	OROR	290
#define	ANDAND	291
#define	EQCOMPARE	292
#define	ARITHCOMPARE	293
#define	LSHIFT	294
#define	RSHIFT	295
#define	UNARY	296
#define	PLUSPLUS	297
#define	MINUSMINUS	298
#define	HYPERUNARY	299
#define	POINTSAT	300
#define	INTERFACE	301
#define	IMPLEMENTATION	302
#define	END	303
#define	SELECTOR	304
#define	DEFS	305
#define	ENCODE	306
#define	CLASSNAME	307
#define	PUBLIC	308


extern YYSTYPE yylval;
