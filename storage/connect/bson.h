/**************** bson H Declares Source Code File (.H) ****************/
/*  Name: bson.h   Version 1.0                                         */
/*                                                                     */
/*  (C) Copyright to the author Olivier BERTRAND          2020         */
/*                                                                     */
/*  This file contains the BSON classe declares.                       */
/***********************************************************************/
#pragma once
#include <mysql_com.h>
#include "json.h"
#include "xobject.h"

#if defined(_DEBUG)
#define X  assert(false);
#else
#define X												  
#endif

#define ARGS    MY_MIN(24,(int)len-i),s+MY_MAX(i-3,0)

class BDOC;
class BOUT;
class BJSON;

typedef class BDOC* PBDOC;
typedef class BJSON* PBJSON;
typedef uint  OFFSET;

/***********************************************************************/
/* Structure BVAL. Binary representation of a JVALUE.                  */
/***********************************************************************/
typedef struct _jvalue {
	union {
		OFFSET To_Val;    // Offset to a value
		int    N;         // An integer value
		float  F;				  // A float value
		bool   B;				  // A boolean value True or false (0)
	};
	short    Nd;				// Number of decimals
	short    Type;      // The value type
	OFFSET   Next;      // Offset to the next value in array
} BVAL, *PBVAL;  // end of struct BVALUE

/***********************************************************************/
/* Structure BPAIR. The pairs of a json Object.                        */
/***********************************************************************/
typedef struct _jpair {
	OFFSET Key;    // Offset to this pair key name
	OFFSET Vlp;    // To the value of the pair
	OFFSET Next;   // Offset to the next pair in object
} BPAIR, *PBPR;  // end of struct BPAIR

char* NextChr(PSZ s, char sep);
char* GetJsonNull(void);
const char* GetFmt(int type, bool un);

DllExport bool IsNum(PSZ s);

/***********************************************************************/
/* Class BJSON. The class handling all BJSON operations.               */
/***********************************************************************/
class BJSON : public BLOCK {
public:
	// Constructor
	BJSON(PGLOBAL g, PBVAL vp = NULL) { G = g, Base = G->Sarea; Bvp = vp; }

	// Utility functions
	inline OFFSET   MOF(void  *p) {return MakeOff(Base, p);}
	inline void     *MP(OFFSET o) {return MakePtr(Base, o);} 
	inline PBPR     MPP(OFFSET o) {return (PBPR)MakePtr(Base, o);}
	inline PBVAL    MVP(OFFSET o) {return (PBVAL)MakePtr(Base, o);}
	inline PSZ      MZP(OFFSET o) {return (PSZ)MakePtr(Base, o);}
	inline longlong LLN(OFFSET o) {return *(longlong*)MakePtr(Base, o);}
	inline double   DBL(OFFSET o) {return *(double*)MakePtr(Base, o);}

	void* GetBase(void) { return Base; }
	void  SubSet(bool b = false);
	void  MemSave(void) {G->Saved_Size = ((PPOOLHEADER)G->Sarea)->To_Free;}
	void  GetMsg(PGLOBAL g) { if (g != G) strcpy(g->Message, G->Message); }

	// SubAlloc functions
	void* BsonSubAlloc(size_t size);
	PBPR  SubAllocPair(OFFSET key, OFFSET val = 0);
	OFFSET NewPair(PSZ key, OFFSET val = 0)
				{return MOF(SubAllocPair(DupStr(key), val));}
	PBVAL NewVal(int type = TYPE_NULL);
	PBVAL NewVal(PVAL valp);
	PBVAL SubAllocVal(OFFSET toval, int type = TYPE_NULL, short nd = 0);
	PBVAL SubAllocVal(PBVAL toval, int type = TYPE_NULL, short nd = 0)
				{return SubAllocVal(MOF(toval), type, nd);}
	PBVAL SubAllocStr(OFFSET str, short nd = 0);
	PBVAL SubAllocStr(PSZ str, short nd = 0)
				{return SubAllocStr(DupStr(str), nd);}
	PBVAL DupVal(PBVAL bvp);
	OFFSET DupStr(PSZ str);

	// Array functions
	inline PBVAL GetArray(PBVAL vlp) {return MVP(vlp->To_Val);}
	int   GetArraySize(PBVAL bap, bool b = false);
	PBVAL GetArrayValue(PBVAL bap, int i);
  PSZ   GetArrayText(PGLOBAL g, PBVAL bap, PSTRG text);
	void  MergeArray(PBVAL bap1,PBVAL bap2);
	void  DeleteValue(PBVAL bap, int n);
	void  AddArrayValue(PBVAL bap, OFFSET nvp = NULL, int* x = NULL);
	inline void AddArrayValue(PBVAL bap, PBVAL nvp = NULL, int* x = NULL)
				{AddArrayValue(bap, MOF(nvp), x);}
	void  SetArrayValue(PBVAL bap, PBVAL nvp, int n);
	bool  IsArrayNull(PBVAL bap);

