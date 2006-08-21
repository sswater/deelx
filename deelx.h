// deelx.h
//
// DEELX Regular Expression Engine (v1.0)
//
// Copyright 2006 (c) RegExLab.com
// All Rights Reserved.
//
// http://www.regexlab.com/deelx/
//
// Author:  ∑ ŸŒ∞ (sswater shi)
// sswater@gmail.com
//

#ifndef __DEELX_REGEXP__H__
#define __DEELX_REGEXP__H__

#include <memory.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

//
// Data Reference
//
template <class ELT> class CBufferRefT
{
public:
	CBufferRefT(const ELT * pcsz, int length);
	CBufferRefT(const ELT * pcsz);

public:
	int nCompare      (const ELT * pcsz) const;
	int nCompareNoCase(const ELT * pcsz) const;

	ELT At          (int nIndex, ELT def = 0) const;
	ELT operator [] (int nIndex) const;

	const ELT * GetBuffer() const;
	int GetSize() const;

public:
	virtual ~CBufferRefT();

// Content
protected:
	const ELT * m_pRef;
	int         m_nSize;
};

//
// Implemenation
//
template <class ELT> CBufferRefT <ELT> :: CBufferRefT(const ELT * pcsz, int length)
{
	m_pRef  = pcsz;
	m_nSize = length;
}

template <class ELT> CBufferRefT <ELT> :: CBufferRefT(const ELT * pcsz)
{
	m_pRef  = pcsz;
	m_nSize = 0;

	if(pcsz != 0) while(m_pRef[m_nSize] != 0) m_nSize ++;
}

template <class ELT> int CBufferRefT <ELT> :: nCompare(const ELT * pcsz) const
{
	for(int i=0; i<m_nSize; i++)
	{
		if(m_pRef[i] != pcsz[i])
			return m_pRef[i] - pcsz[i];
	}

	return 0;
}

template <class ELT> int CBufferRefT <ELT> :: nCompareNoCase(const ELT * pcsz) const
{
	for(int i=0; i<m_nSize; i++)
	{
		if(m_pRef[i] != pcsz[i])
		{
			if(toupper((int)m_pRef[i]) != toupper((int)pcsz[i]))
				return m_pRef[i] - pcsz[i];
		}
	}

	return 0;
}

template <class ELT> inline ELT CBufferRefT <ELT> :: At(int nIndex, ELT def) const
{
	return nIndex >= m_nSize ? def : m_pRef[nIndex];
}

template <class ELT> inline ELT CBufferRefT <ELT> :: operator [] (int nIndex) const
{
	return nIndex >= m_nSize ? 0 : m_pRef[nIndex];
}

template <class ELT> inline const ELT * CBufferRefT <ELT> :: GetBuffer() const
{
	return m_pRef;
}

template <class ELT> inline int CBufferRefT <ELT> :: GetSize() const
{
	return m_nSize;
}

template <class ELT> CBufferRefT <ELT> :: ~CBufferRefT()
{
}

//
// Data Buffer
//
template <class ELT> class CBufferT : public CBufferRefT <ELT>
{
public:
	CBufferT(const ELT * pcsz, int length);
	CBufferT(const ELT * pcsz);
	CBufferT();

public:
	ELT & operator [] (int nIndex);
	void  Append(const ELT * pcsz, int length, int eol = 0);
	void  Append(ELT el, int eol = 0);

public:
	void  Push(ELT   el);
	int   Pop (ELT & el);
	int   Peek(ELT & el) const;

public:
	const ELT * GetBuffer() const;
	ELT * GetBuffer();
	ELT * Detach();
	void  Release();
	void  Prepare(int index, int fill = 0);
	void  Restore(int size);

public:
	virtual ~CBufferT();

// Content
protected:
	ELT * m_pBuffer;
	int   m_nMaxLength;
};

//
// Implemenation
//
template <class ELT> CBufferT <ELT> :: CBufferT(const ELT * pcsz, int length) : CBufferRefT <ELT> (0, length)
{
	m_nMaxLength = CBufferRefT <ELT> :: m_nSize + 1;

	CBufferRefT <ELT> :: m_pRef = m_pBuffer = new ELT[m_nMaxLength];
	memcpy(m_pBuffer, pcsz, sizeof(ELT) * CBufferRefT <ELT> :: m_nSize);
	m_pBuffer[CBufferRefT <ELT> :: m_nSize] = 0;
}

template <class ELT> CBufferT <ELT> :: CBufferT(const ELT * pcsz) : CBufferRefT <ELT> (pcsz)
{
	m_nMaxLength = CBufferRefT <ELT> :: m_nSize + 1;

	CBufferRefT <ELT> :: m_pRef = m_pBuffer = new ELT[m_nMaxLength];
	memcpy(m_pBuffer, pcsz, sizeof(ELT) * CBufferRefT <ELT> :: m_nSize);
	m_pBuffer[CBufferRefT <ELT> :: m_nSize] = 0;
}

template <class ELT> CBufferT <ELT> :: CBufferT() : CBufferRefT <ELT> (0, 0)
{
	m_nMaxLength = 0;
	m_pBuffer    = 0;
}

template <class ELT> inline ELT & CBufferT <ELT> :: operator [] (int nIndex)
{
	return m_pBuffer[nIndex];
}

template <class ELT> void CBufferT <ELT> :: Append(const ELT * pcsz, int length, int eol)
{
	int nNewLength = m_nMaxLength;

	// Check length
	if(nNewLength < 8)
		nNewLength = 8;

	if(CBufferRefT <ELT> :: m_nSize + length + eol > nNewLength)
		nNewLength *= 2;

	if(CBufferRefT <ELT> :: m_nSize + length + eol > nNewLength)
	{
		nNewLength  = CBufferRefT <ELT> :: m_nSize + length + eol + 11;
		nNewLength -= nNewLength % 8;
	}

	// Realloc
	if(nNewLength > m_nMaxLength)
	{
		ELT * pNewBuffer = new ELT[nNewLength];

		if(m_pBuffer != 0)
		{
			memcpy(pNewBuffer, m_pBuffer, sizeof(ELT) * CBufferRefT <ELT> :: m_nSize);
			delete [] m_pBuffer;
		}

		CBufferRefT <ELT> :: m_pRef = m_pBuffer = pNewBuffer;
		m_nMaxLength = nNewLength;
	}

	// Append
	memcpy(m_pBuffer + CBufferRefT <ELT> :: m_nSize, pcsz, sizeof(ELT) * length);
	CBufferRefT <ELT> :: m_nSize += length;

	if(eol > 0) m_pBuffer[CBufferRefT <ELT> :: m_nSize] = 0;
}

template <class ELT> inline void CBufferT <ELT> :: Append(ELT el, int eol)
{
	Append(&el, 1, eol);
}

template <class ELT> void CBufferT <ELT> :: Push(ELT el)
{
	// Realloc
	if(CBufferRefT <ELT> :: m_nSize >= m_nMaxLength)
	{
		int nNewLength = m_nMaxLength * 2;
		if( nNewLength < 8 ) nNewLength = 8;

		ELT * pNewBuffer = new ELT[nNewLength];

		if(m_pBuffer != 0)
		{
			memcpy(pNewBuffer, m_pBuffer, sizeof(ELT) * CBufferRefT <ELT> :: m_nSize);
			delete [] m_pBuffer;
		}

		CBufferRefT <ELT> :: m_pRef = m_pBuffer = pNewBuffer;
		m_nMaxLength = nNewLength;
	}

	// Append
	m_pBuffer[CBufferRefT <ELT> :: m_nSize++] = el;
}

template <class ELT> inline int CBufferT <ELT> :: Pop(ELT & el)
{
	if(CBufferRefT <ELT> :: m_nSize > 0)
	{
		el = m_pBuffer[--CBufferRefT <ELT> :: m_nSize];
		return 1;
	}
	else
	{
		return 0;
	}
}

template <class ELT> inline int CBufferT <ELT> :: Peek(ELT & el) const
{
	if(CBufferRefT <ELT> :: m_nSize > 0)
	{
		el = m_pBuffer[CBufferRefT <ELT> :: m_nSize - 1];
		return 1;
	}
	else
	{
		return 0;
	}
}

template <class ELT> inline const ELT * CBufferT <ELT> :: GetBuffer() const
{
	return m_pBuffer;
}

template <class ELT> inline ELT * CBufferT <ELT> :: GetBuffer()
{
	return m_pBuffer;
}

template <class ELT> ELT * CBufferT <ELT> :: Detach()
{
	ELT * pBuffer = m_pBuffer;

	CBufferRefT <ELT> :: m_pRef  = m_pBuffer    = 0;
	CBufferRefT <ELT> :: m_nSize = m_nMaxLength = 0;

	return pBuffer;
}

template <class ELT> void CBufferT <ELT> :: Release()
{
	ELT * pBuffer = Detach();

	if(pBuffer != 0) delete [] pBuffer;
}

template <class ELT> void CBufferT <ELT> :: Prepare(int index, int fill)
{
	int nNewSize = index + 1;

	// Realloc
	if(nNewSize > m_nMaxLength)
	{
		int nNewLength = m_nMaxLength;

		if( nNewLength < 8 )
			nNewLength = 8;

		if( nNewSize > nNewLength )
			nNewLength *= 2;

		if( nNewSize > nNewLength )
		{
			nNewLength  = nNewSize + 11;
			nNewLength -= nNewLength % 8;
		}

		ELT * pNewBuffer = new ELT[nNewLength];

		if(m_pBuffer != 0)
		{
			memcpy(pNewBuffer, m_pBuffer, sizeof(ELT) * CBufferRefT <ELT> :: m_nSize);
			delete [] m_pBuffer;
		}

		CBufferRefT <ELT> :: m_pRef = m_pBuffer = pNewBuffer;
		m_nMaxLength = nNewLength;
	}

	// size
	if( CBufferRefT <ELT> :: m_nSize < nNewSize )
	{
		memset(m_pBuffer + CBufferRefT <ELT> :: m_nSize, fill, sizeof(ELT) * (nNewSize - CBufferRefT <ELT> :: m_nSize));
		CBufferRefT <ELT> :: m_nSize = nNewSize;
	}
}

template <class ELT> inline void CBufferT <ELT> :: Restore(int size)
{
	CBufferRefT <ELT> :: m_nSize = size;
}

template <class ELT> CBufferT <ELT> :: ~CBufferT()
{
	if(m_pBuffer != 0) delete [] m_pBuffer;
}

//
// Context
//
class CContext  
{
public:
	CBufferT <int> m_stack;
	CBufferT <int> m_capturestack, m_captureindex;

public:
	int    m_nCurrentPos;
	int    m_nBeginPos;
	int    m_nLastBeginPos;
	int    m_nParenZindex;

	void * m_pMatchString;
	int    m_pMatchStringLength;
};

//
// Interface
//
class ElxInterface
{
public:
	virtual int Match    (CContext * pContext) = 0;
	virtual int MatchNext(CContext * pContext) = 0;

public:
	virtual ~ElxInterface() {};
};

//
// Alternative
//
class CAlternativeElx : public ElxInterface  
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CAlternativeElx();

public:
	CBufferT <ElxInterface *> m_elxlist;
};

//
// Assert
//
class CAssertElx : public ElxInterface  
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CAssertElx(ElxInterface * pelx, int byes = 1);

public:
	ElxInterface * m_pelx;
	int m_byes;
};

