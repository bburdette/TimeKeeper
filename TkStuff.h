#ifndef TkStuff_h
#define TkStuff_h

#include "TimeKeeper.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <gtk/gtk.h>
#include <Poco/Foundation.h>

enum TimeKeeperColumns
{
	TkcTask,
	TkcStartTime,
	TkcEndTime,
	TkcDuration,
	TkcDailyTotal,
	TkcWeeklyTotal,
	TkcCount
};

class TkStuff
{
public:
	TkStuff();
	GtkWindow *mGwMainWindow;
	GtkWindow *mGwReportOptions;
	GtkTreeView *mGtvTimeLog;
	GtkListStore *mGls;
	GtkEntry *mGeTask;
	GtkEntry *mGeFrom;
	GtkEntry *mGeTo;
	GtkEntry *mGeOTThreshold;

	TimeKeeper mTk;
	std::string mSFileName;
	bool mBChanged;
	bool mBDecimalHours;
	Poco::DateTime mDtReportFrom;
	Poco::DateTime mDtReportTo;
    std::string mSLastReportFilename;
	double mDOTHoursThreshold;
	bool mBOnlyLast;

	bool mBCellEditingInProgress;
	GtkMenuItem *mGmiDelete;
	GtkMenu *mGm;
	string mSDeleteAccelPath;
	GtkAccelGroup *mGtkAccelGroup;

	NVThing mNvtPrefs;

	std::vector<GtkCellRenderer *> mVColumnRenderers;

	typedef std::map<string, TimeKeeper::LogEntry*> PathEntryMap;
	PathEntryMap mPem;

	void SetRowFocus();

	void ToListStore();
	void FromListStore();
	bool Save();
	bool SaveAs();

	bool TabDelimitedExport();
	bool TabDelimitedExport(std::ostream &aOs);

	bool TimeReport();
	bool TimeReport(std::ostream &aOs);

	void HoursToText(const Poco::Timespan &aTs, ostream &aOs) const;

	void SavePrefs();
	void ReadPrefs();

	// get name of file that was open last time the app was closed.
	std::string GetLastFile();
};


#endif /*TKSTUFF_H_*/
