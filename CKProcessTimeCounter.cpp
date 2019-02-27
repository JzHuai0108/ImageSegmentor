/******************************************************************
 * Modul name : CKProcessTimeCounter.cpp
 * Author     : Tamas Somogyi (based on a Linux/C code written by Zoltan Kato)
 * Copyright  : GNU General Public License www.gnu.org/copyleft/gpl.html
 * Description:
 * Platform independent timer class to measure elapsed CPU time of
 * arbitrary portions of a C++ code. 
 *
 * $Id: CKProcessTimeCounter.cpp,v 1.1 2004/12/13 12:07:28 kato Exp $
 * $Revision: 1.1 $
 * $State: Exp $
 * $Log: CKProcessTimeCounter.cpp,v $
 * Revision 1.1  2004/12/13 12:07:28  kato
 * Initial revision
 *
 * 
 *****************************************************************/
#ifndef lint
static char rcsid_CKProcessTimeCounter_cpp[]="$Id: CKProcessTimeCounter.cpp,v 1.1 2004/12/13 12:07:28 kato Exp $";
#endif
/**
 * Title:       CKProcessTimeCounter.cpp
 * Description: 
 * Copyright:   Copyright (c) 2002
 * @author      Tamas Somogyi
 * @version     1.0
 */
#include "stdafx.h"
#include "CKProcessTimeCounter.h"
#include <string.h>

#ifndef _WIN32 // Linux/Unix version
#include <sys/time.h>
#include <sys/resource.h>


#define TIMEVAL2MS(tv)  ((double)(tv).tv_sec*1000.0+(double)(tv).tv_usec/1000.0)


CKProcessTimeCounter::CKProcessTimeCounter (const char* strTimerName)
{
	if (strTimerName == 0)
		*m_strTimerName = '\0';
	else
		strcpy (m_strTimerName, strTimerName);
	m_pLastStartTime = new struct rusage;
	Reset ();
}


CKProcessTimeCounter::~CKProcessTimeCounter ()
{
	delete (rusage*) m_pLastStartTime;
}


void CKProcessTimeCounter::Reset ()
{
	struct rusage ru;
	getrusage (RUSAGE_SELF, &ru);
	m_fLastUserTime = m_fInactUserTime = TIMEVAL2MS(ru.ru_utime);
	m_fLastSysTime = m_fInactSysTime = TIMEVAL2MS(ru.ru_stime);
}


void CKProcessTimeCounter::Start ()
{
	getrusage (RUSAGE_SELF, (rusage*) m_pLastStartTime);
	m_fInactUserTime += TIMEVAL2MS(((struct rusage*)m_pLastStartTime)->ru_utime)-m_fLastUserTime;
	m_fInactSysTime += TIMEVAL2MS(((struct rusage*)m_pLastStartTime)->ru_stime)-m_fLastSysTime;
}


void CKProcessTimeCounter::Stop ()
{
	struct rusage ru;
	getrusage (RUSAGE_SELF, &ru);
    m_fLastUserTime = TIMEVAL2MS(ru.ru_utime);
    m_fLastSysTime = TIMEVAL2MS(ru.ru_stime);
}


double CKProcessTimeCounter::GetElapsedLastTimeMs () const
{
	return m_fLastUserTime-TIMEVAL2MS(((struct rusage*)m_pLastStartTime)->ru_utime)+m_fLastSysTime-TIMEVAL2MS(((struct rusage*)m_pLastStartTime)->ru_stime);
}


double CKProcessTimeCounter::GetElapsedTimeMs () const
{
	return m_fLastUserTime-m_fInactUserTime+m_fLastSysTime-m_fInactSysTime;
}
#else  // Windows version

#include "windows.h"


static void DunselFunction ()
{ 
	return;
}