//
// Back reference elx
//
template <class CHART> class CBackrefElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CBackrefElxT(int nnumber, int brightleft, int bignorecase);

public:
	int m_nnumber;
	int m_brightleft;
	int m_bignorecase;
};

//
// Implementation
//
template <class CHART> CBackrefElxT <CHART> :: CBackrefElxT(int nnumber, int brightleft, int bignorecase)
{
	m_nnumber     = nnumber;
	m_brightleft  = brightleft;
	m_bignorecase = bignorecase;
}

template <class CHART> int CBackrefElxT <CHART> :: Match(CContext * pContext)
{
	int index = pContext->m_captureindex[m_nnumber];
	if( index < 0 ) return 0;

	// check
	if( index >= pContext->m_capturestack.GetSize() || pContext->m_capturestack[index] != m_nnumber )
	{
		// to find
		index = pContext->m_capturestack.GetSize() - 4;
		while(index >= 0 && pContext->m_capturestack[index] != m_nnumber) index -= 4;

		// new index
		pContext->m_captureindex[m_nnumber] = index;
		if( index < 0 ) return 0;
	}

	// check enclosed
	int pos1 = pContext->m_capturestack[index + 1];
	int pos2 = pContext->m_capturestack[index + 2];

	if( pos2 < 0 ) pos2 = pContext->m_nCurrentPos;

	// info
	int lpos = pos1 < pos2 ? pos1 : pos2;
	int rpos = pos1 < pos2 ? pos2 : pos1;
	int slen = rpos - lpos;

	const CHART * pcsz = (const CHART *)pContext->m_pMatchString;
	int npos = pContext->m_nCurrentPos;
	int tlen = pContext->m_pMatchStringLength;

	// compare
	int bsucc;
	CBufferRefT <CHART> refstr(pcsz + lpos, slen);

	if( m_brightleft )
	{
		if(npos < slen)
			return 0;

		if(m_bignorecase)
			bsucc = ! refstr.nCompareNoCase(pcsz + (npos - slen));
		else
			bsucc = ! refstr.nCompare      (pcsz + (npos - slen));

		if( bsucc )
		{
			pContext->m_stack.Push(npos);
			pContext->m_nCurrentPos -= slen;
		}
	}
	else
	{
		if(npos + slen > tlen)
			return 0;

		if(m_bignorecase)
			bsucc = ! refstr.nCompareNoCase(pcsz + npos);
		else
			bsucc = ! refstr.nCompare      (pcsz + npos);

		if( bsucc )
		{
			pContext->m_stack.Push(npos);
			pContext->m_nCurrentPos += slen;
		}
	}

	return bsucc;
}

template <class CHART> int CBackrefElxT <CHART> :: MatchNext(CContext * pContext)
{
	int npos = 0;

	pContext->m_stack.Pop(npos);
	pContext->m_nCurrentPos = npos;

	return 0;
}

// RCHART
#ifndef RCHART
	#define RCHART(ch) ((CHART)ch)
#endif

// BOUNDARY_TYPE
enum BOUNDARY_TYPE
{
	BOUNDARY_FILE_BEGIN, // begin of whole text
	BOUNDARY_FILE_END  , // end of whole text
	BOUNDARY_LINE_BEGIN, // begin of line
	BOUNDARY_LINE_END  , // end of line
	BOUNDARY_WORD_BEGIN, // begin of word
	BOUNDARY_WORD_END  , // end of word
	BOUNDARY_WORD_EDGE ,
};

//
// Boundary Elx
//
template <class CHART> class CBoundaryElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CBoundaryElxT(int ntype, int byes = 1);

protected:
	int IsWordChar(CHART ch);

public:
	int m_ntype;
	int m_byes;
};

//
// Implementation
//
template <class CHART> CBoundaryElxT <CHART> :: CBoundaryElxT(int ntype, int byes)
{
	m_ntype = ntype;
	m_byes  = byes;
}

template <class CHART> int CBoundaryElxT <CHART> :: Match(CContext * pContext)
{
	const CHART * pcsz  = (const CHART *)pContext->m_pMatchString;
	int npos = pContext->m_nCurrentPos;
	int tlen = pContext->m_pMatchStringLength;

	CHART chL = npos > 0    ? pcsz[npos - 1] : 0;
	CHART chR = npos < tlen ? pcsz[npos    ] : 0;

	int bsucc = 0;

	switch(m_ntype)
	{
	case BOUNDARY_FILE_BEGIN:
		bsucc = (npos <= 0);
		break;

	case BOUNDARY_FILE_END:
		bsucc = (npos >= tlen);
		break;

	case BOUNDARY_LINE_BEGIN:
		bsucc = (npos <= 0   ) || (chL == RCHART('\n')) || ((chL == RCHART('\r')) && (chR != RCHART('\n')));
		break;

	case BOUNDARY_LINE_END:
		bsucc = (npos >= tlen) || (chR == RCHART('\r')) || ((chR == RCHART('\n')) && (chL != RCHART('\r')));
		break;

	case BOUNDARY_WORD_BEGIN:
		bsucc = ! IsWordChar(chL) &&   IsWordChar(chR);
		break;

	case BOUNDARY_WORD_END:
		bsucc =   IsWordChar(chL) && ! IsWordChar(chR);
		break;

	case BOUNDARY_WORD_EDGE:
		bsucc =   IsWordChar(chL) ?  ! IsWordChar(chR) : IsWordChar(chR);
		break;
	}

	return bsucc;
}

template <class CHART> int CBoundaryElxT <CHART> :: MatchNext(CContext *)
{
	return 0;
}

template <class CHART> inline int CBoundaryElxT <CHART> :: IsWordChar(CHART ch)
{
	return (ch >= RCHART('A') && ch <= RCHART('Z')) || (ch >= RCHART('a') && ch <= RCHART('z')) || (ch >= RCHART('0') && ch <= RCHART('9')) || (ch == RCHART('_'));
}

//
// Bracket
//
class CBracketElx : public ElxInterface  
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CBracketElx(int nnumber, int bright);

public:
	int m_nnumber;
	int m_bright;
};

//
// Deletage
//
class CDelegateElx : public ElxInterface  
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CDelegateElx(int ndata = 0);

public:
	ElxInterface * m_pelx;
	int m_ndata;
};

//
// Empty
//
class CEmptyElx : public ElxInterface  
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CEmptyElx();
};

//
// Global
//
class CGlobalElx : public ElxInterface  
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CGlobalElx();
};

//
// Repeat
//
class CRepeatElx : public ElxInterface  
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CRepeatElx(ElxInterface * pelx, int ntimes);

protected:
	int MatchFixed    (CContext * pContext);
	int MatchNextFixed(CContext * pContext);

public:
	ElxInterface * m_pelx;
	int m_nfixed;
};

//
// Greedy
//
class CGreedyElx : public CRepeatElx  
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CGreedyElx(ElxInterface * pelx, int nmin = 0, int nmax = INT_MAX);

protected:
	int MatchVart    (CContext * pContext);
	int MatchNextVart(CContext * pContext);

public:
	int m_nvart;
};

//
// Independent
//
class CIndependentElx : public ElxInterface  
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CIndependentElx(ElxInterface * pelx);

public:
	ElxInterface * m_pelx;
};

//
// List
//
class CListElx : public ElxInterface  
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CListElx(int brightleft);

public:
	CBufferT <ElxInterface *> m_elxlist;
	int m_brightleft;
};

//
// Posix Elx
//
template <class CHART> class CPosixElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CPosixElxT(const char * posix, int brightleft);

protected:
	static int misblank(int c);

public:
	int (*m_posixfun)(int);
	int m_brightleft;
	int m_byes;
};

//
// Implementation
//
template <class CHART> CPosixElxT <CHART> :: CPosixElxT(const char * posix, int brightleft)
{
	m_brightleft = brightleft;

	if(posix[1] == '^')
	{
		m_byes = 0;
		posix += 2;
	}
	else
	{
		m_byes = 1;
		posix += 1;
	}

	if     (!strncmp(posix, "alnum:", 6)) m_posixfun = isalnum ;
	else if(!strncmp(posix, "alpha:", 6)) m_posixfun = isalpha ;
	else if(!strncmp(posix, "ascii:", 6)) m_posixfun = isascii ;
	else if(!strncmp(posix, "cntrl:", 6)) m_posixfun = iscntrl ;
	else if(!strncmp(posix, "digit:", 6)) m_posixfun = isdigit ;
	else if(!strncmp(posix, "graph:", 6)) m_posixfun = isgraph ;
	else if(!strncmp(posix, "lower:", 6)) m_posixfun = islower ;
	else if(!strncmp(posix, "print:", 6)) m_posixfun = isprint ;
	else if(!strncmp(posix, "punct:", 6)) m_posixfun = ispunct ;
	else if(!strncmp(posix, "space:", 6)) m_posixfun = isspace ;
	else if(!strncmp(posix, "upper:", 6)) m_posixfun = isupper ;
	else if(!strncmp(posix, "xdigit:",7)) m_posixfun = isxdigit;
	else if(!strncmp(posix, "blank:", 6)) m_posixfun = misblank;
	else                                  m_posixfun = 0       ;
}

template <class CHART> int CPosixElxT <CHART> :: misblank(int c)
{
	return c == 0x20 || c == '\t';
}

template <class CHART> int CPosixElxT <CHART> :: Match(CContext * pContext)
{
	int tlen = pContext->m_pMatchStringLength;
	int npos = pContext->m_nCurrentPos;

	// check
	int at   = m_brightleft ? npos - 1 : npos;
	if( at < 0 || at >= tlen )
		return 0;

	CHART ch = ((const CHART *)pContext->m_pMatchString)[at];

	int bsucc = m_posixfun != 0 ? (*m_posixfun)(ch) : 0;

	if( ! m_byes )
		bsucc = ! bsucc;

	if( bsucc )
		pContext->m_nCurrentPos += m_brightleft ? -1 : 1;

	return bsucc;
}

template <class CHART> int CPosixElxT <CHART> :: MatchNext(CContext * pContext)
{
	pContext->m_nCurrentPos -= m_brightleft ? -1 : 1;
	return 0;
}

//
// Possessive
//
class CPossessiveElx : public CGreedyElx  
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CPossessiveElx(ElxInterface * pelx, int nmin = 0, int nmax = INT_MAX);
};

//
// Range Elx
//
template <class CHART> class CRangeElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CRangeElxT(int brightleft, int byes);

public:
	CBufferT <CHART> m_ranges;
	CBufferT <CHART> m_chars;
	CBufferT <ElxInterface *> m_embeds;

public:
	int m_brightleft;
	int m_byes;
};

//
// Implementation
//
template <class CHART> CRangeElxT <CHART> :: CRangeElxT(int brightleft, int byes)
{
	m_brightleft = brightleft;
	m_byes       = byes;
}

