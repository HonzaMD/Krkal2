/////////////////////////////////////////////////////////////////////////////
//
// ComConstants.h
//
// Seznam vsech konstant, vyctu a nastaveni kompilatoru
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#if !defined COM_COMCONSTANTS_H__INCLUDED

#define COM_COMCONSTANTS_H__INCLUDED

#include "KerConstants.h"

// Obecne
const int cUnknown = -1;
const int cError = -1;
const int cOK = 0;

// Sluzby kernelu:

const int cKerServicesNr = 135;
const int cKerServicesMaxParams = 11;
const int cKerConstantsNr = 59;


// Token
enum enmTokens
{
	tokVoidToken		= -10000000,	// -10M
	tokMetaVersion		= -3000000,		// -3M
	tokMetaPosFirst		= -3000000,
	tokMetaFile			= -2000000,
	tokMetaLineLast		= -2000000,
	tokMetaLineFirst	= -1000000,
	tokMetaColumnLast	= -1000000,
	tokMetaColumnFirst	= 0,
	tokMetaPosLast,
	tokMetaLast,
	statFirst,
	lexFirst, 
	lexStartBlock, 
	lexEndBlock,
	lexSeparator,
	lexEOF,
	lexEditorStringSeparator,
	lexLast,
	synFirst,
	synStartObjectDecl,
	synEndObjectDecl,
	synAttributeDecl,
	synMethodDecl,
	synStartExpression,
	synEndExpression,
	synStartBlock,
	synEndBlock,
	synStartFunctionCall,
	synNextParameter,
	synEndFunctionCall,
	synSpecialCall,
	synTimeDecl,
	synPassingName,
	synLocalDefinition,
	synGlobalDefinition,
	synNameDefinition,
	synConditional1,
	synConditional2,
	synLogicOr,
	synLogicAnd,
	synArgsEnd,
	synEndCase,
	synCaseCount,
	synLocalizedArgument,
	synSafeArgument,
	synAssigning, 
	synAddressing, 
	synLast,
	semFirst,
	semMethodBody,
	semDynamicArrayCall,
	semNameCall,
	semImplicitSpec,
	semLast,
	statLast,
	tokUnknownFirst	= 1000,
	tokUnknownLast	= 10000000,	// 10M
	tokConstantFirst,
	tokConstantLast	= 20000000,	
	tokObjectFirst,
	tokObjectLast	= 30000000,
	tokNameFirst,
	tokNameLast		= 40000000,
	tokDirectNameFirst,
	tokDirectNameLast = 50000000,
	tokGlobalNameFirst,
	tokGlobalNameLast = 60000000,
	tokDependFirst,
	tokDependLast	= 70000000,
	tokGlobalFirst,
	tokGlobalLast	= 80000000,
	tokMethodFirst,
	tokMethodLast	= 90000000,
	tokAttributeFirst,
	tokAttributeLast= 100000000,// 100M
	tokLocalFirst,
	tokLocalLast	= 110000000, 
	tokArgumentFirst,
	tokArgumentLast	= 120000000, 
	tokTmpExpFirst,
	tokTmpExpLast	= 130000000, 
	tokSaveTmpExpFirst,
	tokSaveTmpExpLast = 140000000, 
	tokStructureFirst,
	tokStructureLast= 150000000, 
	tokDataFirst	= 1000000000 // 1G
};

// CComSymbolTable konstanty

enum
{
	findAny		= -3,
	findNewest	= -2,
};

// CComSTE konstanty
enum
{
	pteFirst,
	pteDefinedSymbol,		// symbol z #define symbol
	pteDirective,			// direktiva preprocesoru - napriklad #ifdef
	pteFileName,			// jmeno souboru s prekladanym kodem 
	pteKeyword,				// klicove slovo
	pteKnownName,			// zname jmeno
	pteEditTag,				// edit tag
	pteKernelService,		// sluzba kernelu
	pteKernelConstant,		// konstanta kernelu
	pteLast
};

enum
{
	dsymDefined,
	dsymUndefined
};

// CComPart konstanty
const int	cDefaultQueueLn = 1024;

enum enmInputType
{
	inpNone,
	inpPart,
	inpWindow,
	inpFile,
	inpString
};

