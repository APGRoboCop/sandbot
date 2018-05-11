//
// HPB_bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// linkfunc.h
//

#ifndef LINKFUNC_H
#define LINKFUNC_H

#include "archtypes.h"

extern WORD *pOrdinals;
extern DWORD *pFunctionAddresses;
extern DWORD *pNameAddresses;

void LoadExtraExports();

uint32 NameToAddress( const char *pName );
const char *AddressToName(uint32 function);

#endif // LINKFUNC_H