template <class CHART> int CRangeElxT <CHART> :: Match(CContext * pContext)
{
	int tlen = pContext->m_pMatchStringLength;
	int npos = pContext->m_nCurrentPos;

	// check
	int at   = m_brightleft ? npos - 1 : npos;
	if( at < 0 || at >= tlen )
		return 0;

	CHART ch = ((const CHART *)pContext->m_pMatchString)[at];
	int bsucc = 0, i;

	// compare
	for(i=0; !bsucc && i<m_ranges.GetSize(); i+=2)
	{
		if(m_ranges[i] <= ch && ch <= m_ranges[i+1]) bsucc = 1;
	}

	for(i=0; !bsucc && i<m_chars.GetSize(); i++)
	{
		if(m_chars[i] == ch) bsucc = 1;
	}

	for(i=0; !bsucc && i<m_embeds.GetSize(); i++)
	{
		if(m_embeds[i]->Match(pContext))
		{
			pContext->m_nCurrentPos = npos;
			bsucc = 1;
		}
	}

	if( ! m_byes )
		bsucc = ! bsucc;

	if( bsucc )
		pContext->m_nCurrentPos += m_brightleft ? -1 : 1;

	return bsucc;
}

template <class CHART> int CRangeElxT <CHART> :: MatchNext(CContext * pContext)
{
	pContext->m_nCurrentPos -= m_brightleft ? -1 : 1;
	return 0;
}

//
// Reluctant
//
class CReluctantElx : public CRepeatElx  
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CReluctantElx(ElxInterface * pelx, int nmin = 0, int nmax = INT_MAX);

protected:
	int MatchVart    (CContext * pContext);
	int MatchNextVart(CContext * pContext);

public:
	int m_nvart;
};

//
// String Elx
//
template <class CHART> class CStringElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext);
	int MatchNext(CContext * pContext);

public:
	CStringElxT(const CHART * fixed, int nlength, int brightleft, int bignorecase);

public:
	CBufferT <CHART> m_szPattern;
	int m_brightleft;
	int m_bignorecase;
};

//
// Implementation
//
template <class CHART> CStringElxT <CHART> :: CStringElxT(const CHART * fixed, int nlength, int brightleft, int bignorecase) : m_szPattern(fixed, nlength)
{
	m_brightleft  = brightleft;
	m_bignorecase = bignorecase;
}

template <class CHART> int CStringElxT <CHART> :: Match(CContext * pContext)
{
	const CHART * pcsz  = (const CHART *)pContext->m_pMatchString;
	int npos = pContext->m_nCurrentPos;
	int tlen = pContext->m_pMatchStringLength;
	int slen = m_szPattern.GetSize();

	int bsucc;

	if(m_brightleft)
	{
		if(npos < slen)
			return 0;

		if(m_bignorecase)
			bsucc = ! m_szPattern.nCompareNoCase(pcsz + (npos - slen));
		else
			bsucc = ! m_szPattern.nCompare      (pcsz + (npos - slen));

		if( bsucc )
			pContext->m_nCurrentPos -= slen;
	}
	else
	{
		if(npos + slen > tlen)
			return 0;

		if(m_bignorecase)
			bsucc = ! m_szPattern.nCompareNoCase(pcsz + npos);
		else
			bsucc = ! m_szPattern.nCompare      (pcsz + npos);

		if( bsucc )
			pContext->m_nCurrentPos += slen;
	}

	return bsucc;
}

template <class CHART> int CStringElxT <CHART> :: MatchNext(CContext * pContext)
{
	int slen = m_szPattern.GetSize();

	if(m_brightleft)
		pContext->m_nCurrentPos += slen;
	else
		pContext->m_nCurrentPos -= slen;

	return 0;
}

//
// MatchResult
//
class MatchResult
{
public:
	int IsMatched();

public:
	int GetStart();
	int GetEnd  ();

public:
	int MaxGroupNumber();
	int GetGroupStart(int nGroupNumber);
	int GetGroupEnd  (int nGroupNumber);

public:
	MatchResult(CContext * pContext, int nMaxNumber = -1)
	{
		if(pContext != 0)
		{
			m_result.Prepare(nMaxNumber * 2 + 3, -1);

			// matched
			m_result[0] = 1;
			m_result[1] = nMaxNumber;

			for(int n = 0; n <= nMaxNumber; n++)
			{
				int index = pContext->m_captureindex[n];
				if( index < 0 ) continue;

				// check
				if( index >= pContext->m_capturestack.GetSize() || pContext->m_capturestack[index] != n )
				{
					// to find
					index = pContext->m_capturestack.GetSize() - 4;
					while(index >= 0 && pContext->m_capturestack[index] != n) index -= 4;

					// new index
					pContext->m_captureindex[n] = index;
					if( index < 0 ) continue;
				}

				// check enclosed
				int pos1 = pContext->m_capturestack[index + 1];
				int pos2 = pContext->m_capturestack[index + 2];

				// info
				m_result[n*2 + 2] = pos1 < pos2 ? pos1 : pos2;
				m_result[n*2 + 3] = pos1 < pos2 ? pos2 : pos1;
			}
		}
	}

	MatchResult & operator = (const MatchResult &);
	inline operator int() { return IsMatched(); }

public:
	CBufferT <int> m_result;
};

// Stocked Elx IDs
enum STOCKELX_ID_DEFINES
{
	STOCKELX_EMPTY = 0,

	///////////////////////

	STOCKELX_DOT_ALL,
	STOCKELX_DOT_NOT_ALL,

	STOCKELX_WORD,
	STOCKELX_WORD_NOT,

	STOCKELX_SPACE,
	STOCKELX_SPACE_NOT,

	STOCKELX_DIGITAL,
	STOCKELX_DIGITAL_NOT,

	//////////////////////

	STOCKELX_DOT_ALL_RIGHTLEFT,
	STOCKELX_DOT_NOT_ALL_RIGHTLEFT,

	STOCKELX_WORD_RIGHTLEFT,
	STOCKELX_WORD_RIGHTLEFT_NOT,

	STOCKELX_SPACE_RIGHTLEFT,
	STOCKELX_SPACE_RIGHTLEFT_NOT,

	STOCKELX_DIGITAL_RIGHTLEFT,
	STOCKELX_DIGITAL_RIGHTLEFT_NOT,

	/////////////////////

	STOCKELX_COUNT
};

// REGEX_FLAGS
#ifndef _REGEX_FLAGS_DEFINED
	enum REGEX_FLAGS
	{
		NO_FLAG        = 0,
		SINGLELINE     = 0x01,
		MULTILINE      = 0x02,
		GLOBAL         = 0x04,
		IGNORECASE     = 0x08,
		RIGHTTOLEFT    = 0x10,
		EXTENDED       = 0x20,
	};
	#define _REGEX_FLAGS_DEFINED
#endif

//
// Builder T
//
template <class CHART> class CBuilderT
{
// Methods
public:
	ElxInterface * Build(const CBufferRefT <CHART> & pattern, int flags);
	void Clear();

public:
	 CBuilderT();
	~CBuilderT();

// Public Attributes
public:
	ElxInterface * m_pTopElx;
	int            m_nFlags;
	int            m_nMaxNumber;

	CBufferT <ElxInterface *> m_objlist;
	CBufferT <ElxInterface *> m_grouplist;
	CBufferT <CDelegateElx *> m_recursivelist;

// CHART_INFO
protected:
	struct CHART_INFO
	{
	public:
		CHART ch;
		int   type;
		int   pos;
		int   len;

	public:
		CHART_INFO(CHART c, int t, int p = 0, int l = 0) { ch = c; type = t; pos = p; len = l;    }
		inline int operator == (const CHART_INFO & ci)   { return ch == ci.ch && type == ci.type; }
		inline int operator != (const CHART_INFO & ci)   { return ! operator == (ci);             }
	};

protected:
	static unsigned int Hex2Int(const CHART * pcsz, int length, int & used);
	void MoveNext();
	int  GetNext2();

	ElxInterface * BuildAlternative(int vaflags);
	ElxInterface * BuildList       (int & flags);
	ElxInterface * BuildRepeat     (int & flags);
	ElxInterface * BuildSimple     (int & flags);
	ElxInterface * BuildCharset    (int & flags);
	ElxInterface * BuildRecursive  (int & flags);
	ElxInterface * BuildBoundary   (int & flags);
	ElxInterface * BuildBackref    (int & flags);

	ElxInterface * GetStockElx     (int nStockId);
	ElxInterface * Keep(ElxInterface * pElx);

// Private Attributes
protected:
	CBufferRefT <CHART> m_pattern;
	CHART_INFO prev, curr, next, nex2;
	int m_nNextPos;
	int m_nCharsetDepth;

	ElxInterface * m_pStockElxs[STOCKELX_COUNT];
};

//
// Implementation
//
template <class CHART> CBuilderT <CHART> :: CBuilderT() : m_pattern(0, 0), prev(0, 0), curr(0, 0), next(0, 0), nex2(0, 0)
{
	Clear();
}

template <class CHART> CBuilderT <CHART> :: ~CBuilderT()
{
	Clear();
}

template <class CHART> ElxInterface * CBuilderT <CHART> :: Build(const CBufferRefT <CHART> & pattern, int flags)
{
	// init
	m_pattern       = pattern;
	m_nNextPos      = 0;
	m_nCharsetDepth = 0;
	m_nMaxNumber    = 0;
	m_nFlags        = flags;

	m_grouplist    .Restore(0);
	m_recursivelist.Restore(0);

	int i;
	for(i=0; i<3; i++) MoveNext();

	// build
	m_pTopElx = BuildAlternative(flags);

	// group 0
	m_grouplist.Prepare(0);
	m_grouplist[0] = m_pTopElx;

	// connect recursive
	for(i=0; i<m_recursivelist.GetSize(); i++)
	{
		if( m_recursivelist[i]->m_ndata < m_grouplist.GetSize() )
			m_recursivelist[i]->m_pelx = m_grouplist[m_recursivelist[i]->m_ndata];
	}

	return m_pTopElx;
}

template <class CHART> void CBuilderT <CHART> :: Clear()
{
	for(int i=0; i<m_objlist.GetSize(); i++)
	{
		delete m_objlist[i];
	}

	m_objlist.Restore(0);
	m_pTopElx = 0;

	memset(m_pStockElxs, 0, sizeof(m_pStockElxs));
}

//
// hex to int
//
template <class CHART> unsigned int CBuilderT <CHART> :: Hex2Int(const CHART * pcsz, int length, int & used)
{
	unsigned int result = 0;
	int & i = used;

	for(i=0; i<length; i++)
	{
		if(pcsz[i] >= RCHART('0') && pcsz[i] <= RCHART('9'))
			result = (result << 4) + (pcsz[i] - RCHART('0'));
		else if(pcsz[i] >= RCHART('A') && pcsz[i] <= RCHART('F'))
			result = (result << 4) + (0x0A + (pcsz[i] - RCHART('A')));
		else if(pcsz[i] >= RCHART('a') && pcsz[i] <= RCHART('f'))
			result = (result << 4) + (0x0A + (pcsz[i] - RCHART('a')));
		else
			break;
	}

	return result;
}

template <class CHART> inline ElxInterface * CBuilderT <CHART> :: Keep(ElxInterface * pelx)
{
	m_objlist.Push(pelx);
	return pelx;
}

template <class CHART> void CBuilderT <CHART> :: MoveNext()
{
	// forwards
	prev = curr;
	curr = next;
	next = nex2;

	// get nex2
	while( ! GetNext2() ) {};
}