// CComError konstanty
enum 
{
	errNone,
	errExpectedIdentifier,
	errIdentifierTooLong,
	errExpectedNumberValue,
	errIntegerOverflow,
	errExpectedTime,
	errInvalidOperator,
	errDirectiveTooLong,
	errInvalidDirective,
	errMissingIncludeFile,
	errDirectiveArgTooLong,
	errDirectiveArgBadFormat,
	errCantOpenIncludeFile,
	errFileAlreadyIncluded,
	errCannotUseOperator,
	errMissingSymbolName,
	errUnexpectedCharacter,
	errExpectedSymbol,
	errEndifWithoutIf,
	errUnknownVersion,
	errUnexpectedSymbol,
	errNestedObjectDecl,
	errUnexpectedDirective,
	errUnexpectedEOF,
	errMultipleStorage,
	errMultipleSign,
	errMultipleType,
	errUnknownType,
	errInvalidFunctionStorage,
	errInvalidDataStorage,
	errFeatureNotSupported,
	errMissingFunctionBody,
	errMissingType,
	errEndBlockWithoutStart,
	errMissingEndBlock,
	errNotDeclared,
	errExpectedDynamicSymbol,
	errIdentifierBadFormat,
	errUnexpectedKeyword,
	errAlreadyDeclared,
	errExpectedType,
	errCannotDoMultipleMethodDecl,
	errDefaultArgWithinDirect,
	errArgumentViolatesSafeCallRules,
	errExpectedKeyword,
	errAlreadyDefined,
	errUndefinedSymbol,
	errInvalidType,
	errUnknownIdentifier,
	errEscapeSequence,
	errStingTooLong,
	errIllegalFunctionCall,
	errReturnTypeMismatch,
	errShouldReturnValue,
	errIllegalMemberAccess,
	errExpectedMethodCall,
	errBadParameterFormat,
	errParameterTypeMismatch,
	errPassingTooManyParameters,
	errPassingTooFewParameters,
	errMessagesCantReturnValue,
	errExpectedName,
	errMissingConstructor,
	errMissingDestructor,
	errConstructorOrDestructorReturns,
	errMultipleDestructors,
	errTypeMismatch,
	errCannotDeleteStaticalObject,
	errExpectedObjectType,
	errTooMuchArguments,
	errExpectedAttributeDecl,
	errUnknownEditTag,
	errExpectedConstant,
	errUnknownKnownName,
	errExpectedMethodDecl,
	errExpectedMethodKnownName,
	errInvalidMemberSpec,
	errExpectedParameterKnownName,
	errInvalidIndexType,
	errInvalidEditTag,
	errCycleInNameDependencies,
	errExpectedNameSetOperator,
	errMethodBodyNotDefined,
	errIllegalBreak,
	errUnknownKernelService,
	errMultipleRetInParam,
	errSameCase,
	errIllegalContinue,
	errCannotDeclareLocalInSwitch,
	errInvalidStringLn,
	errUnresolvedDirtyOp,
	errMissingHead,
	errMissingVersion,
	errWrongVersion,
	errExpectedArgument,
	errNestedAttributeGroup,	
	errInvalidMemberInAttributeGroup,
	errIncompleteAttributeGroup,
	errTooMuchMembersInAttributeGroup,
	errExpectedEditorType,
	errCannotLocalizeParamToDirect,
	errBadDirectMethodName,
	errTooFewArguments,
	errExpectedLValue,
	errExpectedMethodName,
	errDirectiveNotAllowedInDirect,
	errWarningFirst = 1000,
	warnTypeNotSupported,
	warnNonPointerObject,
	warnNameTypeMismatch,
	errLast
};

