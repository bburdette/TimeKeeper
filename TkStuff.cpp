#include "TkStuff.h"

#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeParser.h>
#include <Poco/Timezone.h>
#include <Poco/Exception.h>
#include <fstream>
#include <sstream>
#include <list>
#include <iomanip>

using namespace std;
using namespace Poco;

TkStuff::TkStuff()
	:mBChanged(false),
	mBDecimalHours(false),
	mDtReportFrom(2008, 1, 1),
	mDtReportTo(2008, 1, 20),
	mDOTHoursThreshold(40),
	mBOnlyLast(true),
	mBCellEditingInProgress(false)
{
}

void TkStuff::SetRowFocus()
{
	// Set focus to the last incomplete row, or barring that, the last row.
	GtkTreeIter lGti;
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(mGls), &lGti))
	{
		GtkTreeIter lGtiLast = lGti;
		bool lBIncomplete = false;
		GValue lGv = {0,};
		g_value_init(&lGv, G_TYPE_STRING);
		while (gtk_tree_model_iter_next(GTK_TREE_MODEL(mGls), &lGti))
		{
			gtk_tree_model_get_value (GTK_TREE_MODEL(mGls),
				&lGti,
				TkcEndTime,
				&lGv);
			if (strcmp(g_value_get_string(&lGv), "") == 0)
			{
				lGtiLast = lGti;
				lBIncomplete = true;
			}
			else if (!lBIncomplete)
			{
				lGtiLast = lGti;
			}
		}

		// Set focus to lGtiLast.
		GtkTreeSelection *lGts = gtk_tree_view_get_selection(mGtvTimeLog);
		gtk_tree_selection_select_iter(lGts, &lGtiLast);

		GtkTreePath *lGtp = gtk_tree_model_get_path(GTK_TREE_MODEL(mGls), &lGtiLast);

		gtk_tree_view_scroll_to_cell(mGtvTimeLog,  // treeview
									 lGtp,			// path (row)
									 0,				// column (none)
									 true,			// use align.
									 0.0,			// align row at bottom.
									 0.0);			// align column at left.

		gtk_tree_path_free(lGtp);
	}
}

void TkStuff::SavePrefs()
{
	// Save settings to prefs NVT

	// if filename is blank, don't write over whatever is in prefs.
	if (mSFileName != "")
	{
        mNvtPrefs.Delete("LastFile");
        mNvtPrefs.Add("LastFile", mSFileName);
	}
	mNvtPrefs.Delete("DecimalHours");
	mNvtPrefs.Add("DecimalHours", mBDecimalHours);
	mNvtPrefs.Delete("OnlyTotalsOnLast");
	mNvtPrefs.Add("OnlyTotalsOnLast", mBOnlyLast);
	mNvtPrefs.Delete("OTHoursThreshold");
	mNvtPrefs.Add("OTHoursThreshold", mDOTHoursThreshold);
	mNvtPrefs.Delete("ReportFrom");
	mNvtPrefs.Add("ReportFrom",
		DateTimeFormatter::format(mDtReportFrom, DateTimeFormat::SORTABLE_FORMAT, 0));
	mNvtPrefs.Delete("ReportTo");
	mNvtPrefs.Add("ReportTo",
		DateTimeFormatter::format(mDtReportTo, DateTimeFormat::SORTABLE_FORMAT, 0));
	mNvtPrefs.Delete("LastReportFileName");
    mNvtPrefs.Add("LastReportFileName", mSLastReportFilename.c_str());

	gint lGiX, lGiY;
	gtk_window_get_position(mGwMainWindow, &lGiX, &lGiY);
	mNvtPrefs.Delete("MainX");
	mNvtPrefs.Add("MainX", lGiX);
	mNvtPrefs.Delete("MainY");
	mNvtPrefs.Add("MainY", lGiY);

	gtk_window_get_size(mGwMainWindow, &lGiX, &lGiY);
	mNvtPrefs.Delete("MainW");
	mNvtPrefs.Add("MainW", lGiX);
	mNvtPrefs.Delete("MainH");
	mNvtPrefs.Add("MainH", lGiY);

	// Write prefs to the file.
	string lS = g_get_home_dir();
	lS += "/TimeKeeper.prefs";
	ofstream lOfs(lS.c_str());

	lOfs << mNvtPrefs;
}