template <class CHART> int CBuilderT <CHART> :: GetNext2()
{
	int delta = 1;
	nex2     = CHART_INFO(0, 0, m_nNextPos, 0);

	CHART ch  = m_pattern[m_nNextPos];

	switch(ch)
	{
	case RCHART('\\'):
		{
			CHART ch1 = m_pattern[m_nNextPos+1];

			// backref
			if(ch1 >= RCHART('0') && ch1 <= RCHART('9'))
			{
				nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
				break;
			}

			// escape
			delta     = 2;

			switch(ch1)
			{
			case RCHART('A'):
			case RCHART('Z'):
			case RCHART('w'):
			case RCHART('W'):
			case RCHART('s'):
			case RCHART('S'):
			case RCHART('B'):
			case RCHART('d'):
			case RCHART('D'):
				nex2 = CHART_INFO(ch1, 1, m_nNextPos, delta);
				break;

			case RCHART('b'):
				if(m_nCharsetDepth > 0)
					nex2 = CHART_INFO('\b', 0, m_nNextPos, delta);
				else
					nex2 = CHART_INFO(ch1, 1, m_nNextPos, delta);
				break;

			/*
			case RCHART('<'):
			case RCHART('>'):
				if(m_nCharsetDepth > 0)
					nex2 = CHART_INFO(ch1, 0, m_nNextPos, delta);
				else
					nex2 = CHART_INFO(ch1, 1, m_nNextPos, delta);
				break;
			*/

			case RCHART('x'):
				{
					int red = 0;
					unsigned int ch2 = Hex2Int(m_pattern.GetBuffer() + m_nNextPos + 2, 2, red);

					delta += red;

					if(red > 0)
						nex2 = CHART_INFO(RCHART(ch2), 0, m_nNextPos, delta);
					else
						nex2 = CHART_INFO(ch1, 0, m_nNextPos, delta);
				}
				break;

			case RCHART('u'):
				{
					int red = 0;
					unsigned int ch2 = Hex2Int(m_pattern.GetBuffer() + m_nNextPos + 2, 4, red);

					delta += red;

					if(red > 0)
						nex2 = CHART_INFO(RCHART(ch2), 0, m_nNextPos, delta);
					else
						nex2 = CHART_INFO(ch1, 0, m_nNextPos, delta);
				}
				break;

			case RCHART('a'): nex2 = CHART_INFO(RCHART('\a'), 0, m_nNextPos, delta); break;
			case RCHART('f'): nex2 = CHART_INFO(RCHART('\f'), 0, m_nNextPos, delta); break;
			case RCHART('n'): nex2 = CHART_INFO(RCHART('\n'), 0, m_nNextPos, delta); break;
			case RCHART('r'): nex2 = CHART_INFO(RCHART('\r'), 0, m_nNextPos, delta); break;
			case RCHART('t'): nex2 = CHART_INFO(RCHART('\t'), 0, m_nNextPos, delta); break;
			case RCHART('v'): nex2 = CHART_INFO(RCHART('\v'), 0, m_nNextPos, delta); break;
			case RCHART('e'): nex2 = CHART_INFO(RCHART( 27 ), 0, m_nNextPos, delta); break;

			case RCHART('G'):  // skip '\G'
				if(m_nCharsetDepth > 0)
				{
					m_nNextPos += 2;
					return 0;
				}
				else
				{
					nex2 = CHART_INFO(ch1, 1, m_nNextPos, delta);
					break;
				}

			case 0:
				if(m_nNextPos+1 >= m_pattern.GetSize())
				{
					delta = 1;
					nex2 = CHART_INFO(ch , 0, m_nNextPos, delta);
				}
				else
					nex2 = CHART_INFO(ch1, 0, m_nNextPos, delta); // common '\0' char
				break;

			default:
				nex2 = CHART_INFO(ch1, 0, m_nNextPos, delta);
				break;
			}
		}
		break;

	case RCHART('*'):
	case RCHART('+'):
	case RCHART('?'):
	case RCHART('.'):
	case RCHART('{'):
	case RCHART('}'):
	case RCHART(')'):
	case RCHART('|'):
	case RCHART('$'):
		if(m_nCharsetDepth > 0)
			nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
		else
			nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
		break;

	case RCHART('-'):
		if(m_nCharsetDepth > 0)
			nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
		else
			nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
		break;

	case RCHART('('):
		if(m_nCharsetDepth > 0)
		{
			nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
		}
		else
		{
			CHART ch1 = m_pattern[m_nNextPos+1];
			CHART ch2 = m_pattern[m_nNextPos+2];

			// skip remark
			if(ch1 == RCHART('?') && ch2 == RCHART('#'))
			{
				m_nNextPos += 2;
				while(m_nNextPos < m_pattern.GetSize())
				{
					if(m_pattern[m_nNextPos] == RCHART(')'))
						break;

					m_nNextPos ++;
				}

				if(m_pattern[m_nNextPos] == RCHART(')'))
				{
					m_nNextPos ++;

					// get next nex2
					return 0;
				}
			}
			else
			{
				nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
			}
		}
		break;

	case RCHART('#'):
		if(m_nFlags & EXTENDED)
		{
			// skip remark
			m_nNextPos ++;

			while(m_nNextPos < m_pattern.GetSize())
			{
				if(m_pattern[m_nNextPos] == RCHART('\n') || m_pattern[m_nNextPos] == RCHART('\r'))
					break;

				m_nNextPos ++;
			}

			// get next nex2
			return 0;
		}
		else
		{
			nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
		}
		break;

	case RCHART(' '):
	case RCHART('\f'):
	case RCHART('\n'):
	case RCHART('\r'):
	case RCHART('\t'):
	case RCHART('\v'):
		if(m_nFlags & EXTENDED)
		{
			m_nNextPos ++;

			// get next nex2
			return 0;
		}
		else
		{
			nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
		}
		break;

	case RCHART('['):
		m_nCharsetDepth ++;
		nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
		break;

	case RCHART(']'):
		m_nCharsetDepth --;
		nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
		break;

	case RCHART(':'):
		if(next == CHART_INFO(RCHART('['), 1))
			nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
		else
			nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
		break;

	case RCHART('^'):
		if(m_nCharsetDepth == 0 || next == CHART_INFO(RCHART('['), 1) || (curr == CHART_INFO(RCHART('['), 1) && next == CHART_INFO(RCHART(':'), 1)))
			nex2 = CHART_INFO(ch, 1, m_nNextPos, delta);
		else
			nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
		break;

	case 0:
		if(m_nNextPos >= m_pattern.GetSize())
			nex2 = CHART_INFO(ch, 1, m_nNextPos, delta); // end of string
		else
			nex2 = CHART_INFO(ch, 0, m_nNextPos, delta); // common '\0' char
		break;

	default:
		nex2 = CHART_INFO(ch, 0, m_nNextPos, delta);
		break;
	}

	m_nNextPos += delta;

	return 1;
}

template <class CHART> ElxInterface * CBuilderT <CHART> :: GetStockElx(int nStockId)
{
	ElxInterface ** pStockElxs = m_pStockElxs;

	// check
	if(nStockId < 0 || nStockId >= STOCKELX_COUNT)
		return GetStockElx(0);

	// create if no
	if(pStockElxs[nStockId] == 0)
	{
		switch(nStockId)
		{
		case STOCKELX_EMPTY:
			pStockElxs[nStockId] = Keep(new CEmptyElx());
			break;

		case STOCKELX_WORD:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (0, 1));

				pRange->m_ranges.Push(RCHART('A')); pRange->m_ranges.Push(RCHART('Z'));
				pRange->m_ranges.Push(RCHART('a')); pRange->m_ranges.Push(RCHART('z'));
				pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));
				pRange->m_chars .Push(RCHART('_'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_WORD_NOT:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (0, 0));

				pRange->m_ranges.Push(RCHART('A')); pRange->m_ranges.Push(RCHART('Z'));
				pRange->m_ranges.Push(RCHART('a')); pRange->m_ranges.Push(RCHART('z'));
				pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));
				pRange->m_chars .Push(RCHART('_'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_DOT_ALL:
			pStockElxs[nStockId] = Keep(new CRangeElxT <CHART> (0, 0));
			break;

		case STOCKELX_DOT_NOT_ALL:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (0, 0));

				pRange->m_chars .Push(RCHART('\n'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_SPACE:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (0, 1));

				pRange->m_chars .Push(RCHART(' '));
				pRange->m_chars .Push(RCHART('\t'));
				pRange->m_chars .Push(RCHART('\r'));
				pRange->m_chars .Push(RCHART('\n'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_SPACE_NOT:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (0, 0));

				pRange->m_chars .Push(RCHART(' '));
				pRange->m_chars .Push(RCHART('\t'));
				pRange->m_chars .Push(RCHART('\r'));
				pRange->m_chars .Push(RCHART('\n'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_DIGITAL:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (0, 1));

				pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_DIGITAL_NOT:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (0, 0));

				pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_WORD_RIGHTLEFT:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (1, 1));

				pRange->m_ranges.Push(RCHART('A')); pRange->m_ranges.Push(RCHART('Z'));
				pRange->m_ranges.Push(RCHART('a')); pRange->m_ranges.Push(RCHART('z'));
				pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));
				pRange->m_chars .Push(RCHART('_'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_WORD_RIGHTLEFT_NOT:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (1, 0));

				pRange->m_ranges.Push(RCHART('A')); pRange->m_ranges.Push(RCHART('Z'));
				pRange->m_ranges.Push(RCHART('a')); pRange->m_ranges.Push(RCHART('z'));
				pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));
				pRange->m_chars .Push(RCHART('_'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_DOT_ALL_RIGHTLEFT:
			pStockElxs[nStockId] = Keep(new CRangeElxT <CHART> (1, 0));
			break;

		case STOCKELX_DOT_NOT_ALL_RIGHTLEFT:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (1, 0));

				pRange->m_chars .Push(RCHART('\n'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_SPACE_RIGHTLEFT:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (1, 1));

				pRange->m_chars .Push(RCHART(' '));
				pRange->m_chars .Push(RCHART('\t'));
				pRange->m_chars .Push(RCHART('\r'));
				pRange->m_chars .Push(RCHART('\n'));
				pRange->m_chars .Push(RCHART('\f'));
				pRange->m_chars .Push(RCHART('\v'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_SPACE_RIGHTLEFT_NOT:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (1, 0));

				pRange->m_chars .Push(RCHART(' '));
				pRange->m_chars .Push(RCHART('\t'));
				pRange->m_chars .Push(RCHART('\r'));
				pRange->m_chars .Push(RCHART('\n'));
				pRange->m_chars .Push(RCHART('\f'));
				pRange->m_chars .Push(RCHART('\v'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_DIGITAL_RIGHTLEFT:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (1, 1));

				pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));

				pStockElxs[nStockId] = pRange;
			}
			break;

		case STOCKELX_DIGITAL_RIGHTLEFT_NOT:
			{
				CRangeElxT <CHART> * pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (1, 0));

				pRange->m_ranges.Push(RCHART('0')); pRange->m_ranges.Push(RCHART('9'));

				pStockElxs[nStockId] = pRange;
			}
			break;
		}
	}

	// return
	return pStockElxs[nStockId];
}

template <class CHART> ElxInterface * CBuilderT <CHART> :: BuildAlternative(int vaflags)
{
	if(curr == CHART_INFO(0, 1))
		return GetStockElx(STOCKELX_EMPTY);

	// flag instance
	int flags = vaflags;

	// first part
	ElxInterface * pAlternativeOne = BuildList(flags);

	// check alternative
	if(curr == CHART_INFO(RCHART('|'), 1))
	{
		CAlternativeElx * pAlternative = (CAlternativeElx *)Keep(new CAlternativeElx());
		pAlternative->m_elxlist.Push(pAlternativeOne);

		// loop
		while(curr == CHART_INFO(RCHART('|'), 1))
		{
			// skip '|' itself
			MoveNext();

			pAlternativeOne = BuildList(flags);
			pAlternative->m_elxlist.Push(pAlternativeOne);
		}

		return pAlternative;
	}

	return pAlternativeOne;
}

