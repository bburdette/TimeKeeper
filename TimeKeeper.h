#ifndef TimeKeeper_h
#define TimeKeeper_h

#include <string>
#include <NVThing.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>
#include <Poco/DateTime.h>
#include <set>
#include "TimePeriod.h"

class TimeKeeper
{
public:
	TimeKeeper();

	enum TotalType
	{
		TtWeekly,
		TtDaily
	};

	// LogEntry class - one instance of this per time period.
	class LogEntry
	{
	public:
		// TO DO: prevent End dates before Start dates.
		string mSTask;

		const Poco::Timestamp& GetStart() const { return mTp.GetStart(); }
		const Poco::Timestamp& GetEnd() const { return mTp.GetEnd(); }

		void SetStart(const Poco::Timestamp& aT) { mTp.SetStart(aT); }
		void SetEnd(const Poco::Timestamp& aT) { mTp.SetEnd(aT); }

		const TimePeriod& GetTimePeriod() const { return mTp; }

		// Ftns for totals.
		void SetTotal(AbsoluteTimePeriod::TimePeriodType, Poco::Timespan &aTotal);
		void IncrementTotal(AbsoluteTimePeriod::TimePeriodType, Poco::Timespan &aTotal);

		void ClearTotal(AbsoluteTimePeriod::TimePeriodType);
		void ClearTotals();

		bool GetTotal(AbsoluteTimePeriod::TimePeriodType aTpt, Poco::Timespan &aTotal) const;

		// Ordering for log entry container.
		bool operator<(const LogEntry &aLe) const 
			{ return GetStart() < aLe.GetStart(); }

		void ToNvt(NVThing &aNvt) const;
		void FromNvt(const NVThing &aNvt);
	private:
		TimePeriod mTp;
		typedef map<AbsoluteTimePeriod::TimePeriodType, Poco::Timespan> TotalMap;
		TotalMap mTm;
	};

	typedef set<LogEntry> LeContainer;
	LeContainer mLcEntries;

	// Create a new entry with start and end datetime set to 
	// current datetime.
	void NewEntry(const string &aSTask);

	// 0 means no entry has 'focus'.
//	void SetCurrentEntry(LogEntry* aLe) { mLeCurrent = aLe; }
//	LogEntry& GetCurrentEntry() const { return *mLeCurrent; }

	bool IsEmpty() const { return mLcEntries.empty(); }

	void Clear() { mLcEntries.clear(); }

	void ComputeTotals(AbsoluteTimePeriod::TimePeriodType aTpt, bool aBOnlyLast);

	void Open(const char *aCFile);
	void Save(const char *aCFile) const;
	void ToNvt(NVThing &aNvt) const;
	void FromNvt(const NVThing &aNvt);

	// text time report to aOs.
	void TimeReport(Poco::DateTime &aDtFrom, Poco::DateTime &aDtTo, 
					Poco::Timespan aTOtThreshold,
					bool aBDecimalHours,
					ostream &aOs);

protected:
	// Starting at aIterStart, computes total time within the time period.
	// If aBOnlyLast, then the final total goes in the last record for the 
	// time period.  If !aBOnlyLast, each record gets the total up to and 
	// including itself.
	Poco::Timespan ComputeTimePeriodTotal(LeContainer::iterator aIterStart, 
									AbsoluteTimePeriod &aAtp,
									LeContainer::iterator &aIterLast,
									bool aBOnlyLast);
	static void OTComp(Poco::Timespan aTAmt, Poco::Timespan aTTotal, Poco::Timespan aTOtThres,
				Poco::Timespan &aTOT, Poco::Timespan &aTNonOT);
	
};

#include <iostream>
#include <iomanip>

// Use like this:
// aOs << HoursToText(lTs, true);
class HoursToText
{
public:
	HoursToText(Poco::Timespan aT, bool aBDecimalHours, bool aBEmptyWhenZero = true)
	:mT(aT), mBDecimalHours(aBDecimalHours), mBEmptyWhenZero(aBEmptyWhenZero)
	{}
	
	void ToOs(ostream &aOs) const
	{
		if (mBEmptyWhenZero && mT == 0)
			return;
		
		if (mBDecimalHours)
		{
			aOs << fixed << setprecision(2) << setw(3) << setfill('0') << mT.totalMilliseconds() / (1000.0 * 60.0 * 60.0);
		}
		else
		{
			aOs << fixed << setprecision(0) << setw(2) << setfill('0') << 
				int(mT.totalMilliseconds() / (1000.0 * 60.0 * 60.0)) << ":" << 
				setw(2) << mT.minutes() << ":" << setw(2) << mT.seconds();
		}
	}
	
	double ToDouble() const
	{
		return mT.totalMilliseconds() / (1000.0 * 60.0 * 60.0);
	}
	
private:
	Poco::Timespan mT;
	bool mBDecimalHours;
	bool mBEmptyWhenZero;
	
};


inline ostream& operator<<(ostream &aOs, const HoursToText &aHtt)
{
	aHtt.ToOs(aOs);
	return aOs;
}


/*double operator=(double &aD, const HoursToText &aHtt)
{
	aD = aHtt.ToDouble();
	return aD;
}
*/


#endif