void TkStuff::ReadPrefs()
{
	string lS = g_get_home_dir();
	lS += "/TimeKeeper.prefs";
	ifstream lIfs(lS.c_str());

	if (lIfs.is_open())
	{
		lIfs >> mNvtPrefs;

        // Read settings from prefs.
        mNvtPrefs.Find("DecimalHours", mBDecimalHours);
        mNvtPrefs.Find("OnlyTotalsOnLast", mBOnlyLast);
        mNvtPrefs.Find("OTHoursThreshold", mDOTHoursThreshold);

        // Don't retrieve this, because mSFileName is for the current file that is open.
    //	mNvtPrefs.Find("LastFile", mSFileName);

        int lITzd;		// Time zone differential (returned)
        if (mNvtPrefs.Find("ReportFrom", lS))
        {
            Poco::DateTimeParser::tryParse(lS, mDtReportFrom, lITzd);
        }
        if (mNvtPrefs.Find("ReportTo", lS))
        {
            Poco::DateTimeParser::tryParse(lS, mDtReportTo, lITzd);
        }

        mSLastReportFilename = "";
        mNvtPrefs.Find("LastReportFileName", mSLastReportFilename);
	}
	else
	{
		// throw Common::MessageException("Unable to open prefs file: ") << lS;
	    // set default values.

        // Read settings from prefs.
        mBDecimalHours = false;
        mBOnlyLast = true;
        mDOTHoursThreshold = 40;

        /*
        int lITzd;		// Time zone differential (returned)
        if (mNvtPrefs.Find("ReportFrom", lS))
        {
            Poco::DateTimeParser::tryParse(lS, mDtReportFrom, lITzd);
        }
        if (mNvtPrefs.Find("ReportTo", lS))
        {
            Poco::DateTimeParser::tryParse(lS, mDtReportTo, lITzd);
        }
        */
	}

}

string TkStuff::GetLastFile()
{
	string lS("");
	mNvtPrefs.Find("LastFile", lS);

	return lS;
}

void TkStuff::HoursToText(const Timespan &aTs, ostream &aOs) const
{
	if (mBDecimalHours)
	{
		aOs << fixed << setprecision(2) << setw(3) << setfill('0') << aTs.totalMilliseconds() / (1000.0 * 60.0 * 60.0);
	}
	else
	{
		aOs << fixed << setprecision(0) << setw(2) << setfill('0') <<
			int(aTs.totalMilliseconds() / (1000.0 * 60.0 * 60.0)) << ":" <<
			setw(2) << aTs.minutes() << ":" << setw(2) << aTs.seconds();
	}
}

void TkStuff::ToListStore()
{
	// clear the list store.
	gtk_list_store_clear(mGls);

	GtkTreeIter lGti;

	Poco::Timespan lTs;
	Poco::DateTime lDtCurrent;

	mPem.clear();

	TimeKeeper::LeContainer::const_iterator lIter = mTk.mLcEntries.begin();
	if (lIter != mTk.mLcEntries.end())
	{
		DateTime lDtWk(lIter->GetStart());
		lDtCurrent = DateTime(lDtWk.year(), lDtWk.month(), lDtWk.day());
	}
	while (lIter != mTk.mLcEntries.end())
	{
		gtk_list_store_append (mGls, &lGti);
		if (lIter->GetEnd() != lIter->GetStart())
		{
			Timespan lTs = lIter->GetEnd() - lIter->GetStart();

			ostringstream lOssDuration;
			HoursToText(lTs, lOssDuration);

			ostringstream lOssDaily;
			if (lIter->GetTotal(AbsoluteTimePeriod::TptDay, lTs))
				HoursToText(lTs, lOssDaily);

			ostringstream lOssWeekly;
			if (lIter->GetTotal(AbsoluteTimePeriod::TptWeek, lTs))
				HoursToText(lTs, lOssWeekly);

			// Determine format for end datetime.
			string lSEndFormat;
			string lSDB1 = DateTimeFormatter::format(lIter->GetStart(), "%Y%m%e", 0);
			string lSDB2 = DateTimeFormatter::format(lIter->GetEnd(), "%Y%m%e", 0);

			if (DateTimeFormatter::format(lIter->GetStart(), "%Y%m%e", 0) ==
				DateTimeFormatter::format(lIter->GetEnd(), "%Y%m%e", 0))
				lSEndFormat = "%H:%M:%S";
			else
				lSEndFormat = DateTimeFormat::SORTABLE_FORMAT;

			gtk_list_store_set (mGls, &lGti,
				TkcTask, lIter->mSTask.c_str(),
				TkcStartTime, DateTimeFormatter::format(lIter->GetStart(), DateTimeFormat::SORTABLE_FORMAT, 0).c_str(),
				TkcEndTime, DateTimeFormatter::format(lIter->GetEnd(), lSEndFormat, 0).c_str(),
				TkcDuration, lOssDuration.str().c_str(),
				TkcDailyTotal, lOssDaily.str().c_str(),
				TkcWeeklyTotal, lOssWeekly.str().c_str(),
				-1);

		}
		else
		{
			gtk_list_store_set (mGls, &lGti,
				TkcTask, lIter->mSTask.c_str(),
				TkcStartTime, DateTimeFormatter::format(lIter->GetStart(), DateTimeFormat::SORTABLE_FORMAT, 0).c_str(),
				TkcEndTime, "",
				TkcDuration, "",
				TkcDailyTotal, "",
				TkcWeeklyTotal, "",
				-1);
		}
		// get the gtktreepath for lGti.
		GtkTreePath *lGtp = gtk_tree_model_get_path(GTK_TREE_MODEL(mGls), &lGti);
		// translate to char.
		gchar *lGc = gtk_tree_path_to_string(lGtp);
		TimeKeeper::LogEntry* lLe = const_cast<TimeKeeper::LogEntry*>(&(*lIter));
		// Put the path, logentry into the map.
		this->mPem.insert(PathEntryMap::value_type(lGc, lLe));
		g_free(lGc);

		// On to the next!
		++lIter;
	}
}