template <class CHART> ElxInterface * CBuilderT <CHART> :: BuildList(int & flags)
{
	if(curr == CHART_INFO(0, 1) || curr == CHART_INFO(RCHART('|'), 1) || curr == CHART_INFO(RCHART(')'), 1))
		return GetStockElx(STOCKELX_EMPTY);

	// first
	ElxInterface * pListOne = BuildRepeat(flags);

	if(curr != CHART_INFO(0, 1) && curr != CHART_INFO(RCHART('|'), 1) && curr != CHART_INFO(RCHART(')'), 1))
	{
		CListElx * pList = (CListElx *)Keep(new CListElx(flags & RIGHTTOLEFT));
		pList->m_elxlist.Push(pListOne);

		while(curr != CHART_INFO(0, 1) && curr != CHART_INFO(RCHART('|'), 1) && curr != CHART_INFO(RCHART(')'), 1))
		{
			pListOne = BuildRepeat(flags);

			// add
			pList->m_elxlist.Push(pListOne);
		}

		return pList;
	}

	return pListOne;
}

template <class CHART> ElxInterface * CBuilderT <CHART> :: BuildRepeat(int & flags)
{
	// simple
	ElxInterface * pSimple = BuildSimple(flags);

	if(curr.type == 0) return pSimple;

	// is quantifier or not
	int bIsQuantifier = 1;

	// quantifier range
	unsigned int nMin = 0, nMax = 0;
	char comma;

	switch(curr.ch)
	{
	case RCHART('{'):
		{
			CBufferT <char> re;

			// skip '{'
			MoveNext();

			// copy
			while(curr != CHART_INFO(0, 1) && curr != CHART_INFO(RCHART('}'), 1))
			{
				re.Append((char)curr.ch, 1);
				MoveNext();
			}

			// skip '}'
			MoveNext();

			// read
			int red = sscanf(re.GetBuffer() ? re.GetBuffer() : "", "%u%1s%u", &nMin, &comma, &nMax);

			// check
			if(red  <=  1 ) nMax = nMin;
			if(red  ==  2 ) nMax = INT_MAX;
			if(nMax < nMin) nMax = nMin;
		}
		break;

	case RCHART('?'):
		nMin = 0;
		nMax = 1;

		// skip '?'
		MoveNext();
		break;

	case RCHART('*'):
		nMin = 0;
		nMax = INT_MAX;

		// skip '*'
		MoveNext();
		break;

	case RCHART('+'):
		nMin = 1;
		nMax = INT_MAX;

		// skip '+'
		MoveNext();
		break;

	default:
		bIsQuantifier = 0;
		break;
	}

	// do quantify
	if(bIsQuantifier)
	{
		// 0 times
		if(nMax == 0)
			return GetStockElx(STOCKELX_EMPTY);

		// fixed times
		if(nMin == nMax)
		{
			if(curr == CHART_INFO(RCHART('?'), 1) || curr == CHART_INFO(RCHART('+'), 1))
				MoveNext();

			return Keep(new CRepeatElx(pSimple, nMin));
		}

		// range times
		if(curr == CHART_INFO(RCHART('?'), 1))
		{
			MoveNext();
			return Keep(new CReluctantElx(pSimple, nMin, nMax));
		}
		else if(curr == CHART_INFO(RCHART('+'), 1))
		{
			MoveNext();
			return Keep(new CPossessiveElx(pSimple, nMin, nMax));
		}
		else
		{
			return Keep(new CGreedyElx(pSimple, nMin, nMax));
		}
	}

	return pSimple;
}

template <class CHART> ElxInterface * CBuilderT <CHART> :: BuildSimple(int & flags)
{
	CBufferT <CHART> fixed;

	while(curr != CHART_INFO(0, 1))
	{
		if(curr.type == 0)
		{
			if(next == CHART_INFO(RCHART('{'), 1) || next == CHART_INFO(RCHART('?'), 1) || next == CHART_INFO(RCHART('*'), 1) || next == CHART_INFO(RCHART('+'), 1))
			{
				if(fixed.GetSize() == 0)
				{
					fixed.Append(curr.ch, 1);
					MoveNext();
				}

				break;
			}
			else
			{
				fixed.Append(curr.ch, 1);
				MoveNext();
			}
		}
		else if(curr.type == 1)
		{
			CHART vch = curr.ch;

			// end of simple
			if(vch == RCHART(')') || vch == RCHART('|'))
				break;

			// has fixed already
			if(fixed.GetSize() > 0)
				break;

			// left parentheses
			if(vch == RCHART('('))
			{
				return BuildRecursive(flags);
			}

			// char set
			if( vch == RCHART('[') || vch == RCHART('.') || vch == RCHART('w') || vch == RCHART('W') ||
				vch == RCHART('s') || vch == RCHART('S') || vch == RCHART('d') || vch == RCHART('D')
			)
			{
				return BuildCharset(flags);
			}

			// boundary
			if( vch == RCHART('^') || vch == RCHART('$') || vch == RCHART('A') || vch == RCHART('Z') ||
				vch == RCHART('b') || vch == RCHART('B') || vch == RCHART('G') // vch == RCHART('<') || vch == RCHART('>')
			)
			{
				return BuildBoundary(flags);
			}

			// backref
			if(vch == RCHART('\\'))
			{
				return BuildBackref(flags);
			}

			// treat vchar as char
			fixed.Append(curr.ch, 1);
			MoveNext();
		}
	}

	if(fixed.GetSize() > 0)
		return Keep(new CStringElxT <CHART> (fixed.GetBuffer(), fixed.GetSize(), flags & RIGHTTOLEFT, flags & IGNORECASE));
	else
		return GetStockElx(STOCKELX_EMPTY);
}

template <class CHART> ElxInterface * CBuilderT <CHART> :: BuildCharset(int & flags)
{
	// char
	CHART ch = curr.ch;

	// skip
	MoveNext();

	switch(ch)
	{
	case RCHART('.'):
		return GetStockElx(
			flags & RIGHTTOLEFT ?
			((flags & SINGLELINE) ? STOCKELX_DOT_ALL_RIGHTLEFT : STOCKELX_DOT_NOT_ALL_RIGHTLEFT) :
			((flags & SINGLELINE) ? STOCKELX_DOT_ALL : STOCKELX_DOT_NOT_ALL)
		);

	case RCHART('w'):
		return GetStockElx(flags & RIGHTTOLEFT ? STOCKELX_WORD_RIGHTLEFT : STOCKELX_WORD);

	case RCHART('W'):
		return GetStockElx(flags & RIGHTTOLEFT ? STOCKELX_WORD_RIGHTLEFT_NOT : STOCKELX_WORD_NOT);

	case RCHART('s'):
		return GetStockElx(flags & RIGHTTOLEFT ? STOCKELX_SPACE_RIGHTLEFT : STOCKELX_SPACE);

	case RCHART('S'):
		return GetStockElx(flags & RIGHTTOLEFT ? STOCKELX_SPACE_RIGHTLEFT_NOT : STOCKELX_SPACE_NOT);

	case RCHART('d'):
		return GetStockElx(flags & RIGHTTOLEFT ? STOCKELX_DIGITAL_RIGHTLEFT : STOCKELX_DIGITAL);

	case RCHART('D'):
		return GetStockElx(flags & RIGHTTOLEFT ? STOCKELX_DIGITAL_RIGHTLEFT_NOT : STOCKELX_DIGITAL_NOT);

	case RCHART('['):
		{
			CRangeElxT <CHART> * pRange;

			// create
			if(curr == CHART_INFO(RCHART(':'), 1))
			{
				CBufferT <char> posix;

				do {
					posix.Append((char)curr.ch, 1);
					MoveNext();
				}
				while(curr.ch != RCHART(0) && curr != CHART_INFO(RCHART(']'), 1));

				MoveNext(); // skip ']'

				// posix
				return Keep(new CPosixElxT <CHART> (posix.GetBuffer(), flags & RIGHTTOLEFT));
			}
			else if(curr == CHART_INFO(RCHART('^'), 1))
			{
				MoveNext(); // skip '^'
				pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (flags & RIGHTTOLEFT, 0));
			}
			else
			{
				pRange = (CRangeElxT <CHART> *)Keep(new CRangeElxT <CHART> (flags & RIGHTTOLEFT, 1));
			}

			// parse
			while(curr != CHART_INFO(0, 1) && curr != CHART_INFO(RCHART(']'), 1))
			{
				ch = curr.ch;

				if(curr.type == 1 && (
					ch == RCHART('.') || ch == RCHART('w') || ch == RCHART('W') || ch == RCHART('s') || ch == RCHART('S') || ch == RCHART('d') || ch == RCHART('D') ||
					(ch == RCHART('[') && next ==CHART_INFO(RCHART(':'), 1))
				))
				{
					pRange->m_embeds.Push(BuildCharset(flags));
				}
				else if(next == CHART_INFO(RCHART('-'), 1) && nex2.type == 0)
				{
					pRange->m_ranges.Push(ch); pRange->m_ranges.Push(nex2.ch);

					// next
					MoveNext();
					MoveNext();
					MoveNext();
				}
				else
				{
					pRange->m_chars.Push(ch);

					// next
					MoveNext();
				}
			}

			// skip ']'
			MoveNext();

			return pRange;
		}
	}

	return GetStockElx(STOCKELX_EMPTY);
}

