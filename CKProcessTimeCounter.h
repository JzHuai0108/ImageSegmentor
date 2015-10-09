/******************************************************************
 * Modul name : CKProcessTimeCounter.h
 * Author     : Tamas Somogyi (based on a Linux/C code written by Zoltan Kato)
 * Copyright  : GNU General Public License www.gnu.org/copyleft/gpl.html
 * Description:
 * Platform independent timer class to measure elapsed CPU time of
 * arbitrary portions of a C++ code. 
 *
 * $Id: CKProcessTimeCounter.h,v 1.1 2004/12/13 12:07:28 kato Exp $
 * $Revision: 1.1 $
 * $State: Exp $
 * $Log: CKProcessTimeCounter.h,v $
 * Revision 1.1  2004/12/13 12:07:28  kato
 * Initial revision
 *
 * 
 *****************************************************************/

#ifndef CKPROCESSTIMECOUNTER_H
#define CKPROCESSTIMECOUNTER_H


class CKProcessTimeCounter
{
public:
	CKProcessTimeCounter (const char* strTimerName = 0);
	~CKProcessTimeCounter ();

	void Start ();
	void Stop ();
	void Reset ();

	double GetElapsedTimeMs () const;
	double GetElapsedLastTimeMs () const;

	void PrintElapsedTime () const;
	void PrintElapsedLastTime () const;
	void Etap (const char* strEtapName = "");
	void LastEtap (const char* strEtapName = "");

private:
	char m_strTimerName[255];
#ifdef _WIN32
	void* m_piStartTime;

	double m_fTime;
	double m_fLastTime;
	void* m_piPerfFreq;			// ticks per second
	int m_iPerfFreqAdjust;		// in case Freq is too big
	int m_iOverheadTicks;		// overhead in calling timer

	void DetermineTimer();
#else
	void* m_pLastStartTime;
	double m_fLastUserTime;
	double m_fLastSysTime;
	double m_fInactUserTime;
	double m_fInactSysTime;
#endif
};


#endif
