#ifndef RESREF_H
#define RESREF_H

#include "win32def.h"

#include "cstringex.h"

class ResRef {
public:
	~ResRef();
    ResRef();
	ResRef(const IECString& s);
	ResRef(LPCTSTR sz);
    ResRef(unsigned char* sz);

    void       CopyToString(IECString& s) const;
	void       Clean();
	LPTSTR     GetResRef() const;   // return NOT NULL-ENDED char *
	IECString  GetResRefStr() const;
	BOOL       IsValid() const;
    void       CopyToString(LPTSTR sz) const;

	BOOL operator!=(const ResRef& r) const;
	BOOL operator!=(const IECString& s) const;
    BOOL operator!=(LPCTSTR sz) const;
    BOOL operator==(const IECString& s) const;
    BOOL operator==(LPCTSTR sz) const;
    BOOL EqualsSubstring(LPCTSTR sz) const;

	BOOL IsEmpty() const;
    ResRef operator=(const ResRef& r);
	ResRef operator=(unsigned char* sz);
	ResRef operator=(const IECString& s);
    ResRef operator=(LPCTSTR sz);
	void MakeUpper();
	ResRef operator+=(const IECString& s);
    void CopyTo8Chars(LPTSTR sz);           // GetResRef(LPTSTR sz), copy ResRef as 8 bytes

	//additional implementations
	//operator LPTSTR() const;
    LPTSTR  GetResRefNulled() const;        // not multithreading safe !
    LPTSTR  GetResRefNulled2() const;       // not multithreading safe !
    BOOL operator==(const ResRef& r) const;

protected:
	char buf[8];
};

void GetResRefNulled_asm();
void GetCharPtrNulled_asm();

#endif //RESREF_H