template <class CHART> ElxInterface * CBuilderT <CHART> :: BuildRecursive(int & flags)
{
	ElxInterface * pElx;

	// skip '('
	MoveNext();

	if(curr == CHART_INFO(RCHART('?'), 1))
	{
		// skip '?'
		MoveNext();

		int bNegative = 0;

		switch(curr.ch)
		{
		case RCHART('!'):
			bNegative = 1;

		case RCHART('='):
			{
				MoveNext(); // skip '!' or '='
				pElx = Keep(new CAssertElx(BuildAlternative(flags & ~RIGHTTOLEFT), !bNegative));
			}
			break;

		case RCHART('<'):
			MoveNext(); // skip '<'
			switch(curr.ch)
			{
			case RCHART('!'):
				bNegative = 1;

			case RCHART('='):
				MoveNext(); // skip '!' or '='

			default: // treat '(?<xxx)' as '(?<=xxx)'
				pElx = Keep(new CAssertElx(BuildAlternative(flags | RIGHTTOLEFT), !bNegative));
				break;
			}
			break;

		case RCHART('>'):
			{
				MoveNext(); // skip '>'
				pElx = Keep(new CIndependentElx(BuildAlternative(flags)));
			}
			break;

		case RCHART('R'):
			{
				MoveNext(); // skip 'R'

				CBufferT <char> rto;
				while(curr.ch != RCHART(0) && curr.ch != RCHART(')'))
				{
					rto.Append((char)curr.ch, 1);
					MoveNext();
				}

				unsigned int rtono = 0;
				sscanf(rto.GetBuffer() ? rto.GetBuffer() : "", "%u", &rtono);

				CDelegateElx * pDelegate = (CDelegateElx *)Keep(new CDelegateElx(rtono));

				m_recursivelist.Push(pDelegate);

				pElx = pDelegate;
			}
			break;

		default:
			{
				// flag
				int newflags = flags;
				while(curr != CHART_INFO(0, 1) && curr.ch != RCHART(':') && curr.ch != RCHART(')'))
				{
					int tochange = 0;

					switch(curr.ch)
					{
					case RCHART('i'):
					case RCHART('I'):
						tochange = IGNORECASE;
						break;

					case RCHART('s'):
					case RCHART('S'):
						tochange = SINGLELINE;
						break;

					case RCHART('m'):
					case RCHART('M'):
						tochange = MULTILINE;
						break;

					case RCHART('g'):
					case RCHART('G'):
						tochange = GLOBAL;
						break;

					case RCHART('-'):
						bNegative = 1;
						break;
					}

					if(bNegative)
						newflags &= ~tochange;
					else
						newflags |=  tochange;

					// move to next char
					MoveNext();
				}

				if(curr.ch == RCHART(':'))
				{
					// skip ':'
					MoveNext();

					pElx = BuildAlternative(newflags);
				}
				else
				{
					// change parent flags
					flags = newflags;

					pElx = GetStockElx(STOCKELX_EMPTY);
				}
			}
			break;
		}
	}
	else
	{
		// backref
		CListElx * pList = (CListElx *)Keep(new CListElx(flags & RIGHTTOLEFT));

		int nThisBackref = ++ m_nMaxNumber;

		// left
		pList->m_elxlist.Push(Keep(new CBracketElx(nThisBackref, flags & RIGHTTOLEFT ? 1 : 0)));

		// for recursive
		m_grouplist.Prepare(nThisBackref);

		pElx = BuildAlternative(flags);
		pList->m_elxlist.Push(pElx);

		m_grouplist[nThisBackref] = pElx;

		// right
		MoveNext(); // skip ')' 

		pList->m_elxlist.Push(Keep(new CBracketElx(nThisBackref, flags & RIGHTTOLEFT ? 0 : 1)));

		return pList;
	}

	MoveNext(); // skip ')'

	return pElx;
}

template <class CHART> ElxInterface * CBuilderT <CHART> :: BuildBoundary(int & flags)
{
	// char
	CHART ch = curr.ch;

	// skip
	MoveNext();

	switch(ch)
	{
	case RCHART('^'):
		return Keep(new CBoundaryElxT <CHART> ((flags & MULTILINE) ? BOUNDARY_LINE_BEGIN : BOUNDARY_FILE_BEGIN));

	case RCHART('$'):
		return Keep(new CBoundaryElxT <CHART> ((flags & MULTILINE) ? BOUNDARY_LINE_END : BOUNDARY_FILE_END));

	case RCHART('b'):
		return Keep(new CBoundaryElxT <CHART> (BOUNDARY_WORD_EDGE));

	case RCHART('B'):
		return Keep(new CBoundaryElxT <CHART> (BOUNDARY_WORD_EDGE, 0));

	case RCHART('A'):
		return Keep(new CBoundaryElxT <CHART> (BOUNDARY_FILE_BEGIN));

	case RCHART('Z'):
		return Keep(new CBoundaryElxT <CHART> (BOUNDARY_FILE_END));

	case RCHART('G'):
		if(flags & GLOBAL)
			return Keep(new CGlobalElx());
		else
			return GetStockElx(STOCKELX_EMPTY);

	default:
		return GetStockElx(STOCKELX_EMPTY);
	}
}

template <class CHART> ElxInterface * CBuilderT <CHART> :: BuildBackref(int & flags)
{
	// skip '\\'
	MoveNext();

	unsigned int nbackref = 0;

	for(int i=0; i<3; i++)
	{
		if(curr.ch >= RCHART('0') && curr.ch <= RCHART('9'))
			nbackref = nbackref * 10 + (curr.ch - RCHART('0'));
		else
			break;

		MoveNext();
	}

	return Keep(new CBackrefElxT <CHART> (nbackref, flags & RIGHTTOLEFT, flags & IGNORECASE));
}

//
// Regexp
//
template <class CHART> class CRegexpT
{
public:
	CRegexpT(const CHART * pattern = 0, int flags = 0);
	CRegexpT(const CHART * pattern, int length, int flags);
	void Compile(const CHART * pattern, int flags = 0);
	void Compile(const CHART * pattern, int length, int flags);

public:
	MatchResult MatchExact(const CHART * tstring);
	MatchResult MatchExact(const CHART * tstring, int length);
	MatchResult Match(const CHART * tstring, int start = -1);
	MatchResult Match(const CHART * tstring, int length, int start);
	MatchResult Match(CContext * pContext);
	CContext * PrepareMatch(const CHART * tstring, int start = -1);
	CContext * PrepareMatch(const CHART * tstring, int length, int start);
	CHART * Replace(const CHART * tstring, const CHART * replaceto, int start = -1, int ntimes = -1, MatchResult * result = 0);
	CHART * Replace(const CHART * tstring, int string_length, const CHART * replaceto, int to_length, int & result_length, int start = -1, int ntimes = -1, MatchResult * result = 0);

public:
	static void ReleaseString (CHART    * tstring );
	static void ReleaseContext(CContext * pContext);

public:
	CBuilderT <CHART> m_builder;
};

//
// Implementation
//
template <class CHART> CRegexpT <CHART> :: CRegexpT(const CHART * pattern, int flags)
{
	Compile(pattern, CBufferRefT<CHART>(pattern).GetSize(), flags);
}

template <class CHART> CRegexpT <CHART> :: CRegexpT(const CHART * pattern, int length, int flags)
{
	Compile(pattern, length, flags);
}

template <class CHART> inline void CRegexpT <CHART> :: Compile(const CHART * pattern, int flags)
{
	Compile(pattern, CBufferRefT<CHART>(pattern).GetSize(), flags);
}

template <class CHART> void CRegexpT <CHART> :: Compile(const CHART * pattern, int length, int flags)
{
	m_builder.Clear();
	if(pattern != 0) m_builder.Build(CBufferRefT<CHART>(pattern, length), flags);
}

template <class CHART> inline MatchResult CRegexpT <CHART> :: MatchExact(const CHART * tstring)
{
	return MatchExact(tstring, CBufferRefT<CHART>(tstring).GetSize());
}

template <class CHART> MatchResult CRegexpT <CHART> :: MatchExact(const CHART * tstring, int length)
{
	if(m_builder.m_pTopElx == 0)
		return 0;

	// info
	int endpos = 0;

	CContext context;

	context.m_nParenZindex  = 0;
	context.m_nLastBeginPos = -1;
	context.m_pMatchString  = (void*)tstring;
	context.m_pMatchStringLength = length;

	if(m_builder.m_nFlags & RIGHTTOLEFT)
	{
		context.m_nBeginPos   = length;
		context.m_nCurrentPos = length;
		endpos = 0;
	}
	else
	{
		context.m_nBeginPos   = 0;
		context.m_nCurrentPos = 0;
		endpos = length;
	}

	context.m_captureindex.Prepare(m_builder.m_nMaxNumber, -1);
	context.m_captureindex[0] = 0;
	context.m_capturestack.Push(0);
	context.m_capturestack.Push(context.m_nCurrentPos);
	context.m_capturestack.Push(-1);
	context.m_capturestack.Push(-1);

	// match
	if( ! m_builder.m_pTopElx->Match( &context ) )
		return 0;
	else
	{
		while( context.m_nCurrentPos != endpos )
		{
			if( ! m_builder.m_pTopElx->MatchNext( &context ) )
				return 0;
			else
			{
				if( context.m_nLastBeginPos == context.m_nBeginPos && context.m_nBeginPos == context.m_nCurrentPos )
					return 0;
				else
					context.m_nLastBeginPos = context.m_nCurrentPos;
			}
		}

		// end pos
		context.m_capturestack[2] = context.m_nCurrentPos;

		return MatchResult( &context, m_builder.m_nMaxNumber );
	}
}

template <class CHART> inline MatchResult CRegexpT <CHART> :: Match(const CHART * tstring, int start)
{
	return Match(tstring, CBufferRefT<CHART>(tstring).GetSize(), start);
}

template <class CHART> MatchResult CRegexpT <CHART> :: Match(const CHART * tstring, int length, int start)
{
	if(m_builder.m_pTopElx == 0)
		return 0;

	CContext context;

	context.m_nParenZindex  =  0;
	context.m_nLastBeginPos = -1;
	context.m_pMatchString  = (void*)tstring;
	context.m_pMatchStringLength = length;

	if(start < 0)
	{
		if(m_builder.m_nFlags & RIGHTTOLEFT)
		{
			context.m_nBeginPos   = length;
			context.m_nCurrentPos = length;
		}
		else
		{
			context.m_nBeginPos   = 0;
			context.m_nCurrentPos = 0;
		}
	}
	else
	{
		context.m_nBeginPos   = start;
		context.m_nCurrentPos = start;
	}

	return Match( &context );
}

template <class CHART> MatchResult CRegexpT <CHART> :: Match(CContext * pContext)
{
	if(m_builder.m_pTopElx == 0)
		return 0;

	int endpos, delta;

	if(m_builder.m_nFlags & RIGHTTOLEFT)
	{
		endpos = -1;
		delta  = -1;
	}
	else
	{
		endpos = pContext->m_pMatchStringLength + 1;
		delta  = 1;
	}

	pContext->m_captureindex.Prepare(m_builder.m_nMaxNumber, -1);
	pContext->m_captureindex[0] = 0;

	while(pContext->m_nCurrentPos != endpos)
	{
		pContext->m_stack       .Restore(0);
		pContext->m_capturestack.Restore(0);
		pContext->m_capturestack.Push(0);
		pContext->m_capturestack.Push(pContext->m_nCurrentPos);
		pContext->m_capturestack.Push(-1);
		pContext->m_capturestack.Push(-1);

		if( m_builder.m_pTopElx->Match( pContext ) )
		{
			// zero width
			if( pContext->m_nLastBeginPos == pContext->m_nBeginPos && pContext->m_nBeginPos == pContext->m_nCurrentPos )
			{
				pContext->m_nCurrentPos += delta;
				continue;
			}

			// save pos
			pContext->m_nLastBeginPos   = pContext->m_nBeginPos;
			pContext->m_nBeginPos       = pContext->m_nCurrentPos;
			pContext->m_capturestack[2] = pContext->m_nCurrentPos;

			// return
			return MatchResult( pContext, m_builder.m_nMaxNumber );
		}
		else
		{
			pContext->m_nCurrentPos += delta;
		}
	}

	return 0;
}

template <class CHART> inline CContext * CRegexpT <CHART> :: PrepareMatch(const CHART * tstring, int start)
{
	return PrepareMatch(tstring, CBufferRefT<CHART>(tstring).GetSize(), start);
}

template <class CHART> CContext * CRegexpT <CHART> :: PrepareMatch(const CHART * tstring, int length, int start)
{
	if(m_builder.m_pTopElx == 0)
		return 0;

	CContext * pContext = new CContext();

	pContext->m_nParenZindex  =  0;
	pContext->m_nLastBeginPos = -1;
	pContext->m_pMatchString  = (void*)tstring;
	pContext->m_pMatchStringLength = length;

	if(start < 0)
	{
		if(m_builder.m_nFlags & RIGHTTOLEFT)
		{
			pContext->m_nBeginPos   = length;
			pContext->m_nCurrentPos = length;
		}
		else
		{
			pContext->m_nBeginPos   = 0;
			pContext->m_nCurrentPos = 0;
		}
	}
	else
	{
		pContext->m_nBeginPos   = start;
		pContext->m_nCurrentPos = start;
	}

	return pContext;
}