	// Object functions
	inline PBPR GetObject(PBVAL bop) {return MPP(bop->To_Val);}
	inline PBPR	GetNext(PBPR brp) { return MPP(brp->Next); }
	int   GetObjectSize(PBVAL bop, bool b = false);
  PSZ   GetObjectText(PGLOBAL g, PBVAL bop, PSTRG text);
	PBVAL MergeObject(PBVAL bop1, PBVAL bop2);
	void  AddPair(PBVAL bop, PSZ key, OFFSET val = 0);
	PSZ   GetKey(PBPR prp) {return MZP(prp->Key);}
	PBVAL GetVal(PBPR prp) {return MVP(prp->Vlp);}
	PBVAL GetKeyValue(PBVAL bop, PSZ key);
	PBVAL GetKeyList(PBVAL bop);
	PBVAL GetObjectValList(PBVAL bop);
	void  SetKeyValue(PBVAL bop, OFFSET bvp, PSZ key);
	inline void SetKeyValue(PBVAL bop, PBVAL vlp, PSZ key)
				{SetKeyValue(bop, MOF(vlp), key);}
	void  DeleteKey(PBVAL bop, PCSZ k);
	bool  IsObjectNull(PBVAL bop);

	// Value functions
	int   GetSize(PBVAL vlp, bool b = false);
	PBVAL GetNext(PBVAL vlp) {return MVP(vlp->Next);}
	//PJSON GetJsp(void) { return (DataType == TYPE_JSON ? Jsp : NULL); }
	PSZ   GetValueText(PGLOBAL g, PBVAL vlp, PSTRG text);
	inline PBVAL GetBson(PBVAL bvp) { return IsJson(bvp) ? MVP(bvp->To_Val) : bvp; }
	PSZ   GetString(PBVAL vp, char* buff = NULL);
	int   GetInteger(PBVAL vp);
	long long GetBigint(PBVAL vp);
	double GetDouble(PBVAL vp);
	PVAL  GetValue(PGLOBAL g, PBVAL vp);
	void  SetValueObj(PBVAL vlp, PBVAL bop);
	void  SetValueArr(PBVAL vlp, PBVAL bap);
	void  SetValueVal(PBVAL vlp, PBVAL vp);
	PBVAL SetValue(PBVAL vlp, PVAL valp);
	void  SetString(PBVAL vlp, PSZ s, int ci = 0);
	void  SetInteger(PBVAL vlp, int n);
	void  SetBigint(PBVAL vlp, longlong ll);
	void  SetFloat(PBVAL vlp, double f);
	void  SetBool(PBVAL vlp, bool b);
	void  Clear(PBVAL vlp) { vlp->N = 0; vlp->Nd = 0; vlp->Next = 0; }
	bool  IsValueNull(PBVAL vlp);
	bool  IsJson(PBVAL vlp) {return (vlp->Type == TYPE_JAR || vlp->Type == TYPE_JOB);}

	// Members
	PGLOBAL G;
	PBVAL   Bvp;
	void   *Base;

protected:
	// Default constructor not to be used
	BJSON(void) {}
}; // end of class BJSON

/***********************************************************************/
/* Class JDOC. The class for parsing and serializing json documents.   */
/***********************************************************************/
class BDOC : public BJSON {
public:
	BDOC(PGLOBAL G);

	PBVAL ParseJson(PGLOBAL g, char* s, size_t n, int* prty = NULL, bool* b = NULL);
	PSZ   Serialize(PGLOBAL g, PBVAL bvp, char* fn, int pretty);

protected:
	OFFSET ParseArray(int& i);
	OFFSET ParseObject(int& i);
	PBVAL  ParseValue(int& i);
	OFFSET ParseString(int& i);
	void   ParseNumeric(int& i, PBVAL bvp);
	OFFSET ParseAsArray(int& i, int pretty, int* ptyp);
	bool   SerializeArray(OFFSET arp, bool b);
	bool   SerializeObject(OFFSET obp);
	bool   SerializeValue(PBVAL vp);

	// Members used when parsing and serializing
	JOUT* jp;						 // Used with serialize
	char* s;						 // The Json string to parse
	int   len;					 // The Json string length
	bool  pty[3];				 // Used to guess what pretty is

	// Default constructor not to be used
	BDOC(void) {}
}; // end of class BDOC