enum 
{
	noteNone,
	noteMayBeEndOfComment,
	noteCheckDefine,
	noteCheckUndef,
	noteNumberSyntaxError,
	noteCheckObject,
	noteCheckIfdef,
	noteCheckIfndef,
	noteCheckHead,
	noteDeclNotComplete,
	noteCheckFunction,
	noteForgetSeparator,
	noteForgetEndBlock,
	noteCheckIf,
	noteCheckWhile,
	noteCheckDo,
	noteCheckFor,
	noteCheckConditional,
	noteTimeAndNotSafe,
	noteMissingSepBeforeEndBlock,
	noteCheckIdentifierRules,
	noteMissingEndHead,
	noteCheckTopLevelRules,
	noteAlreadyDefinedGlobal,
	noteCheckDeclRules,
	noteWillCompileAsPointer,
	noteCheckBaseTypes,
	noteNotMethodNorAttribute,
	noteAlreadyDeclaredAttribute,
	noteAlreadyDeclaredMethod,
	noteMethodReturnsValue,
	noteCheckSafeCallRules,
	noteCheckDefaultArgs,
	noteCheckArgListRules,
	noteAlreadyDeclaredArgument,
	noteMissingConstructorArgs,
	noteCantContinueParamList,
	noteCheckLocalDefRules,
	noteAlreadyDefinedLocal,
	noteCheckNameDef,
	noteAlreadyDefinedName,
	noteSupposedToBeAnObject,
	noteSupposedToBeAMethod,
	noteSupposedToBeAnAttribute,
	noteDefineBeforeDepend,
	noteObjptrBeforeNonObject,
	noteForgetDeclarator,
	noteCheckGlobalDefRules,
	noteVoidData,
	noteMissingEndIf,
	noteAlreadyDeclaredObject,
	noteCheckOpSemantic,
	noteMustBeLValue,
	noteCannotDereferenceVoid,
	noteMustBeMemberDot,
	noteMustBeMemberArrow,
	noteHasntSuchMember,
	noteIsntMember,
	noteCheckEscape,
	noteCheckChar,
	noteCheckString,
	noteForgetEndString,
	noteNotMethod,
	noteExpectedDirectFormat,
	noteExpectedSafeFormat,
	noteCheckTime,
	noteTimeAtDirect,
	noteConstructorOrDestructorBadName,
	noteAlreadyDeclaredStructure,
	noteLast
};

const static char* cLexicalSymbols[] = 
{
	"{",
	"}",
	";",
	"end of file",
	"@",
};

const static char* cSyntaxSymbols[] = 
{
	"start of object declaration",
	"end of object declaration",
	"attribute declaration",
	"method declaration",
	"start of expression",
	"end of expression",
	"start of block",
	"end of block",
	"start of function call",
	"next parameter",
	"end of function call",
	"special call",
	"time declared",
	"passing name",
	"local definition",
	"global definition",
	"name definition",
	"conditional first expession",
	"conditional second expession",
	"logic or expression",
	"logic and expression",
	"end of argument list",
	"end of case statement",
	"count of case statements",
	"localized argument",
	"safe argument",
	"assigning",
	"addressing",
};

const static char* cSemanticSymbols[] = 
{
	"method body",
	"dynamic array call",
	"name call",
	"implicit specification",
};


const int cMaxStringLn = 250;			// POZOR!!! ZAVISI NA KERNELU.
// POZOR Pøenastaveno by M.D.
const int cMaxStringLnMD = 2500;			// POZOR!!! ZAVISI NA KERNELU.

// - klicova slova:
enum enmKeywords
{
	kwFirst = statLast,
	kwANullChar,kwANullDouble,kwANullInt,kwANullObjptr,kwANullName,kwANullVoid,kwArea2D,kwArea3D,kwAssigned,
	kwBreak,
	kwCallend,kwCase,kwCell2D,kwCell3D,kwCellArea2D,kwCellArea3D,kwChar,kwCharArray,kwConstructor,kwContinue,kwCConstructor,
	kwDecl,kwDefault,kwDelete,kwDepend,kwDepends,kwDestructor,kwDirect,kwDo,kwDouble,kwDoubleArray,
	kwEdit,kwElse,kwEnd,kwEndModify,
	kwFor,
	kwIf,kwInherit,kwInt,kwIntArray,
	kwLConstructor,
	kwMethodName,kwMessage,kwModify,
	kwName,kwNameArray,kwNew,kwNextTurn,kwNextEnd,kwNNull,kwNull,
	kwObject,kwObjectName,kwObjptr,kwObjptrArray,kwONull,
	kwParamName,kwPoint2D, kwPoint3D,
	kwRemove,kwRet,kwRetAdd,kwRetAnd,kwRetOr,kwReturn,kwReturns,
	kwSafe,kwSender,kwSizeof,kwStruct,kwSwitch,
	kwThis,kwTimed,kwTypeOf,
	kwUConstructor,kwUnion,
	kwVarType,kwVoid,kwVoidArray,kwVoidName,
	kwWith,kwWhile,
	kwScripted,kwString,
	kwStringLast = kwString+cMaxStringLn,
	kwLast,
// - znama jmena:
	knFirst,
	knSecond = knFirst + KERVARUSESSIZE,
	knLast = knSecond + MAXKNOWNNAMES,
// - sluzby kernelu:
	ksFirst,
	ksLast = ksFirst + cKerServicesNr,
// - konstanty kernelu:
	kcFirst,
	kcLast = kcFirst + cKerConstantsNr,
	
