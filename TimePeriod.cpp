#include "TimePeriod.h"
#include <Poco/DateTime.h>
#include <Poco/Timezone.h>

using namespace Poco;

// ------------------------------------------------------------------------

Timespan TimePeriod::TimeWithinTimePeriod(const TimePeriod &aTp) const
{
	// Completely before the interval?
	if (mTEnd < aTp.mTStart)
		return 0;

	// Completely after the interval?
	if (mTStart > aTp.mTEnd)
		return 0;

	Timestamp lTTheEnd;
	if (mTEnd < aTp.mTEnd)
		lTTheEnd = mTEnd;
	else
		lTTheEnd = aTp.mTEnd;

	Timestamp lTTheStart;
	if (mTStart > aTp.mTStart)
		lTTheStart = mTStart;
	else
		lTTheStart = aTp.mTStart;

	return lTTheEnd - lTTheStart;
}

// ------------------------------------------------------------------------

AbsoluteTimePeriod& AbsoluteTimePeriod::operator++()
{
	SetToContain(mTEnd);
	return *this;
}

AbsoluteTimePeriod& AbsoluteTimePeriod::operator--()
{
	// Back up 1 second from start, into the previous period.
	mTStart -= (Timespan(0, 0, 0, 1, 0)).totalMicroseconds();
	SetToContain(mTStart);
	return *this;
}

AbsoluteTimePeriod::AbsoluteTimePeriod(TimePeriodType aTpt, Timestamp aTContained)
{
	mTpt = aTpt;
	SetToContain(aTContained);
}

void AbsoluteTimePeriod::SetToContain(const Timestamp &aT)
{
	DateTime lDt(aT);
	switch (mTpt)
	{
	case TptDay:
		mTStart = DateTime(lDt.year(), lDt.month(), lDt.day()).timestamp();
		mTEnd = mTStart + Timespan(1, 0, 0, 0, 0).totalMicroseconds();
		break;
	case TptWeek:
		{
			int lIDow = lDt.dayOfWeek();
			mTStart = DateTime(lDt.year(), lDt.month(), lDt.day()).timestamp();
			mTStart -= Timespan(lIDow, 0, 0, 0, 0).totalMicroseconds();
			mTEnd = mTStart + Timespan(7, 0, 0, 0, 0).totalMicroseconds();
			break;
		}
	case TptMonth:
		mTStart = DateTime(lDt.year(), lDt.month(), 1).timestamp();
		if (lDt.month() == 12)
			mTEnd = DateTime(lDt.year() + 1, 1, 1).timestamp();
		else
			mTEnd = DateTime(lDt.year(), lDt.month() + 1, 1).timestamp();
		break;
	case TptYear:
		mTStart = DateTime(lDt.year(), 1, 1).timestamp();
		mTEnd = DateTime(lDt.year() + 1, 1, 1).timestamp();
		break;
	}
}

// ------------------------------------------------------------------------

Timestamp ToLocalTime(const Timestamp &aT)
{
	Timestamp lT(aT);
	Timestamp::TimeDiff lTd(Poco::Timezone::tzd());
	lTd *= 1000000;
	lT += lTd;
	return lT;
}

Timestamp FromLocalTime(const Timestamp &aT)
{
	Timestamp lT(aT);
	Timestamp::TimeDiff lTd(Poco::Timezone::tzd());
	lTd *= 1000000;
	lT -= lTd;
	return lT;
}