void CKProcessTimeCounter::DetermineTimer ()
{
	QueryPerformanceFrequency ((LARGE_INTEGER*) m_piPerfFreq);
	// We can use hires timer, determine overhead
	m_iOverheadTicks = 200;
	for (int i=0; i < 20; i++ )
	{
		LARGE_INTEGER b;
		QueryPerformanceCounter (&b);
		DunselFunction ();
		LARGE_INTEGER e;
		QueryPerformanceCounter (&e);
		int iTicks = e.LowPart-b.LowPart;
		if (iTicks >= 0 && iTicks < m_iOverheadTicks)
        m_iOverheadTicks = iTicks;
	}
	// See if Freq fits in 32 bits; if not lose some precision
	m_iPerfFreqAdjust = 0;
	int iHigh32 = ((LARGE_INTEGER*) m_piPerfFreq)->HighPart;
	while (iHigh32)
	{
		iHigh32 >>= 1;
		m_iPerfFreqAdjust++;
	}
}


CKProcessTimeCounter::CKProcessTimeCounter (const char* strTimerName)
{
	if (strTimerName == 0)
		*m_strTimerName = '\0';
	else
		strcpy (m_strTimerName, strTimerName);
	m_piStartTime = new LARGE_INTEGER;
	m_piPerfFreq = new LARGE_INTEGER;
#ifndef _DEBUG
	SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);
#endif
	DetermineTimer ();
	Reset ();
}


CKProcessTimeCounter::~CKProcessTimeCounter ()
{
	delete m_piStartTime;
	delete m_piPerfFreq;
}


void CKProcessTimeCounter::Reset ()
{
	m_fTime = 0;
}


void CKProcessTimeCounter::Start ()
{
//#ifndef _DEBUG
//	SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);
//#endif

	QueryPerformanceCounter ((LARGE_INTEGER*) m_piStartTime);
}


void CKProcessTimeCounter::Stop ()
{
	LARGE_INTEGER iEndTime;
	QueryPerformanceCounter (&iEndTime);

//	SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_NORMAL);

	// Results are 64 bits but we only do 32
	LARGE_INTEGER iStartTime = *((LARGE_INTEGER*) m_piStartTime);
	unsigned int iHigh32 = iEndTime.HighPart-iStartTime.HighPart;
	int iReduceMag = 0;
	while (iHigh32)
	{
		iHigh32 >>= 1;
		iReduceMag++;
	}
	int iOht = m_iOverheadTicks;
	LARGE_INTEGER iFreq = *((LARGE_INTEGER*) m_piPerfFreq);
	if (m_iPerfFreqAdjust || iReduceMag)
	{
		if (m_iPerfFreqAdjust > iReduceMag)
			iReduceMag = m_iPerfFreqAdjust;
		iStartTime.QuadPart = Int64ShrlMod32 (iStartTime.QuadPart, iReduceMag);
		iEndTime.QuadPart = Int64ShrlMod32 (iEndTime.QuadPart, iReduceMag);
		iFreq.QuadPart = Int64ShrlMod32 (iFreq.QuadPart, iReduceMag);
		iOht >>= iReduceMag;
	}

    // Reduced numbers to 32 bits, now can do the math
    if (iFreq.LowPart == 0)
        m_fLastTime = 0.0;
    else
        m_fLastTime = ((double)(iEndTime.LowPart-iStartTime.LowPart-iOht))/iFreq.LowPart*1000;
	
	m_fTime += m_fLastTime;
}


double CKProcessTimeCounter::GetElapsedLastTimeMs () const
{
	return m_fLastTime;
}


double CKProcessTimeCounter::GetElapsedTimeMs () const
{
	return m_fTime;
}
#endif //_WIN32


#include <iostream>

void CKProcessTimeCounter::PrintElapsedTime () const
{
	std::cout << m_strTimerName << " - Elapsed time [ms]:" << GetElapsedTimeMs () << "\n";
}


void CKProcessTimeCounter::PrintElapsedLastTime () const
{
	std::cout << m_strTimerName << " - Elapsed last time [ms]:" << GetElapsedLastTimeMs () << "\n";
}


void CKProcessTimeCounter::Etap (const char* strEtapName)
{
	Stop ();
	std::cout << m_strTimerName << "|" << strEtapName << " - Elapsed time [ms]:" << GetElapsedTimeMs () << "\n";
	Start ();
}


void CKProcessTimeCounter::LastEtap (const char* strEtapName)
{
	Stop ();
	std::cout << m_strTimerName << "(" << strEtapName << ") - Elapsed last time [ms]:" << GetElapsedLastTimeMs () << "\n";
	Start ();
}