template <class CHART> CHART * CRegexpT <CHART> :: Replace(const CHART * tstring, const CHART * replaceto, int start, int ntimes, MatchResult * result)
{
	int result_length = 0;
	return Replace(tstring, CBufferRefT<CHART>(tstring).GetSize(), replaceto, CBufferRefT<CHART>(replaceto).GetSize(), result_length, start, ntimes, result);
}

template <class CHART> CHART * CRegexpT <CHART> :: Replace(const CHART * tstring, int string_length, const CHART * replaceto, int to_length, int & result_length, int start, int ntimes, MatchResult * remote_result)
{
	typedef CBufferRefT <CHART> StringRef;

	MatchResult local_result(0), * result = remote_result ? remote_result : & local_result;

	if(m_builder.m_pTopElx == 0) return 0;

	// Prepare
	CContext * pContext = PrepareMatch(tstring, string_length, start);

	int flags     = m_builder.m_nFlags;
	int lastIndex = (flags & RIGHTTOLEFT) ? string_length : 0;
	int endpos    = (flags & RIGHTTOLEFT) ? 0 : string_length;
	int toIndex   = 0, toLastIndex = 0;
	int i, ntime;

	CBufferT <StringRef *> buffer, buf;

	static const CHART rtoptn[] = { RCHART('\\'), RCHART('$' ), RCHART('('), RCHART('?'), RCHART(':'), RCHART('[' ), RCHART('$' ), RCHART('&' ), RCHART('`' ), RCHART('\''), RCHART('+'), RCHART('_' ), RCHART('\\'), RCHART('d'), RCHART(']'), RCHART(')' ), RCHART('\0') };
	static int   rtoptnlen      = StringRef(rtoptn).GetSize();
	static CRegexpT <CHART> rtoreg(rtoptn, rtoptnlen, 0);

	// Match
	for(ntime = 0; ntimes < 0 || ntime < ntimes; ntime ++)
	{
		(*result) = Match(pContext);

		if( ! result->IsMatched() )
			break;

		toIndex = toLastIndex;

		// before
		if( flags & RIGHTTOLEFT )
		{
			int distance = lastIndex - result->GetEnd();
			if( distance )
			{
				buffer.Push(new StringRef(tstring + result->GetEnd(), distance));
				toIndex -= distance;
			}
			lastIndex = result->GetStart();
		}
		else
		{
			int distance = result->GetStart() - lastIndex;
			if( distance )
			{
				buffer.Push(new StringRef(tstring + lastIndex, distance));
				toIndex += distance;
			}
			lastIndex = result->GetEnd();
		}

		toLastIndex = toIndex;

		// middle
		CContext * pCtx = rtoreg.PrepareMatch(replaceto, to_length, -1);
		int lastI = 0;

		buf.Restore(0);

		while(1)
		{
			MatchResult res = rtoreg.Match(pCtx);

			if( ! res.IsMatched() )
				break;

			// before
			int distance = res.GetStart() - lastI;
			if( distance )
			{
				buf.Push(new StringRef(replaceto + lastI, distance));
			}
			lastI = res.GetStart();

			// middle
			int delta = 2, nmatch = 0, nzindex = -1, i = 0;

			switch(replaceto[res.GetStart() + 1])
			{
			case RCHART('$'):
				buf.Push(new StringRef(rtoptn + 1, 1)); // '$' itself
				break;

			case RCHART('&'):
				buf.Push(new StringRef(tstring + result->GetStart(), result->GetEnd() - result->GetStart()));
				break;

			case RCHART('`'):
				buf.Push(new StringRef(tstring, result->GetStart()));
				break;

			case RCHART('\''):
				buf.Push(new StringRef(tstring + result->GetEnd(), string_length - result->GetEnd()));
				break;

			case RCHART('+'):
				for(i = 0; i<pContext->m_capturestack.GetSize(); i+= 4)
				{
					if(pContext->m_capturestack[i+3] > nzindex)
					{
						nmatch  = pContext->m_capturestack[i];
						nzindex = pContext->m_capturestack[i + 3];
					}
				}
				buf.Push(new StringRef(tstring + result->GetGroupStart(nmatch), result->GetGroupEnd(nmatch) - result->GetGroupStart(nmatch)));
				break;

			case RCHART('_'):
				buf.Push(new StringRef(tstring, string_length));
				break;

			default:
				nmatch = 0;
				for(delta=1; delta<=3; delta++)
				{
					CHART ch = replaceto[lastI + delta];

					if(ch < RCHART('0') || ch > RCHART('9'))
						break;

					nmatch = nmatch * 10 + (ch - RCHART('0'));
				}

				if(nmatch > m_builder.m_nMaxNumber)
				{
					while(nmatch > m_builder.m_nMaxNumber)
					{
						nmatch /= 10;
						delta --;
					}

					if(nmatch == 0)
					{
						delta = 1;
					}
				}

				if(delta == 1)
					buf.Push(new StringRef(rtoptn + 1, 1)); // '$' itself
				else
					buf.Push(new StringRef(tstring + result->GetGroupStart(nmatch), result->GetGroupEnd(nmatch) - result->GetGroupStart(nmatch)));
				break;
			}

			lastI += delta;
		}

		// after
		if(lastI < to_length)
			buf.Push(new StringRef(replaceto + lastI, to_length - lastI));

		// append to buffer
		if(flags & RIGHTTOLEFT)
		{
			for(i=buf.GetSize()-1; i>=0; i--)
			{
				buffer.Push(buf[i]);
				toLastIndex -= buf[i]->GetSize();
			}
		}
		else
		{
			for(i=0; i<buf.GetSize(); i++)
			{
				buffer.Push(buf[i]);
				toLastIndex += buf[i]->GetSize();
			}
		}

		rtoreg.ReleaseContext(pCtx);
	}

	// after
	if(flags & RIGHTTOLEFT)
	{
		if(endpos < lastIndex) buffer.Push(new StringRef(tstring + endpos, lastIndex - endpos));
	}
	else
	{
		if(lastIndex < endpos) buffer.Push(new StringRef(tstring + lastIndex, endpos - lastIndex));
	}

	ReleaseContext(pContext);

	// join string
	result_length = 0;
	for(i=0; i<buffer.GetSize(); i++) result_length += buffer[i]->GetSize();

	CBufferT <CHART> result_string;
	result_string.Prepare(result_length);
	result_string.Restore(0);

	if(flags & RIGHTTOLEFT)
	{
		for(i=buffer.GetSize()-1; i>=0; i--)
		{
			result_string.Append(buffer[i]->GetBuffer(), buffer[i]->GetSize());
			delete buffer[i];
		}
	}
	else
	{
		for(i=0; i<buffer.GetSize(); i++)
		{
			result_string.Append(buffer[i]->GetBuffer(), buffer[i]->GetSize());
			delete buffer[i];
		}
	}

	result_string[result_length] = 0;

	result->m_result.Append(toIndex < toLastIndex ? toIndex : toLastIndex, 2);
	result->m_result.Append(toIndex > toLastIndex ? toIndex : toLastIndex);
	result->m_result.Append(ntime);

	return result_string.Detach();
}

template <class CHART> inline void CRegexpT <CHART> :: ReleaseString(CHART * tstring)
{
	if(tstring != 0) delete [] tstring;
}

template <class CHART> inline void CRegexpT <CHART> :: ReleaseContext(CContext * pContext)
{
	if(pContext != 0) delete pContext;
}

//
// All implementations
//
inline CAlternativeElx::CAlternativeElx()
{
}

inline int CAlternativeElx::Match(CContext * pContext)
{
	if(m_elxlist.GetSize() == 0)
		return 1;

	// try all
	for(int n = 0; n < m_elxlist.GetSize(); n++)
	{
		if(m_elxlist[n]->Match(pContext))
		{
			pContext->m_stack.Push(n);
			return 1;
		}
	}

	return 0;
}

inline int CAlternativeElx::MatchNext(CContext * pContext)
{
	if(m_elxlist.GetSize() == 0)
		return 0;

	int n = 0;

	// recall prev
	pContext->m_stack.Pop(n);

	// prev
	if(m_elxlist[n]->MatchNext(pContext))
	{
		pContext->m_stack.Push(n);
		return 1;
	}
	else
	{
		// try rest
		for(n++; n < m_elxlist.GetSize(); n++)
		{
			if(m_elxlist[n]->Match(pContext))
			{
				pContext->m_stack.Push(n);
				return 1;
			}
		}

		return 0;
	}
}

// assertx.cpp: implementation of the CAssertElx class.
//
inline CAssertElx::CAssertElx(ElxInterface * pelx, int byes)
{
	m_pelx = pelx;
	m_byes = byes;
}

inline int CAssertElx::Match(CContext * pContext)
{
	int nbegin = pContext->m_nCurrentPos;
	int nsize  = pContext->m_stack.GetSize();
	int ncsize = pContext->m_capturestack.GetSize();
	int bsucc;

	// match
	if( m_byes )
		bsucc =   m_pelx->Match(pContext);
	else
		bsucc = ! m_pelx->Match(pContext);

	// status
	pContext->m_stack.Restore(nsize);
	pContext->m_nCurrentPos = nbegin;

	if( bsucc )
		pContext->m_stack.Push(ncsize);
	else
		pContext->m_capturestack.Restore(ncsize);

	return bsucc;
}

inline int CAssertElx::MatchNext(CContext * pContext)
{
	int ncsize = 0;

	pContext->m_stack.Pop(ncsize);
	pContext->m_capturestack.Restore(ncsize);

	return 0;
}

// bracketx.cpp: implementation of the CBracketElx class.
//
inline CBracketElx::CBracketElx(int nnumber, int bright)
{
	m_nnumber = nnumber;
	m_bright  = bright;
}

inline int CBracketElx::Match(CContext * pContext)
{
	if( ! m_bright )
	{
		pContext->m_captureindex.Prepare(m_nnumber, -1);
		int index = pContext->m_captureindex[m_nnumber];

		// check
		if(index > 0 && index < pContext->m_capturestack.GetSize() && pContext->m_capturestack[index+2] < 0)
		{
			pContext->m_capturestack[index+3] --;
			return 1;
		}

		// save
		pContext->m_captureindex[m_nnumber] = pContext->m_capturestack.GetSize();

		pContext->m_capturestack.Push(m_nnumber);
		pContext->m_capturestack.Push(pContext->m_nCurrentPos);
		pContext->m_capturestack.Push(-1);
		pContext->m_capturestack.Push( 0); // z-index
	}
	else
	{
		// check
		int index = pContext->m_captureindex[m_nnumber];

		if(pContext->m_capturestack[index + 3] < 0)
		{
			pContext->m_capturestack[index + 3] ++;
			return 1;
		}

		// save
		pContext->m_capturestack[index + 2] = pContext->m_nCurrentPos;
		pContext->m_capturestack[index + 3] = pContext->m_nParenZindex ++;
	}

	return 1;
}