	kwBlockLast
};

const static char* cKeywords[] = 
{
	"anullchar","anulldouble","anullint","anullobjptr","anullname","anullvoid","area2D","area3D","assigned",
	"break",
	"callend","case","cell2D","cell3D","cellarea2D","cellarea3D","char","chara","constructor","continue","cconstructor",
	"decl","default","delete","depend","depends","destructor","direct","do","double","doublea",
	"edit","else","end","endmodify",
	"for",
	"if","inherit","int","inta",
	"lconstructor",
	"methodname","message","modify",
	"name","namea","new","nextturn","nextend","nnull","null",
	"object","objectname","objptr","objptra","onull",
	"paramname","pos2D","pos3D",
	"remove","ret","retadd","retand","retor","return","returns",
	"safe","sender","sizeof","struct","switch",
	"this","timed","typeof",
	"uconstructor","union",
    "vartype","void","voida","voidname",
	"with","while",
	"scripted","string",
};

static const char* cStrOPointer = "OPointer";
static const char* cStrCKerName = "CKerName";
static const char* cStrEmpty = "";
static const char* cStrUnsignedChar = "unsigned char";
static const char* cStrString = "char*";
static const char* cStrThisVersion = "this version";
// - operatory:

static const char* cDynamicArrayNames[] = {"CKerArrChar","CKerArrDouble","CKerArrInt",
	"CKerArrPointer","CKerArrObject","CKerArrName"};

enum enmOperators
{
	opFirst = kwBlockLast,
	opPlus,opMinus,opMultiply,opDivide,opModulo,
	opLeftShift,opRightShift,
	opLess,opLessEqual,opGreater,opGreaterEqual,opEqual,opNotEqual,
	opBitAnd,opBitOr,opBitXor,
	opLogicAnd,opLogicOr,
	opAssign,opAssignPlus,opAssignMinus,opAssignMultiply,opAssignDivide,opAssignModulo,
	opAssignLeftShift,opAssignRightShift,opAssignAnd,opAssignOr,opAssignXor,
	opConditional,
	opSizeof,
	opPreIncrement,opPreDecrement,
	opBitNot,opLogicNot,
	opAddress,opIndirection,
	opPostIncrement,opPostDecrement,
    opUnaryPlus,opUnaryMinus,
	opMemberDot,opMemberArrow,opMember4Dot,
	opComma,opDoubleDot,
	opBracketStart,opBracketEnd,
	opParentStart,opParentEnd,
	opLast
};

const static char* cOperators[] =
{	
	"+","-","*","/","%",
	"<<",">>",
	"<","<=",">",">=","==","!=",
	"&","|","^",
	"&&","||",
	"=","+=","-=","*=","/=","%=",
	"<<=",">>=","&=","|=","^=",
	"?",
	"sizeof",
	"++","--",
	"~","!",
	"&","*",
	"++","--",
	"+","-",	// unary + a -
	".","->","::",
	",",":",
	"[","]",
	"(",")",
};

// direktivy:

enum 
{
	dirFirst,
	dirDefine,dirUndef,
	dirIf,dirIfdef,dirIfndef,dirElif,dirElse,dirEndif,
	dirEmpty,
	dirLast
};

const static char* cDirectives[] =
{
	"#define","#undef",
	"#if","#ifdef","#ifndef","#elif","#else","#endif",
	"#",
};

// edit-tagy:
enum
{
	etUserName,
	etComment,
	etDefaultValue,
	etDefaultMember,
	etInterval,
	etList,
	etIs,
	etFunction,
	etEditable,
	etSpecialEdit,
	etPozorPozor,
	etEditType,
	etExclusive,
	etIncludeNull,
	etPlanarNames,

	etInMap,
	etOutMap,
	etNoGrid,
	etEditor,
	etCollReplace,
	etCollDontPlace,
	etCollIgnore,

