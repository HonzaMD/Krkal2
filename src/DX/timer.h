//////////////////////////////////////////////////////////////////////
//
// timer.h
//
// casovac
//
// A: Petr Altman
//
//////////////////////////////////////////////////////////////////////

#ifndef TIMER_H
#define TIMER_H

class CTimer
{
public:
	CTimer();

	LONGLONG GetTicksPerSec(){return QPFTicksPerSec;}

	LONGLONG GetAbsoluteTimeTicks();

private:
	
	BOOL UsingQPF;
	LONGLONG QPFTicksPerSec;
};

/*
enum TIMER_COMMAND { TIMER_RESET, TIMER_START, TIMER_STOP, TIMER_ADVANCE,
                     TIMER_GETABSOLUTETIME, TIMER_GETAPPTIME, TIMER_GETELAPSEDTIME };
FLOAT DXUtil_Timer( TIMER_COMMAND command );
*/

#endif