inline int CBracketElx::MatchNext(CContext * pContext)
{
	int index = pContext->m_captureindex[m_nnumber];

	if( ! m_bright )
	{
		if(pContext->m_capturestack[index + 3] < 0)
		{
			pContext->m_capturestack[index + 3] ++;
			return 0;
		}

		pContext->m_capturestack.Restore(pContext->m_capturestack.GetSize() - 4);
	}
	else
	{
		if(pContext->m_capturestack[index + 3] < 0)
		{
			pContext->m_capturestack[index + 3] --;
			return 0;
		}

		pContext->m_capturestack[index + 2] = -1;
		pContext->m_capturestack[index + 3] =  0;
	}

	return 0;
}

// delegatx.cpp: implementation of the CDelegateElx class.
//
inline CDelegateElx::CDelegateElx(int ndata)
{
	m_pelx  = 0;
	m_ndata = ndata;
}

inline int CDelegateElx::Match(CContext * pContext)
{
	if(m_pelx != 0)
		return m_pelx->Match(pContext);
	else
		return 1;
}

inline int CDelegateElx::MatchNext(CContext * pContext)
{
	if(m_pelx != 0)
		return m_pelx->MatchNext(pContext);
	else
		return 0;
}

// emptyelx.cpp: implementation of the CEmptyElx class.
//
inline CEmptyElx::CEmptyElx()
{
}

inline int CEmptyElx::Match(CContext *)
{
	return 1;
}

inline int CEmptyElx::MatchNext(CContext *)
{
	return 0;
}

// globalx.cpp: implementation of the CGlobalElx class.
//
inline CGlobalElx::CGlobalElx()
{
}

inline int CGlobalElx::Match(CContext * pContext)
{
	return pContext->m_nCurrentPos == pContext->m_nBeginPos;
}

inline int CGlobalElx::MatchNext(CContext *)
{
	return 0;
}

// greedelx.cpp: implementation of the CGreedyElx class.
//
inline CGreedyElx::CGreedyElx(ElxInterface * pelx, int nmin, int nmax) : CRepeatElx(pelx, nmin)
{
	m_nvart = nmax - nmin;
}

inline int CGreedyElx::Match(CContext * pContext)
{
	if( ! MatchFixed(pContext) )
		return 0;

	while( ! MatchVart(pContext) )
	{
		if( ! MatchNextFixed(pContext) )
			return 0;
	}

	return 1;
}

inline int CGreedyElx::MatchNext(CContext * pContext)
{
	if( MatchNextVart(pContext) )
		return 1;

	if( ! MatchNextFixed(pContext) )
		return 0;

	while( ! MatchVart(pContext) )
	{
		if( ! MatchNextFixed(pContext) )
			return 0;
	}

	return 1;
}

inline int CGreedyElx::MatchVart(CContext * pContext)
{
	int n = 0;

	while(n < m_nvart && m_pelx->Match(pContext))
		n ++;

	pContext->m_stack.Push(n);

	return 1;
}

inline int CGreedyElx::MatchNextVart(CContext * pContext)
{
	int n = 0;

	pContext->m_stack.Pop(n);

	if(n == 0)
		return 0;

	if(m_pelx->MatchNext(pContext))
	{
		while(n < m_nvart && m_pelx->Match(pContext))
			n ++;
	}
	else
	{
		n --;
	}

	pContext->m_stack.Push(n);

	return 1;
}

// indepelx.cpp: implementation of the CIndependentElx class.
//
inline CIndependentElx::CIndependentElx(ElxInterface * pelx)
{
	m_pelx = pelx;
}

inline int CIndependentElx::Match(CContext * pContext)
{
	int nbegin = pContext->m_nCurrentPos;
	int nsize  = pContext->m_stack.GetSize();
	int ncsize = pContext->m_capturestack.GetSize();

	// match
	int bsucc  = m_pelx->Match(pContext);

	// status
	pContext->m_stack.Restore(nsize);

	if( bsucc )
	{
		pContext->m_stack.Push(nbegin);
		pContext->m_stack.Push(ncsize);
	}

	return bsucc;
}

inline int CIndependentElx::MatchNext(CContext * pContext)
{
	int nbegin = 0, ncsize = 0;

	pContext->m_stack.Pop(ncsize);
	pContext->m_stack.Pop(nbegin);

	pContext->m_capturestack.Restore(ncsize);
	pContext->m_nCurrentPos = nbegin;

	return 0;
}

// listelx.cpp: implementation of the CListElx class.
//
inline CListElx::CListElx(int brightleft)
{
	m_brightleft = brightleft;
}

inline int CListElx::Match(CContext * pContext)
{
	if(m_elxlist.GetSize() == 0)
		return 1;

	// prepare
	int bol = m_brightleft ? m_elxlist.GetSize() : -1;
	int stp = m_brightleft ? -1 : 1;
	int eol = m_brightleft ? -1 : m_elxlist.GetSize();

	// from first
	int n = bol + stp;

	// match all
	while(n != eol)
	{
		if(m_elxlist[n]->Match(pContext))
		{
			n += stp;
		}
		else
		{
			n -= stp;

			while(n != bol && ! m_elxlist[n]->MatchNext(pContext))
				n -= stp;

			if(n != bol)
				n += stp;
			else
				return 0;
		}
	}

	return 1;
}

inline int CListElx::MatchNext(CContext * pContext)
{
	if(m_elxlist.GetSize() == 0)
		return 0;

	// prepare
	int bol = m_brightleft ? m_elxlist.GetSize() : -1;
	int stp = m_brightleft ? -1 : 1;
	int eol = m_brightleft ? -1 : m_elxlist.GetSize();

	// from last
	int n = eol - stp;

	while(n != bol && ! m_elxlist[n]->MatchNext(pContext))
		n -= stp;

	if(n != bol)
		n += stp;
	else
		return 0;

	// match rest
	while(n != eol)
	{
		if(m_elxlist[n]->Match(pContext))
		{
			n += stp;
		}
		else
		{
			n -= stp;

			while(n != bol && ! m_elxlist[n]->MatchNext(pContext))
				n -= stp;

			if(n != bol)
				n += stp;
			else
				return 0;
		}
	}

	return 1;
}

// mresult.cpp: implementation of the MatchResult class.
//
inline int MatchResult::IsMatched()
{
	return m_result.At(0, 0);
}

inline int MatchResult::MaxGroupNumber()
{
	return m_result.At(1, 0);
}

inline int MatchResult::GetStart()
{
	return m_result.At(2, -1);
}

inline int MatchResult::GetEnd()
{
	return m_result.At(3, -1);
}

inline int MatchResult::GetGroupStart(int nGroupNumber)
{
	return m_result.At(2 + nGroupNumber * 2, -1);
}

inline int MatchResult::GetGroupEnd(int nGroupNumber)
{
	return m_result.At(2 + nGroupNumber * 2 + 1, -1);
}

inline MatchResult & MatchResult::operator = (const MatchResult & result)
{
	m_result.Restore(0);
	if(result.m_result.GetSize() > 0) m_result.Append(result.m_result.GetBuffer(), result.m_result.GetSize());

	return *this;
}

// posselx.cpp: implementation of the CPossessiveElx class.
//
inline CPossessiveElx::CPossessiveElx(ElxInterface * pelx, int nmin, int nmax) : CGreedyElx(pelx, nmin, nmax)
{
}

inline int CPossessiveElx::Match(CContext * pContext)
{
	int nbegin = pContext->m_nCurrentPos;
	int nsize  = pContext->m_stack.GetSize();
	int ncsize = pContext->m_capturestack.GetSize();
	int bsucc  = 1;

	// match
	if( ! MatchFixed(pContext) )
	{
		bsucc = 0;
	}
	else
	{
		while( ! MatchVart(pContext) )
		{
			if( ! MatchNextFixed(pContext) )
			{
				bsucc = 0;
				break;
			}
		}
	}

	// status
	pContext->m_stack.Restore(nsize);

	if( bsucc )
	{
		pContext->m_stack.Push(nbegin);
		pContext->m_stack.Push(ncsize);
	}

	return bsucc;
}

inline int CPossessiveElx::MatchNext(CContext * pContext)
{
	int nbegin = 0, ncsize = 0;

	pContext->m_stack.Pop(ncsize);
	pContext->m_stack.Pop(nbegin);

	pContext->m_capturestack.Restore(ncsize);
	pContext->m_nCurrentPos = nbegin;

	return 0;
}

// reluctx.cpp: implementation of the CReluctantElx class.
//
inline CReluctantElx::CReluctantElx(ElxInterface * pelx, int nmin, int nmax) : CRepeatElx(pelx, nmin)
{
	m_nvart = nmax - nmin;
}

inline int CReluctantElx::Match(CContext * pContext)
{
	if( ! MatchFixed(pContext) )
		return 0;

	while( ! MatchVart(pContext) )
	{
		if( ! MatchNextFixed(pContext) )
			return 0;
	}

	return 1;
}

inline int CReluctantElx::MatchNext(CContext * pContext)
{
	if( MatchNextVart(pContext) )
		return 1;

	if( ! MatchNextFixed(pContext) )
		return 0;

	while( ! MatchVart(pContext) )
	{
		if( ! MatchNextFixed(pContext) )
			return 0;
	}

	return 1;
}

inline int CReluctantElx::MatchVart(CContext * pContext)
{
	pContext->m_stack.Push(0);

	return 1;
}

inline int CReluctantElx::MatchNextVart(CContext * pContext)
{
	int n = 0;

	pContext->m_stack.Pop(n);

	if(n < m_nvart && m_pelx->Match(pContext))
	{
		n ++;
	}
	else
	{
		while(n > 0 && ! m_pelx->MatchNext(pContext))
			n --;
	}

	if(n > 0)
	{
		pContext->m_stack.Push(n);
		return 1;
	}
	else
	{
		return 0;
	}
}

// repeatx.cpp: implementation of the CRepeatElx class.
//
inline CRepeatElx::CRepeatElx(ElxInterface * pelx, int ntimes)
{
	m_pelx   = pelx;
	m_nfixed = ntimes;
}

inline int CRepeatElx::Match(CContext * pContext)
{
	return MatchFixed(pContext);
}

inline int CRepeatElx::MatchNext(CContext * pContext)
{
	return MatchNextFixed(pContext);
}

inline int CRepeatElx::MatchFixed(CContext * pContext)
{
	if(m_nfixed == 0)
		return 1;

	int n = 0;

	while(n < m_nfixed)
	{
		if(m_pelx->Match(pContext))
		{
			n ++;
		}
		else
		{
			n --;

			while(n >= 0 && ! m_pelx->MatchNext(pContext))
				n --;

			if(n >= 0)
				n ++;
			else
				return 0;
		}
	}

	return 1;
}

inline int CRepeatElx::MatchNextFixed(CContext * pContext)
{
	if(m_nfixed == 0)
		return 0;

	// from last
	int n = m_nfixed - 1;

	while(n >= 0 && ! m_pelx->MatchNext(pContext))
		n --;

	if(n >= 0)
		n ++;
	else
		return 0;

	// match rest
	while(n < m_nfixed)
	{
		if(m_pelx->Match(pContext))
		{
			n ++;
		}
		else
		{
			n --;

			while(n >= 0 && ! m_pelx->MatchNext(pContext))
				n --;

			if(n >= 0)
				n ++;
			else
				return 0;
		}
	}

	return 1;
}

// Regexp
typedef CRegexpT <char> CRegexpA;
typedef CRegexpT <unsigned short> CRegexpW;

#if defined(_UNICODE) || defined(UNICODE)
	typedef CRegexpW CRegexp;
#else
	typedef CRegexpA CRegexp;
#endif

#endif//__DEELX_REGEXP__H__
