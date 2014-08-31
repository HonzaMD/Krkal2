/////////////////////////////////////////////////////////////////////////////
//
// ComInstructions.h
//
// Implementace tridy CIntInstructionRec - reprezentace jedne instrukce
// Seznam instrukci a jejich popisu
//
// A: Jiri Margaritov
//
/////////////////////////////////////////////////////////////////////////////

#if !defined COM_COMINSTRUCTIONS_H__INCLUDED
#define COM_COMINSTRUCTIONS_H__INCLUDED

const int KER_INTERPRET_DEFAULT_IL = 10000000;		// nejvyssi pocet instrukci, ktere smi interpret vykonat - prekroceni je povazovano za zacykleni a skonci chybou

// jedna polozka instrukce 
union UIntInstruction
{
	unsigned char	opCode;
	signed short	wordValue[2];
	unsigned char	byteValue[4];
	unsigned char*	ptr;
	int		intValue;
};

// trida reprezentujici instrukci
class CIntInstructionRec
{
public:
	CIntInstructionRec()
	{
		Clear();
	}

	CIntInstructionRec(CIntInstructionRec& ir)
	{
		instr.intValue	= ir.instr.intValue;
		res.intValue	= ir.res.intValue;
		op1.intValue	= ir.op1.intValue;
		op2.intValue	= ir.op2.intValue;
	}

	~CIntInstructionRec() { }

	CIntInstructionRec operator=(CIntInstructionRec ir)
	{
		instr.intValue	= ir.instr.intValue;
		res.intValue	= ir.res.intValue;
		op1.intValue	= ir.op1.intValue;
		op2.intValue	= ir.op2.intValue;
		return *this;
	}

	void Clear()
	{
		instr.intValue = res.intValue = op1.intValue = op2.intValue = 0;
	}

	int GetPart(int part)
	{
		switch(part)
		{
		case 0:		return instr.intValue;
		case 1:		return res.intValue;
		case 2:		return op1.intValue;
		case 3:		return op2.intValue;
		default:
			assert(false);
		}
		return 0;
	}

	// datove cleny - ctyri 32 bitove zaznamy
	UIntInstruction instr;
	UIntInstruction res;
	UIntInstruction op1;
	UIntInstruction op2;
};

const int cInstructionSize = 4 * sizeof(UIntInstruction);

union UIntData64
{
	int	data32[2];
	double data64;
};

// typy operandu instrukce
enum enmOpType
{
eOpTpMissing,
eOpTpImmediate,
eOpTpLocal,
eOpTpStackTmp,
eOpTpAttribute,
eOpTpGlobal,

eOpTpDouble = 8,
eOpTpChar	= 16,
};

const static char* cOpTypes[] = 
{
"missing",
"immediate",
"local",
"stack",
"attribute",
"global",
"name",
};

#define getARG(i)	((i)&7)				// 1. az 3. bit
#define isDOUBLE(i)	(((i)&8)!=0)		// 4. bit
#define isCHAR(i)	(((i)&16)!=0)		// 5. bit

// vycet instrukci
enum enmInstructions
{
instrDATA,
instrDBG,

instrJMP,
instrJMPZ,
instrJMPNZ,
instrRSS,
instrDRET,
instrSRET,
instrSWITCH,

instrMOV,

instrADD,
instrSUB,
instrMUL,
instrDIV,
instrMOD,
instrSHR,
instrSHL,

instrAND,
instrOR,
instrXOR,

instrLAND,
instrLOR,

instrNEG,
instrNOT,

instrINC,
instrDEC,

instrTESTG,
instrTESTL,
instrTESTGE,
instrTESTLE,
instrTESTE,
instrTESTNE,
instrNCMP,
instrNCMPEX,

instrASGND,
instrTYPEOF,

instrADDR,

instrPEEK,
instrAGETCNT,

instrNEW,
instrNEWO,
instrNEWOV,
instrNEWA,

instrSERV,
instrDCALL,
instrSCALL,

instrTHIS,
instrSENDER,
instrNAME,

instrPOKE,

instrPUTDC,
instrPUTSC,

instrDEL,
instrDELO,
instrDELA,

instrAIGET,
instrACGET,
instrADGET,
instrAOGET,
instrANGET,
instrAPGET,

instrAIREAD,
instrACREAD,
instrADREAD,
instrAOREAD,
instrANREAD,
instrAPREAD,

instrASETCNT,

instrSTRCPY,
instrSTRCAT,

instrMSG,
};

