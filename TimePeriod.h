#ifndef TimePeriod_h
#define TimePeriod_h

#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>

// Represents a period in time with a start and end.
// Time included is up to but not including end.
class TimePeriod
{
public:
	TimePeriod()
		:mTStart(0), mTEnd(0)
	{}
	TimePeriod(const Poco::Timestamp &aTStart, const Poco::Timestamp &aTEnd)
		:mTStart(aTStart), mTEnd(aTEnd)
	{}
	virtual ~TimePeriod() {}

	const Poco::Timestamp& GetStart() const { return mTStart; }
	const Poco::Timestamp& GetEnd() const { return mTEnd; }

	virtual void SetStart(const Poco::Timestamp& aT) { mTStart = aT; }
	virtual void SetEnd(const Poco::Timestamp& aT) { mTEnd = aT; }

	void GetBounds(Poco::Timestamp &aTStart, Poco::Timestamp &aTEnd) { aTStart = mTStart; aTEnd = mTEnd; }
	virtual void SetBounds(const Poco::Timestamp &aTStart, const Poco::Timestamp &aTEnd) { mTStart = aTStart; mTEnd = aTEnd; }

	inline bool Contains(Poco::Timestamp aT) const { return mTStart <= aT && aT < mTEnd; }
	inline Poco::Timespan Duration() const { return mTEnd - mTStart; }
	// How much of this timeperiod is contained within aTp?
	Poco::Timespan TimeWithinTimePeriod(const TimePeriod &aTp) const;
protected:
	Poco::Timestamp mTStart;
	Poco::Timestamp mTEnd;
};

// Represents a period in time that holds to a specific increment.
// A day, week, month, or year.  You can increment to the next 
// period of the current type.
class AbsoluteTimePeriod : public TimePeriod
{
public:
	enum TimePeriodType
	{
		TptDay,
		TptWeek,
		TptMonth,
		TptYear
	};
	AbsoluteTimePeriod(TimePeriodType aTpt, Poco::Timestamp aTContained = Poco::Timestamp());

	TimePeriodType GetTpt() const { return mTpt; }

	AbsoluteTimePeriod& operator++();
	AbsoluteTimePeriod& operator--();
	// Get the bounds for the time period enclosing aT.
	void SetToContain(const Poco::Timestamp &aT);
	// set to the closest absolute timeperiod of the current type.
	virtual void SetBounds(const Poco::Timestamp &aTStart, const Poco::Timestamp &aTEnd) 
		{ SetToContain(aTStart); }
	virtual void SetStart(const Poco::Timestamp& aT) 
		{ SetToContain(aT); }
	virtual void SetEnd(const Poco::Timestamp& aT) 
		{ return; }		// no effect!

private:
	TimePeriodType mTpt;
};

Poco::Timestamp ToLocalTime(const Poco::Timestamp &aT);
Poco::Timestamp FromLocalTime(const Poco::Timestamp &aT);

#endif
