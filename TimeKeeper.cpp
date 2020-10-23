#include "TimeKeeper.h"
#include <fstream>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Exception.h>

using namespace Poco;

// ------------------------------------------------------------------------

void TimeKeeper::LogEntry::ToNvt(NVThing &aNvt) const
{
	aNvt.Clear();

	aNvt.Add("Task", mSTask);
	aNvt.Add("Start", GetStart().utcTime());
	aNvt.Add("End", GetEnd().utcTime());
}
void TimeKeeper::LogEntry::FromNvt(const NVThing &aNvt)
{
	aNvt.Find("Task", mSTask, true);
	Poco::Timestamp::UtcTimeVal lUtc;
	aNvt.Find("Start", lUtc, true); 
	SetStart(Timestamp::fromUtcTime(lUtc));
	aNvt.Find("End", lUtc, true); 
	SetEnd(Timestamp::fromUtcTime(lUtc));
}

void TimeKeeper::LogEntry::SetTotal(AbsoluteTimePeriod::TimePeriodType aTpt, Poco::Timespan &aTotal)
{
	if (aTotal == 0)
	{
		ClearTotal(aTpt);
		return;
	}

	TotalMap::iterator lIter = mTm.find(aTpt);
	if (lIter == mTm.end())
	{
		mTm.insert(TotalMap::value_type(aTpt, aTotal));
	}
	else
	{
		lIter->second = aTotal;
	}
}

void TimeKeeper::LogEntry::IncrementTotal(AbsoluteTimePeriod::TimePeriodType aTpt, Poco::Timespan &aTotal)
{
	TotalMap::iterator lIter = mTm.find(aTpt);
	if (lIter == mTm.end())
	{
		mTm.insert(TotalMap::value_type(aTpt, aTotal));
	}
	else
	{
		lIter->second += aTotal;
	}
}

bool TimeKeeper::LogEntry::GetTotal(AbsoluteTimePeriod::TimePeriodType aTpt, Poco::Timespan &aTotal) const
{
	TotalMap::const_iterator lIter = mTm.find(aTpt);
	if (lIter == mTm.end())
		return false;
	else
	{
		aTotal = lIter->second;
		return true;
	}
}

void TimeKeeper::LogEntry::ClearTotal(AbsoluteTimePeriod::TimePeriodType aTpt)
{
	mTm.erase(aTpt);
}

void TimeKeeper::LogEntry::ClearTotals()
{
	mTm.clear();
}

// ------------------------------------------------------------------------

TimeKeeper::TimeKeeper()
{
	// mLeCurrent = 0;
}

void TimeKeeper::NewEntry(const string &aSTask)
{
	LogEntry lLe;
	lLe.mSTask = aSTask;
	lLe.SetStart(ToLocalTime(Poco::Timestamp()));
	lLe.SetEnd(lLe.GetStart());

	std::pair<set<LogEntry>::iterator,bool> lP = mLcEntries.insert(lLe);
	// set<LogEntry>::_Pairib lP = mLcEntries.insert(lLe);
	if (!lP.second)
		throw Common::MessageException("Duplicate timestamp!");

//	mLeCurrent = &(lP.first.operator *());
}

void TimeKeeper::Open(const char *aCFile)
{
	ifstream lIfs(aCFile, ios::binary);
	if (!lIfs.is_open())
	{
		throw Common::MessageException("Unable to open file: ") << aCFile;
	}

	NVThing lNvt;

	lIfs >> lNvt;

	// read, read it in.
	FromNvt(lNvt);
}
void TimeKeeper::Save(const char *aCFile) const
{
	// binary mode for compatability with linux.
	ofstream lOfs(aCFile, ios::binary);
	if (!lOfs.is_open())
	{
		throw Common::MessageException("Unable to open file: ") << aCFile;
	}

	NVThing lNvt;
	ToNvt(lNvt);

	lOfs << lNvt;
}
void TimeKeeper::ToNvt(NVThing &aNvt) const
{
	aNvt.Clear();

	NVThing *lNvt;

	LeContainer::const_iterator lIter = mLcEntries.begin();
	while (lIter != mLcEntries.end())
	{
		lNvt = new NVThing;
		
		lIter->ToNvt(*lNvt);

		aNvt.Add("LogEntry", lNvt);
		++lIter;
	}
}
void TimeKeeper::FromNvt(const NVThing &aNvt)
{
	NVThing *lNvt;

	mLcEntries.clear();

	aNvt.ResetFindNext();
	while (aNvt.FindNext("LogEntry", lNvt))
	{
		LogEntry lLe;
		lLe.FromNvt(*lNvt);
		mLcEntries.insert(lLe);
	}
}

