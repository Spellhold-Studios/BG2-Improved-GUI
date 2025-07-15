#ifndef STDAFX_H
#define STDAFX_H

#pragma optimize("y", off)  //disable FPO for stack traces

#include "win32def.h"

#include <cassert>
#include <map>

#include "utils.h"
#include "MathPresso.h"

#include "cstringex.h"
#include "cptrlistex.h"
#include "cmapstrstrex.h"
#include "resref.h"

#include "globals.h"

#include "console.h"
#include "log.h"

#include "options.h"

typedef unsigned long STRREF;
typedef          long Enum;     //index to CGameObjectArrayHandler element

typedef IECPtrList CDwordList; //AA5C50, all are EnumList?
typedef IECPtrList CEnumList; //AA5C50
typedef IECPtrList CIECStringList; //AA63B4
typedef IECPtrList CPositionList; //AA702C
typedef IECPtrList CResRefList; //AA6334
typedef IECPtrList CGameObjectList; //AA7128

const Enum ENUM_INVALID_INDEX = UINT_MAX;

#endif //STDAFX_H