struct SIntInstructionDescr
{
	char* name;
	bool hasArg[3];
};

// popis instrukci
const static SIntInstructionDescr cInstructions[] = 
{
	{"DATA",	{false,	false,	false}},
	{"DBG",		{false,	false,	false}},

	{"JMP",		{true,	false,	false}},
	{"JMPZ",	{true,	true,	true}},
	{"JMPNZ",	{true,	true,	true}},
	{"RSS",		{true,	true,	true}},
	{"DRET",	{true,	false,	false}},
	{"SRET",	{true,	false,	false}},
	{"SWITCH",	{true,	true,	true}},

	{"MOV",		{true,	true,	false}},

	{"ADD",		{true,	true,	true}},
	{"SUB",		{true,	true,	true}},
	{"MUL",		{true,	true,	true}},
	{"DIV",		{true,	true,	true}},
	{"MOD",		{true,	true,	true}},
	{"SHR",		{true,	true,	true}},
	{"SHL",		{true,	true,	true}},

	{"AND",		{true,	true,	true}},
	{"OR",		{true,	true,	true}},
	{"XOR",		{true,	true,	true}},

	{"LAND",	{true,	true,	true}},
	{"LOR",		{true,	true,	true}},

	{"NEG",		{true,	true,	false}},
	{"NOT",		{true,	true,	false}},

	{"INC",		{true,	true,	false}},
	{"DEC",		{true,	true,	false}},

	{"TESTG",	{true,	true,	true}},
	{"TESTL",	{true,	true,	true}},
	{"TESTGE",	{true,	true,	true}},
	{"TESTLE",	{true,	true,	true}},
	{"TESTE",	{true,	true,	true}},
	{"TESTNE",	{true,	true,	true}},
	{"NCMP",	{true,	true,	true}},
	{"NCMPEX",	{true,	true,	true}},

	{"ASGND",	{true,	true,	false}},
	{"TYPEOF",	{true,	true,	false}},

	{"ADDR",	{true,	true,	true}},

	{"PEEK",	{true,	true,	true}},
	{"AGETCNT",	{true,	true,	true}},

	{"NEW",		{true,	true,	true}},
	{"NEWO",	{true,	true,	true}},
	{"NEWOV",	{true,	true,	true}},
	{"NEWA",	{true,	true,	false}},

	{"SERV",	{true,	true,	true}},
	{"DCALL",	{true,	true,	true}},
	{"SCALL",	{true,	true,	true}},

	{"THIS",	{true,	false,	false}},
	{"SENDER",	{true,	false,	false}},
	{"NAME",	{true,	true,	false}},

	{"POKE",	{true,	true,	true}},

	{"PUTDC",	{true,	false,	false}},
	{"PUTSC",	{true,	true,	true}},

	{"DEL",		{true,	true,	false}},
	{"DELO",	{true,	true,	false}},
	{"DELA",	{true,	true,	false}},

	{"AIGET",	{true,	true,	true}},
	{"ACGET",	{true,	true,	true}},
	{"ADGET",	{true,	true,	true}},
	{"AOGET",	{true,	true,	true}},
	{"ANGET",	{true,	true,	true}},
	{"APGET",	{true,	true,	true}},

	{"AIREAD",	{true,	true,	true}},
	{"ACREAD",	{true,	true,	true}},
	{"ADREAD",	{true,	true,	true}},
	{"AOREAD",	{true,	true,	true}},
	{"ANREAD",	{true,	true,	true}},
	{"APREAD",	{true,	true,	true}},

	{"ASETCNT",	{true,	true,	true}},

	{"STRCPY",	{true,	true,	false}},
	{"STRCAT",	{true,	true,	true}},

	{"MSG",		{true,	true,	true}},
};

#endif