Timespan TimeKeeper::ComputeTimePeriodTotal(LeContainer::iterator aIterStart, 
											AbsoluteTimePeriod &aAtp,
										LeContainer::iterator &aIterLast,
										bool aBOnlyLast) 
{
	aIterLast = aIterStart;

	Timespan lTTotal(0);
	while (aIterStart != mLcEntries.end())
	{
		Timespan lT = aIterStart->GetTimePeriod().TimeWithinTimePeriod(aAtp);
		if (lT != 0)
		{
			lTTotal += lT;
			aIterLast = aIterStart;
			if (!aBOnlyLast)
				const_cast<LogEntry*>(&(*aIterLast))->IncrementTotal(aAtp.GetTpt(), lTTotal);
			++aIterStart;
		}
		else
		{
			if (aBOnlyLast)
				const_cast<LogEntry*>(&(*aIterLast))->IncrementTotal(aAtp.GetTpt(), lTTotal);
			return lTTotal;
		}
	}
	if (aBOnlyLast)
		const_cast<LogEntry*>(&(*aIterLast))->IncrementTotal(aAtp.GetTpt(), lTTotal);
	return lTTotal;
}

void TimeKeeper::ComputeTotals(AbsoluteTimePeriod::TimePeriodType aTpt, bool aBOnlyLast)
{
	if (mLcEntries.empty())
		return;
		
	LeContainer::iterator lIter = mLcEntries.begin();
	
	while (lIter != mLcEntries.end())
	{
		const_cast<LogEntry*>(&(*lIter))->ClearTotal(aTpt);
		++lIter;
	}

	lIter = mLcEntries.begin();
	
	LeContainer::iterator lIterLast = lIter;

	AbsoluteTimePeriod lAtp(aTpt);

	lAtp.SetToContain((*lIter).GetStart());

	Timespan lT;

	while (lIter != mLcEntries.end())
	{

		lT = ComputeTimePeriodTotal(lIter, lAtp, lIterLast, aBOnlyLast);
		if (lT != 0)
		{
			// Store the total.
			// const_cast<LogEntry*>(&(*lIterLast))->IncrementTotal(lAtp.GetTpt(), lT);

			// Advance to the next ATP.
			++lAtp;
			lIter = lIterLast;
		}
		else
		{
			// No total.  lIter is not in the current time period.
			lIter = lIterLast;
			if (lAtp.GetEnd() < lIter->GetStart())
			{
				// the log has jumped ahead of the time period.  
				lAtp.SetToContain(lIter->GetStart());
			}
			else
			{
				// Time period is ahead of the log.
				++lIter;
			}
		}
	}
}