void TkStuff::FromListStore()
{
	GtkTreeIter lGti;
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(mGls), &lGti))
	{

		mTk.mLcEntries.clear();

		TimeKeeper::LogEntry lLe;
		do
		{
			GValue lGv = {0,};
			g_value_init(&lGv, G_TYPE_STRING);
			// Get the task string.
			gtk_tree_model_get_value (GTK_TREE_MODEL(mGls), &lGti,
				TkcTask, &lGv);
			lLe.mSTask = g_value_get_string(&lGv);

			// Get the start datetime.
			Poco::DateTime lDt;
			int lI = 0;
			gtk_tree_model_get_value (GTK_TREE_MODEL(mGls), &lGti,
				TkcStartTime, &lGv);
			DateTimeParser::parse(DateTimeFormat::SORTABLE_FORMAT, g_value_get_string(&lGv), lDt, lI);
			lLe.SetStart(lDt.timestamp());

			// Get the end datetime.
			gtk_tree_model_get_value (GTK_TREE_MODEL(mGls), &lGti,
				TkcEndTime, &lGv);
			if (strcmp(g_value_get_string(&lGv), ""))
			{
				const char *lCDB = g_value_get_string(&lGv);

				string lSParse;

				if (strlen(lCDB) < 10)
				{
					// try putting the YMD from the start on there, and then parsing.
					lSParse = DateTimeFormatter::format(lDt, "%Y-%m-%e ", 0);
					lSParse += lCDB;
				}
				else
					lSParse = lCDB;

				try
				{
					DateTimeParser::parse(DateTimeFormat::SORTABLE_FORMAT, lSParse.c_str(), lDt, lI);
				}
				catch (SyntaxException &aSe)
				{
					DateTimeParser::parse("%H:%M:%S", g_value_get_string(&lGv), lDt, lI);
				}
				lLe.SetEnd(lDt.timestamp());
			}
			else
				lLe.SetEnd(lLe.GetStart());

			// Insert the completed record.
			mTk.mLcEntries.insert(lLe);
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(mGls), &lGti));

		// Compute totals.
		mTk.ComputeTotals(AbsoluteTimePeriod::TptDay, mBOnlyLast);
		mTk.ComputeTotals(AbsoluteTimePeriod::TptWeek, mBOnlyLast);
	}
}

bool TkStuff::SaveAs()
{
	// pick the file.
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new ("Save File",
		mGwMainWindow, // parent_window,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

	if (mSFileName == "") // user_edited_a_new_document)
	{
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), ".");
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "new_timelog.nvt");
	}
	else
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), mSFileName.c_str()); // filename_for_existing_document);


	bool lBReturn(false);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		// Open the selected file, display data.
		mTk.Save(filename);
		mSFileName = filename;
		mBChanged = false;

		g_free (filename);
		lBReturn = true;
	}

	gtk_widget_destroy (dialog);

	return lBReturn;
}

bool TkStuff::Save()
{
	// save to current filename.

	if (mSFileName == "")
	{
		// no current filename.
		return SaveAs();
	}
	else
	{
		mTk.Save(mSFileName.c_str());
		mBChanged = false;
		return true;
	}
}

