#ifndef MAIN_H
#define MAIN_H

#include "stdafx.h"

void Init();
void Deinit();
BOOL DllGetVersion(char** lplpsz);

void RecordExceptionInfoMain_asm();
void RecordExceptionInfoThreadStart_asm();
void RecordExceptionInfoThreadStartEx_asm();
#endif //MAIN_H