// The mission:
// compute totals for OT hours, non-OT, and all hours for the records that fall in the 
// from/to range.
// Write out all the records in the from/to range as well.
void TimeKeeper::TimeReport(Poco::DateTime &aDtFrom, Poco::DateTime &aDtTo, 
						Timespan aTOtThreshold,
						bool aBDecimalHours,
						ostream &aOs)
{
	aOs << "TimeKeeper Time Report\n";
	aOs << "From: " << DateTimeFormatter::format(aDtFrom, DateTimeFormat::SORTABLE_FORMAT, 0) << "\n";
	aOs << "To: " << DateTimeFormatter::format(aDtTo, DateTimeFormat::SORTABLE_FORMAT, 0) << "\n";
	aOs << "OT for hours over " << HoursToText(aTOtThreshold, aBDecimalHours) <<  " per week\n";
	aOs << "\n";
	aOs << "Time Records";
	aOs << "\n";
	aOs << "Task\tFrom\tTo\tDuration\tRegular\tOT\n";
	
	Timestamp lTsStart = aDtFrom.timestamp();
	Timestamp lTsEnd = aDtTo.timestamp();
	
	if (lTsEnd < lTsStart)
		throw Common::MessageException("The time report end date is before the start date!");
	
	LogEntry lLeStart;
	
	lLeStart.SetStart(lTsStart);
	
	// Compute weekly totals with onlyend == false.
	ComputeTotals(AbsoluteTimePeriod::TptWeek, false);
	
	// first logentry with start >= lTsStart.
	LeContainer::const_iterator lIter = mLcEntries.lower_bound(lLeStart);
	
	// does the previous element contain lTsStart?
	LeContainer::const_iterator lIterPrev(lIter);
	--lIterPrev;
	if (lIterPrev != mLcEntries.end())
	{
		if (lIterPrev->GetTimePeriod().Contains(lTsStart))
		{
			// Yes, start the report there instead.
			lIter = lIterPrev;
		}
	}
	
	Timespan lTTotal(0);	
	Timespan lTTotalOT(0);	
	Timespan lTTotalNonOT(0);
	
	// Loop through all qualifying records, then print totals.
	
	while (lIter != mLcEntries.end() &&
			lIter->GetStart() < lTsEnd)
	{
		// What's the current timeperiod and totals, considering the start
		// and end?
		Timespan lTAmt; 
		Timespan lTTotalWk;
		
		TimePeriod lTp = lIter->GetTimePeriod();
		// If timeperiod includes the report start, change it.
		if (lIter->GetTimePeriod().Contains(lTsStart))
		{
			lTp.SetStart(lTsStart);
		}
		// If the end is included, change the end and the total.
		if (lTp.Contains(lTsEnd))
		{
			lTp.SetEnd(lTsEnd);
			
			if (!lIter->GetTotal(AbsoluteTimePeriod::TptWeek, lTTotalWk))
				throw Common::MessageException("Expected total in LogEntry");
			lTTotalWk -= lIter->GetEnd() - lTsEnd;
		}
		else
		{
			if (!lIter->GetTotal(AbsoluteTimePeriod::TptWeek, lTTotalWk))
				throw Common::MessageException("Expected total in LogEntry");
		}
		
/*		double lDTw = HoursToText(lTTotalWk, true).ToDouble();
		double lDDur = HoursToText(lTp.Duration(), true).ToDouble();
		double lDThres = HoursToText(aTOtThreshold, true).ToDouble();
*/
		// How much is OT and how much is not?
		Timespan lTOt, lTNonOt;
		OTComp(lTp.Duration(), lTTotalWk, aTOtThreshold, lTOt, lTNonOt);
		lTTotal += lTOt + lTNonOt;	
		lTTotalOT += lTOt;	
		lTTotalNonOT += lTNonOt;
		
		// Ok, totals done.  Write each record to the report.
		aOs << lIter->mSTask;
		aOs << "\t" << DateTimeFormatter::format(lTp.GetStart(), DateTimeFormat::SORTABLE_FORMAT, 0);
		aOs << "\t" << DateTimeFormatter::format(lTp.GetEnd(), DateTimeFormat::SORTABLE_FORMAT, 0);
		aOs << "\t" << HoursToText(lTOt + lTNonOt, aBDecimalHours) << "\t" << 
			HoursToText(lTNonOt, aBDecimalHours) << "\t" << 
			HoursToText(lTOt, aBDecimalHours) << "\n";
			
		
		++lIter;
	}
	
	aOs << "\n";
	aOs << "Time Totals";
	aOs << "\n";
	aOs << "Total Hours:\t" << HoursToText(lTTotal, aBDecimalHours) << "\n";
	aOs << "Total Regular Hours:\t" << HoursToText(lTTotalNonOT, aBDecimalHours) << "\n";
	aOs << "Total OT Hours:\t" << HoursToText(lTTotalOT, aBDecimalHours) << "\n";

}

// How much of the timeperiod is OT?
void TimeKeeper::OTComp(Timespan aTAmt, Timespan aTTotal, Timespan aTOtThreshold,
				Timespan &aTOT, Timespan &aTNonOT)
{
	aTOT = 0;
	aTNonOT = 0;
	// Is some of the time OT time?
	if (aTTotal > aTOtThreshold)
	{
		// All OT or just part?
		if (aTTotal - aTAmt < aTOtThreshold)
		{
			// just part OT.
			aTNonOT = aTOtThreshold - (aTTotal - aTAmt);
			aTOT = aTTotal - aTOtThreshold;
		}
		else
		{
			// Its all OT.
			aTOT += aTAmt;
		}
	}
	else
	{
		// no OT.
		aTNonOT += aTAmt;
	}
}