bool TkStuff::TabDelimitedExport()
{
	// pick the file.
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new ("Tab Delimited Export",
		mGwMainWindow, // parent_window,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

	/*
	if (mSFileName == "") // user_edited_a_new_document)
	{
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), ".\\");
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "new_timelog.nvt");
	}
	else
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), mSFileName.c_str()); // filename_for_existing_document);
	*/

	bool lBReturn(false);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		// Open the selected file, display data.
		ofstream lOfs(filename);
		g_free (filename);

		if (!lOfs.is_open())
		{
			// exception or something!!
		}

		TabDelimitedExport(lOfs);

		lBReturn = true;
	}

	gtk_widget_destroy (dialog);

	return lBReturn;
}

bool TkStuff::TabDelimitedExport(ostream &aOs)
{
	// Start by just writing the selected records to the stream.
	GtkTreeSelection *lGts = gtk_tree_view_get_selection(mGtvTimeLog);

	GList *lGlTheList = gtk_tree_selection_get_selected_rows(lGts, 0);

	if (!lGlTheList)
		return false;

	GList *lGl = lGlTheList;

	// Build a list of the LogEntries for the selected items.
	list<TimeKeeper::LogEntry*> lL;
	while (lGl)
	{
		// If we found the last item, select it.
		gchar *lGc = gtk_tree_path_to_string(static_cast<GtkTreePath*>(lGl->data));

		PathEntryMap::iterator lIter = mPem.find(lGc);
		if (lIter != mPem.end())
		{
			lL.push_back(lIter->second);
		}
		g_free(lGc);

		// on to the next.
		lGl = lGl->next;
	}

	// Free the list results.
	g_list_foreach (lGlTheList, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (lGlTheList);

	// Now, report time.
	// Nothing fancy, just write each record out to the ostream.

	for (list<TimeKeeper::LogEntry*>::iterator lIter = lL.begin();
		lIter != lL.end();
		++lIter)
	{
		Timespan lTs = (*lIter)->GetEnd() - (*lIter)->GetStart();

		aOs << (*lIter)->mSTask << "\t";
		ostringstream lOssDuration;
		HoursToText(lTs, lOssDuration);

		ostringstream lOssDaily;
		if ((*lIter)->GetTotal(AbsoluteTimePeriod::TptDay, lTs))
			HoursToText(lTs, lOssDaily);

		ostringstream lOssWeekly;
		if ((*lIter)->GetTotal(AbsoluteTimePeriod::TptWeek, lTs))
			HoursToText(lTs, lOssWeekly);

		aOs << DateTimeFormatter::format((*lIter)->GetStart(), DateTimeFormat::SORTABLE_FORMAT, 0) << "\t" <<
			DateTimeFormatter::format((*lIter)->GetEnd(), DateTimeFormat::SORTABLE_FORMAT, 0) << "\t" <<
			lOssDuration.str() << "\t" <<
			lOssDaily.str() << "\t" <<
			lOssWeekly.str() << endl;
	}

	return true;
}

bool TkStuff::TimeReport()
{
	// pick the file.
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new ("Create Time Report",
		mGwMainWindow, // parent_window,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

    if (mSLastReportFilename != "")
    {
		GFile *lGf = g_file_new_for_path(mSLastReportFilename.c_str());
		GFile *lGfParentDir = g_file_get_parent(lGf);
		if (lGfParentDir)
		{
		    const char *lCParentDir = g_file_get_path(lGfParentDir);
            gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
                                                             lCParentDir);
            g_free((void*)lCParentDir);
            g_object_unref(lGfParentDir);
		}

		g_object_unref(lGf);

    }

	bool lBReturn(false);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		// Open the selected file, display data.
		ofstream lOfs(filename);

		mSLastReportFilename = filename;
		g_free (filename);

		if (!lOfs.is_open())
		{
			// exception or something!!
		}

		TimeReport(lOfs);

		lBReturn = true;
	}

	gtk_widget_destroy (dialog);

	return lBReturn;
}

bool TkStuff::TimeReport(ostream &aOs)
{
	// OT threshold.
	Timespan lTsOT((long int)(mDOTHoursThreshold*60*60), 0);
	// Produce the report.
	mTk.TimeReport(mDtReportFrom, mDtReportTo, lTsOT, mBDecimalHours, aOs);

	// recompute totals if necessary.
	if (mBOnlyLast)
		mTk.ComputeTotals(AbsoluteTimePeriod::TptWeek, mBOnlyLast);
	return true;
}