	etLevelLoad,
	etAuto,
	etNoLevelLoad,

	etNumber,
	etLetter,
	etBool,
	etMethod,
	etParam,
	etAutomatism,
	etObjectshadow,
	etKey,
	etSound,
	etObjVoid,
	etAutoVoid,
	etAnyName,
	etLast
};

const static char* cEditTags[] =
{
	"UserName",
	"Comment",
	"DefaultValue",
	"DefaultMember",
	"Interval",
	"List",
	"Is",
	"Function",
	"Editable",
	"SpecialEdit",
	"PozorPozor",
	"EditType",
	"Exclusive",
	"IncludeNull",
	"PlanarNames",

	"InMap",
	"OutMap",
	"NoGrid",
	"Editor",
	"CollReplace",
	"CollDontPlace",
	"CollIgnore",

	"LevelLoad",
	"Auto",
	"NoLevelLoad",

	"number",
	"letter",
	"bool",
	"method",
	"param",
	"automatism",
	"objectshadow",
	"key",
	"sound",
	"objvoid",
	"autovoid",
	"anyname",
};

enum enmObjET
{
	etbPodu		 = 1,
	etbInMap	 = 2,
	etbOutMap	 = 4,
	etbNoGrid	 = 8,
	etbCollision = 48,
};

enum enmEditTypes
{
	etnmVoid	= 1,
	etnmObject	= 2,
	etnmMethod	= 4,
	etnmParam	= 8,
	etnmAutomatism = 16,
	etnmAnyName = -1,
	etnmObjectShadow = 32,
	etnmKey = 64,
	etnmSound = 128,
	etnmObjVoid = 256,
	etnmAutoVoid = 512,
};

// Pozor v Kernelu to mam nadefinovane taky. Propaguj zmeny!
enum enmDataET
{
	etbDataLevelLoad	= 1,
	etbDataAuto			= 2,
	etbDataEditable		= 4,
	etbDataSpecialEdit	= 8,
	etbDataPozorPozor	= 16,
};

enum enmInherit
{
	inhNotInheritable, inhInheritable, inhInherited,
};

const int cKeywordsNr = kwLast-kwFirst-1;
const int cOperatorsNr = opLast-opFirst-1;
const int cDirectivesNr = dirLast-dirFirst-1;

// CComSyntax konstanty:
enum
{
	lvlTop,
	lvlObject,
	lvlFunction
};

enum
{
	omtAttribute,
	omtMethod
};

// Omezeni a nastaveni:
const int cMaxIdentifierLn = 64;
const int cMaxNumberConstantLn = 32;
const int cMaxDirectiveLn = 16;
const int cMaxFileNameLn = 1024;
const int cMaxErrorMsgLn = 1024;
const int cMaxDebugStringLn = 1024;
const int cMaxParameterNr = 255;

const bool cMatchFileNameCase = false;
const int cTabSize = 1;

const int cUnknownsHTSize = 1024;
const int cConstantsHTSize = 512;
const int cObjectsHTSize = 256;
const int cNameHTSize = 256;
const int cDependHTSize = 256;
const int cGlobalHTSize = 256;
const int cAttributesHTSize = 16;
const int cMethodsHTSize = 32;
const int cStructuresHTSize = 4;
const int cStructureMembersHTSize = 4;
const int cLocalsHTSize = 32;
const int cArgumentsHTSize = 8;
const int cPreprocessorHTSize = 128;
const int cTmpExpHTSize = 512;
const int cGMethodsHTSize = 1024;
const int cGArgumentsHTSize = 1024;

const int cLexicalQueueLn = 1024;
const int cSyntaxQueueLn = 65536;
const int cSemanticQueueLn = 65536;
const int cCodeGenQueueLn = 64;
const int cFileCacheSize = 4096;

const int cFastNameSearchSize = 1024;
const int cFastNameSearchMaxVersions = 4;

const int cMaxOBTabIndent = 16;

const int cFirstTempRegister = 250;
const int cLastTempRegister = 255;
const int cStrVersionLn = 20;

const bool	cNoDebugInCode = true;
const int	cMethodCallRecordSize = 24;
const int	cPointerSize = 4;

const static char* cSemanticOutputFile = "semantic.out";
const static char* cSyntaxOutputFile = "syntax.out";

#endif