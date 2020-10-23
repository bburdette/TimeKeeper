#include <gtk/gtk.h>
#include <glade/glade.h>

#include <glib/gtypes.h>
#include <gdk/gdkkeysyms.h>

#include <Exception.h>
#include <string>
#include <vector>
#include <list>
#include "TimeKeeper.h"
#include "TkStuff.h"
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeParser.h>
#include <Poco/Timezone.h>
#include <Poco/Exception.h>
#include <Poco/Path.h>
#include <fstream>
#include <sstream>
#include <math.h>
#include <iomanip>
#include <sstream>
#include "gladefile.h"

using namespace std;
using namespace Poco;

// ---------------------------------------------------------------------------

TkStuff gTs;

// ---------------------------------------------------------------------------
// GUI ftn declarations.
// ---------------------------------------------------------------------------

// As an alternative to the exports.def file you can use this on ftn declarations.
// You'd need to macro-ize it to have it go away on linux.

//__declspec( dllexport ) void on_new1_activate(GtkWidget *widget, gpointer user_data)
//{
//  /* do something useful here */
//
//	int a = 1;
//}
//
//

// NOTE:  you have to use the exports.def file in the windows project for these
// functions to be available to glade autolinking.

extern "C" {
	void on_new1_activate (GtkMenuItem *menuitem,
		gpointer user_data);
	void on_open1_activate (GtkMenuItem *menuitem,
		gpointer user_data);

	void on_save1_activate (GtkMenuItem *menuitem,
		gpointer user_data);

	void on_save_as1_activate (GtkMenuItem *menuitem,
		gpointer user_data);

	void on_tab_delimited_export_activate (GtkMenuItem *menuitem,
		gpointer user_data);

	void on_time_report_activate (GtkMenuItem *menuitem,
		gpointer user_data);

	void on_quit1_activate (GtkMenuItem *menuitem,
		gpointer user_data);

	void on_reg_hours_activate (GtkMenuItem *menuitem,
		gpointer user_data);

	void on_dec_hours_activate (GtkMenuItem *menuitem,
		gpointer user_data);

	void on_about1_activate (GtkMenuItem *menuitem,
		gpointer user_data);

	void on_TimeLogMain_destroy(GtkObject *object);

	gboolean on_TimeLogMain_delete_event (GtkWidget *widget,
	                                    GdkEvent  *event,
	                                    gpointer   user_data);

	gboolean on_mGtvTimeLog_key_release_event ( GtkWidget *widget, GdkEventKey *event, gpointer data);

	void on_LogIn_clicked (GtkButton *button,
		gpointer user_data);

	void on_LogOut_clicked (GtkButton *button,
		gpointer user_data);

	void on_ReportOptions_Ok_clicked (GtkButton *button,
		gpointer user_data);
	void on_ReportOptions_Cancel_clicked (GtkButton *button,
		gpointer user_data);

	void  on_cell_edited (GtkCellRendererText *renderer,
		gchar *path,
		gchar *new_text,
		gpointer user_data);

	void on_cell_editing_canceled (GtkCellRenderer *cell);
	void on_cell_editing_started (GtkCellRenderer *cell,
								GtkCellEditable *editable,
								const gchar     *path);


	void on_delete1_activate (GtkMenuItem *menuitem,
		gpointer user_data);
}

void open_file (GtkWidget *widget, gpointer user_data);
void open_filename (const char *aCFileName);
void create_file_selection (void);

void DeleteSelectedRecords();

int main(int argc, char *argv[])
{
	try
	{
		// ShowWindow(GetConsoleWindow(), 0);

		// reads from home directory!  make it a '.' name?
		gTs.ReadPrefs();

		GladeXML *xml;

		gtk_init(&argc, &argv);

		// glade file is in the same dir as the executable.
		Poco::Path lP(argv[0]);
		lP.setFileName("timekeeper.glade");

		/* load the interface */
		//xml = glade_xml_new("timekeeper.glade", NULL, NULL);
		// xml = glade_xml_new(lP.toString().c_str(), NULL, NULL);
		xml = glade_xml_new_from_buffer(gladefile, strlen(gladefile), NULL, NULL);

		GtkWidget *lGw;
		// Load the main window.
		lGw = glade_xml_get_widget(xml, "TimeLogMain");
		if (!lGw)
			throw Common::MessageException("Unable to find 'TimeLogMain' in timekeeper.glade file!");
		gTs.mGwMainWindow = GTK_WINDOW(lGw);

		// Load the report options dialog.
		lGw = glade_xml_get_widget(xml, "ReportOptionsDialog");
		if (!lGw)
			throw Common::MessageException("Unable to find 'ReportOptionsDialog' in timekeeper.glade file!");
		gTs.mGwReportOptions = GTK_WINDOW(lGw);

		// Load the accel key group.
//		gTs.mGtkAccelGroup = glade_xml_ensure_accel(xml);

		// Get the Delete menu.
		lGw = glade_xml_get_widget(xml, "delete1");
		if (!lGw)
			throw Common::MessageException("Unable to find 'delete1' in timekeeper.glade file!");
		gTs.mGmiDelete = GTK_MENU_ITEM(lGw);
		const char *lC = gTs.mGmiDelete->accel_path;
		if (lC)
			gTs.mSDeleteAccelPath = lC;
		else
			gTs.mSDeleteAccelPath = "";

		lGw = glade_xml_get_widget(xml, "menu8");
		if (!lGw)
			throw Common::MessageException("Unable to find 'menu8' in timekeeper.glade file!");
		gTs.mGm = GTK_MENU(lGw);

		// Modify the number of columns and etc in the treeview.
		lGw = glade_xml_get_widget(xml, "mGtvTimeLog");
		if (!lGw)
			throw Common::MessageException("Unable to find 'mGtvTimeLog' in timekeeper.glade file!");
		//GtkTreeView *mGtvTimeLog = dynamic_cast<GtkTreeView *>(lGw);
		gTs.mGtvTimeLog = GTK_TREE_VIEW(lGw);

		GtkTreeSelection *selection = gtk_tree_view_get_selection (gTs.mGtvTimeLog);
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

		gTs.mGls = gtk_list_store_new(TkcCount, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
		gtk_tree_view_set_model(gTs.mGtvTimeLog, GTK_TREE_MODEL(gTs.mGls));

		// Set up columns.
		GtkTreeViewColumn *lGtvc;

		// use three lines of code to set up a boolean!
		GValue lGv = {0,};
		g_value_init(&lGv, G_TYPE_BOOLEAN);
		g_value_set_boolean(&lGv, true);

		// create a different renderer for each column, we'll use that to tell
		// which column is which, since gtk doesn't say that in the path.
		GtkCellRenderer *renderer;

		// -------------------
		renderer = gtk_cell_renderer_text_new ();
		g_object_set_property(G_OBJECT(renderer), "editable", &lGv);
		g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK (on_cell_edited), 0);
		g_signal_connect(G_OBJECT(renderer), "editing-canceled", G_CALLBACK (on_cell_editing_canceled), 0);
		g_signal_connect(G_OBJECT(renderer), "editing-started", G_CALLBACK (on_cell_editing_started), 0);
		gTs.mVColumnRenderers.push_back(renderer);

		lGtvc = gtk_tree_view_column_new_with_attributes ("Task",
			renderer,
			"text", TkcTask,
			NULL);
		gtk_tree_view_append_column (gTs.mGtvTimeLog, lGtvc);

		// -------------------
		renderer = gtk_cell_renderer_text_new ();
		g_object_set_property(G_OBJECT(renderer), "editable", &lGv);
		g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK (on_cell_edited), 0);
		g_signal_connect(G_OBJECT(renderer), "editing-canceled", G_CALLBACK (on_cell_editing_canceled), 0);
		g_signal_connect(G_OBJECT(renderer), "editing-started", G_CALLBACK (on_cell_editing_started), 0);
		gTs.mVColumnRenderers.push_back(renderer);

		lGtvc = gtk_tree_view_column_new_with_attributes ("Start",
			renderer,
			"text", TkcStartTime,
			NULL);
		gtk_tree_view_append_column (gTs.mGtvTimeLog, lGtvc);

		// -------------------
		renderer = gtk_cell_renderer_text_new ();
		g_object_set_property(G_OBJECT(renderer), "editable", &lGv);
		g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK (on_cell_edited), 0);
		g_signal_connect(G_OBJECT(renderer), "editing-canceled", G_CALLBACK (on_cell_editing_canceled), 0);
		g_signal_connect(G_OBJECT(renderer), "editing-started", G_CALLBACK (on_cell_editing_started), 0);
		gTs.mVColumnRenderers.push_back(renderer);

		lGtvc = gtk_tree_view_column_new_with_attributes ("End",
			renderer,
			"text", TkcEndTime,
			NULL);
		gtk_tree_view_append_column (gTs.mGtvTimeLog, lGtvc);

		// -------------------
		renderer = gtk_cell_renderer_text_new ();
		// Allow edit, but we won't actually accept changes.
		// this is to allow selecting and copying text.
		g_object_set_property(G_OBJECT(renderer), "editable", &lGv);
		g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK (on_cell_edited), 0);
		g_signal_connect(G_OBJECT(renderer), "editing-canceled", G_CALLBACK (on_cell_editing_canceled), 0);
		g_signal_connect(G_OBJECT(renderer), "editing-started", G_CALLBACK (on_cell_editing_started), 0);
		gTs.mVColumnRenderers.push_back(renderer);

		lGtvc = gtk_tree_view_column_new_with_attributes ("Duration",
			renderer,
			"text", TkcDuration,
			NULL);
		gtk_tree_view_append_column (gTs.mGtvTimeLog, lGtvc);

		// -------------------
		renderer = gtk_cell_renderer_text_new ();
		// Allow edit, but we won't actually accept changes.
		// this is to allow selecting and copying text.
		g_object_set_property(G_OBJECT(renderer), "editable", &lGv);
		g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK (on_cell_edited), 0);
		g_signal_connect(G_OBJECT(renderer), "editing-canceled", G_CALLBACK (on_cell_editing_canceled), 0);
		g_signal_connect(G_OBJECT(renderer), "editing-started", G_CALLBACK (on_cell_editing_started), 0);
		gTs.mVColumnRenderers.push_back(renderer);

		lGtvc = gtk_tree_view_column_new_with_attributes ("Daily Total",
			renderer,
			"text", TkcDailyTotal,
			NULL);
		gtk_tree_view_append_column (gTs.mGtvTimeLog, lGtvc);

		// -------------------
		renderer = gtk_cell_renderer_text_new ();
		// Allow edit, but we won't actually accept changes.
		// this is to allow selecting and copying text.
		g_object_set_property(G_OBJECT(renderer), "editable", &lGv);
		g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK (on_cell_edited), 0);
		g_signal_connect(G_OBJECT(renderer), "editing-canceled", G_CALLBACK (on_cell_editing_canceled), 0);
		g_signal_connect(G_OBJECT(renderer), "editing-started", G_CALLBACK (on_cell_editing_started), 0);
		gTs.mVColumnRenderers.push_back(renderer);

		lGtvc = gtk_tree_view_column_new_with_attributes ("Weekly Total",
			renderer,
			"text", TkcWeeklyTotal,
			NULL);
		gtk_tree_view_append_column (gTs.mGtvTimeLog, lGtvc);

		// Make headers visible.

		gtk_tree_view_set_headers_visible(gTs.mGtvTimeLog, true);

		lGw = glade_xml_get_widget(xml, "mGeTask");
		if (!lGw)
			throw Common::MessageException("Unable to find 'mGeTask' in timekeeper.glade file!");
		gTs.mGeTask = GTK_ENTRY(lGw);

		lGw = glade_xml_get_widget(xml, "mGeFrom");
		if (!lGw)
			throw Common::MessageException("Unable to find 'mGeFrom' in timekeeper.glade file!");
		gTs.mGeFrom = GTK_ENTRY(lGw);

		lGw = glade_xml_get_widget(xml, "mGeTo");
		if (!lGw)
			throw Common::MessageException("Unable to find 'mGeTo' in timekeeper.glade file!");
		gTs.mGeTo = GTK_ENTRY(lGw);

		lGw = glade_xml_get_widget(xml, "mGeOTThreshold");
		if (!lGw)
			throw Common::MessageException("Unable to find 'mGeOTThreshold' in timekeeper.glade file!");
		gTs.mGeOTThreshold = GTK_ENTRY(lGw);

		/* connect the signals in the interface */
		glade_xml_signal_autoconnect(xml);

		// Load the accel key group.   try here, after autoconnect.
		//gTs.mGtkAccelGroup = glade_xml_ensure_accel(xml);

		// Set default window size.
		gint lGiX, lGiY, lGiW, lGiH;
		if (gTs.mNvtPrefs.Find("MainX", lGiX) &&
			gTs.mNvtPrefs.Find("MainY", lGiY) &&
			gTs.mNvtPrefs.Find("MainW", lGiW) &&
			gTs.mNvtPrefs.Find("MainH", lGiH))
		{
	        GdkDisplay *lGd = gdk_display_get_default();
			GdkScreen *lGs = gdk_display_get_default_screen(lGd);
	        gint lGiSWidth = gdk_screen_get_width(lGs);
	        gint lGiSHeight = gdk_screen_get_height(lGs);

			if (0 <= lGiX && lGiX < lGiSWidth - lGiW &&
				0 <= lGiY && lGiY < lGiSHeight - lGiH)
			{
				// Size/position to the saved values.
				gtk_window_move(gTs.mGwMainWindow, lGiX, lGiY);
				gtk_window_resize(gTs.mGwMainWindow, lGiW, lGiH);
			}
		}

		// If there was a last file, open it.

		string lSLast = gTs.GetLastFile();

		if (lSLast != "")
		{
			open_filename(lSLast.c_str());
		}

		/* start the event loop */
		gtk_main();
	}
	catch (Common::Exception &aE)
	{
		cerr << "Exception: " << aE.Message() << "\n";
	}
	catch (Poco::Exception &aE)
	{
		cerr << "Exception: " << aE.message() << "\n";
	}

	return 0;
}

void on_new1_activate(GtkMenuItem *menuitem,
				  gpointer user_data)
{
	if (!gTs.mTk.IsEmpty())
	{
		// Prompt for save if there's data, then clear the list set.
	}

	gTs.mTk.Clear();
	gTs.ToListStore();
	gTs.mSFileName = "";
}

enum ExitCheckResponse
{
    EcrSaveNExit, EcrExitNoSave, EcrCancel
};
// ExitCheck returns true if ok to exit, false if not.
bool ExitCheck()
{
	if (!gTs.mBChanged)
		return true;            // ok to exit.

	// save check

	GtkWidget *dialog;

	dialog = gtk_dialog_new_with_buttons (0,
        gTs.mGwMainWindow,
		static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		"Save Changes",
		EcrSaveNExit,
		"Exit without saving",
		EcrExitNoSave,
		"Cancel",
		EcrCancel,
		0);

    string lSMsg;
    if (gTs.mSFileName == "")
        lSMsg = "Save to a new file?";
    else
        lSMsg = "Save changes to file:\n'" + gTs.mSFileName + "'?";

    // Add some text to the dialog.
    //GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    //GtkWidget *label = gtk_label_new (lSMsg.c_str());
    ///* Add the label, and show everything we've added to the dialog. */
    //gtk_container_add (GTK_CONTAINER (content_area), label);

    //gtk_widget_show_all (dialog);

	gint result = gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);

	switch (static_cast<ExitCheckResponse>(result))
	{
    case EcrSaveNExit:
        return gTs.Save();
    case EcrExitNoSave:
        return true;
    case EcrCancel:
    default:
        return false;
	}
}

void
on_quit1_activate (GtkMenuItem *menuitem,
				   gpointer user_data)
{
	if (ExitCheck())
	{
		gTs.SavePrefs();
		gtk_main_quit();
	}
}

void
on_about1_activate (GtkMenuItem *menuitem,
					gpointer user_data)
{
	AbsoluteTimePeriod lAtp(AbsoluteTimePeriod::TptWeek);

	ofstream lOfs("test.txt");
	lOfs << "Timeperiod: " <<
		DateTimeFormatter::format(lAtp.GetStart(), DateTimeFormat::SORTABLE_FORMAT, 0)
		<< " " <<
		DateTimeFormatter::format(lAtp.GetEnd(), DateTimeFormat::SORTABLE_FORMAT, 0) << endl;
	++lAtp;
	lOfs << "++Timeperiod: " <<
		DateTimeFormatter::format(lAtp.GetStart(), DateTimeFormat::SORTABLE_FORMAT, 0)
		<< " " <<
		DateTimeFormatter::format(lAtp.GetEnd(), DateTimeFormat::SORTABLE_FORMAT, 0) << endl;
	--lAtp;
	lOfs << "--Timeperiod: " <<
		DateTimeFormatter::format(lAtp.GetStart(), DateTimeFormat::SORTABLE_FORMAT, 0)
		<< " " <<
		DateTimeFormatter::format(lAtp.GetEnd(), DateTimeFormat::SORTABLE_FORMAT, 0) << endl;
	--lAtp;
	lOfs << "--Timeperiod: " <<
		DateTimeFormatter::format(lAtp.GetStart(), DateTimeFormat::SORTABLE_FORMAT, 0)
		<< " " <<
		DateTimeFormatter::format(lAtp.GetEnd(), DateTimeFormat::SORTABLE_FORMAT, 0) << endl;
	--lAtp;
	lOfs << "--Timeperiod: " <<
		DateTimeFormatter::format(lAtp.GetStart(), DateTimeFormat::SORTABLE_FORMAT, 0)
		<< " " <<
		DateTimeFormatter::format(lAtp.GetEnd(), DateTimeFormat::SORTABLE_FORMAT, 0) << endl;
	lAtp.SetToContain(Timestamp());
	lOfs << "SetToContain Timeperiod: " <<
		DateTimeFormatter::format(lAtp.GetStart(), DateTimeFormat::SORTABLE_FORMAT, 0)
		<< " " <<
		DateTimeFormatter::format(lAtp.GetEnd(), DateTimeFormat::SORTABLE_FORMAT, 0) << endl;

	gTs.mTk.ComputeTotals(AbsoluteTimePeriod::TptWeek, gTs.mBOnlyLast);
}

void on_TimeLogMain_destroy(GtkObject *object)
{
	gtk_main_quit();
}

gboolean on_TimeLogMain_delete_event (GtkWidget *widget,
                                    GdkEvent  *event,
                                    gpointer   user_data)
{
	if (ExitCheck())
	{
		gTs.SavePrefs();
		return false;		// quit away!
	}
	else
		return true;		// DON'T quit.
}

void on_LogIn_clicked (GtkButton *button,
					   gpointer user_data)
{
	gTs.mTk.NewEntry(gTs.mGeTask->text);
	gTs.mBChanged = true;

	gTs.ToListStore();

	gTs.SetRowFocus();

	gtk_widget_grab_focus(GTK_WIDGET(gTs.mGtvTimeLog));
}

void on_LogOut_clicked (GtkButton *button,
						gpointer user_data)
{
	GtkTreeSelection *lGts = gtk_tree_view_get_selection(gTs.mGtvTimeLog);

	bool lBSingle = false;
	if (lBSingle)
	{
		GtkTreeIter lGti;

		// find current row, update logout date of that.
		if (gtk_tree_selection_get_selected(lGts, 0, &lGti))
		{
			gtk_list_store_set (gTs.mGls, &lGti,
				TkcEndTime, DateTimeFormatter::format(ToLocalTime(Poco::Timestamp()), DateTimeFormat::SORTABLE_FORMAT, 0).c_str(),
				-1);

			gTs.mBChanged = true;
		}
	}
	else
	{
		GList *lGlTheList = gtk_tree_selection_get_selected_rows(lGts, 0);

		GList *lGl = lGlTheList;
		gpointer lGpLast = 0;
		while (lGl)
		{
			lGpLast = lGl->data;
			lGl = lGl->next;
		}

		if (lGpLast)
		{
			// If we found the last item, select it.
			GtkTreeIter lGti;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(gTs.mGls),
										 &lGti,
										 static_cast<GtkTreePath*>(lGpLast)))
			{
				gtk_list_store_set (gTs.mGls, &lGti,
					TkcEndTime, DateTimeFormatter::format(ToLocalTime(Poco::Timestamp()), DateTimeFormat::SORTABLE_FORMAT, 0).c_str(),
					-1);

				gTs.mBChanged = true;
			}
		}

		// Free the list results.
		g_list_foreach (lGlTheList, (GFunc)gtk_tree_path_free, NULL);
		g_list_free (lGlTheList);
	}

	// update the TimeKeeper dataset from the list store data.
	gTs.FromListStore();
	gTs.ToListStore();
	gTs.SetRowFocus();

	gtk_widget_grab_focus(GTK_WIDGET(gTs.mGtvTimeLog));
}

void on_open1_activate (GtkMenuItem *menuitem,
				   gpointer user_data)
{
	create_file_selection ();
}

void on_save1_activate (GtkMenuItem *menuitem,
				   gpointer user_data)
{
	gTs.Save();
}

void on_save_as1_activate (GtkMenuItem *menuitem,
					  gpointer user_data)
{
	gTs.SaveAs();
}

void on_tab_delimited_export_activate (GtkMenuItem *menuitem,
	gpointer user_data)
{
	gTs.TabDelimitedExport();
}

// ------------------------------------------------------------------------------------

void open_filename (const char *aCFileName)
{
	try
	{
		// Open the selected file, display data.
		gTs.mTk.Open(aCFileName);
		gTs.mSFileName = aCFileName;
		gTs.mBChanged = false;
		// Compute totals.
		gTs.mTk.ComputeTotals(AbsoluteTimePeriod::TptDay, gTs.mBOnlyLast);
		gTs.mTk.ComputeTotals(AbsoluteTimePeriod::TptWeek, gTs.mBOnlyLast);
		gTs.ToListStore();
		gTs.SetRowFocus();
		TimeKeeper::LeContainer::const_iterator lIter = gTs.mTk.mLcEntries.end();
		if (!gTs.mTk.mLcEntries.empty() && --lIter != gTs.mTk.mLcEntries.end())
		{
			gtk_entry_set_text(gTs.mGeTask, lIter->mSTask.c_str());
		}
	}
	catch (Common::Exception &aE)
	{
		GtkWidget *dialog = gtk_message_dialog_new (gTs.mGwMainWindow,
		                                  GTK_DIALOG_DESTROY_WITH_PARENT,
		                                  GTK_MESSAGE_ERROR,
		                                  GTK_BUTTONS_CLOSE,
		                                  aE.Message(),
		                                  NULL);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}
}
void open_file (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *file_selector = GTK_WIDGET (user_data);
	const gchar *selected_filename;

	selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));

	open_filename(selected_filename);
}

void create_file_selection (void)
{
	GtkWidget *file_selector;

	/* Create the selector */

	file_selector = gtk_file_selection_new ("Please select a file for editing.");

	// default to old filename.
	gtk_file_selection_set_filename(GTK_FILE_SELECTION (file_selector), gTs.GetLastFile().c_str());

	g_signal_connect (GTK_FILE_SELECTION (file_selector)->ok_button,
		"clicked",
		G_CALLBACK (open_file),
		file_selector);

	/* Ensure that the dialog box is destroyed when the user clicks a button. */

	g_signal_connect_swapped (GTK_FILE_SELECTION (file_selector)->ok_button,
		"clicked",
		G_CALLBACK (gtk_widget_destroy),
		file_selector);

	g_signal_connect_swapped (GTK_FILE_SELECTION (file_selector)->cancel_button,
		"clicked",
		G_CALLBACK (gtk_widget_destroy),
		file_selector);

	/* Display that dialog */

	gtk_widget_show (file_selector);
}

void on_cell_editing_canceled (GtkCellRenderer *cell)
{
	gTs.mBCellEditingInProgress = false;

//	GtkAccelGroup *lGagWhat = gTs.mGtkAccelGroup;
//	GtkMenuItem *lGmi = gTs.mGmiDelete;
//
//	GtkMenu *lGmDummy;
//
//	GtkAccelGroup *lGag = gtk_menu_get_accel_group(gTs.mGm);

//	gtk_widget_add_accelerator (GTK_WIDGET(gTs.mGmiDelete),
//                                 "activate", // GTK_RUN_ACTION,
//                                 lGag, // gTs.mGtkAccelGroup,
//                                 GDK_Delete,
//                                 (GdkModifierType)0,
////                                 GDK_CONTROL_MASK,
//                                 GTK_ACCEL_VISIBLE);

}
void on_cell_editing_started (GtkCellRenderer *cell,
							GtkCellEditable *editable,
							const gchar     *path)
{
	// Set 'cell editing is happening' flag.
	gTs.mBCellEditingInProgress = true;

	// remove the accelerator key from the delete menu.
//	gboolean lGb =
//   		gtk_widget_remove_accelerator(GTK_WIDGET(gTs.mGmiDelete),
//                                     gTs.mGtkAccelGroup,
//                                     GDK_Delete,
//                                     (GdkModifierType)0);
                                     //GDK_CONTROL_MASK);

//	GDK_VoidSymbol
//	gtk_menu_item_set_accel_path(gTs.mGmiDelete, "");

}

void  on_cell_edited (GtkCellRendererText *renderer,
					  gchar *path,
					  gchar *new_text,
					  gpointer user_data)
{
	gTs.mBCellEditingInProgress = false;

	// Which renderer is this?
	for (unsigned int lI = 0; lI < gTs.mVColumnRenderers.size(); ++lI)
	{
		if (gTs.mVColumnRenderers[lI] == GTK_CELL_RENDERER(renderer))
		{
			// Its column lI.  Update with new text?
			if (lI == TkcTask)
			{
				GtkTreeIter lGti;
				/* Modify a particular row */
				GtkTreePath *lGtp = gtk_tree_path_new_from_string (path);
				gtk_tree_model_get_iter (GTK_TREE_MODEL (gTs.mGls),
					&lGti,
					lGtp);
				gtk_tree_path_free(lGtp);
				gtk_list_store_set (gTs.mGls, &lGti,
					TkcTask, new_text,
					-1);

				gTs.mBChanged = true;
				// update.
				gTs.FromListStore();
				gTs.ToListStore();
			}
			else if (lI == TkcStartTime)
			{
				Poco::DateTime lDt;
				int lITzd;		// time zone differential.
				if (Poco::DateTimeParser::tryParse(new_text, lDt, lITzd))
				{
					// if its a valid time, then set.
					GtkTreeIter lGti;
					/* Modify a particular row */
					GtkTreePath *lGtp = gtk_tree_path_new_from_string (path);
					gtk_tree_model_get_iter (GTK_TREE_MODEL (gTs.mGls),
						&lGti,
						lGtp);
					gtk_tree_path_free(lGtp);
					gtk_list_store_set (gTs.mGls, &lGti,
						lI, DateTimeFormatter::format(lDt.timestamp(), DateTimeFormat::SORTABLE_FORMAT, 0).c_str(),
						-1);
					gTs.mBChanged = true;
				}
				// update.
				gTs.FromListStore();
				gTs.ToListStore();
			}
			else if (lI == TkcEndTime)
			{
				Poco::DateTime lDt;
				int lITzd;		// time zone differential.
				string lSNew("");
				if (Poco::DateTimeParser::tryParse(DateTimeFormat::SORTABLE_FORMAT, new_text, lDt, lITzd) && lDt.year() != 0)
				{
					lSNew = new_text;
				}
				else if (Poco::DateTimeParser::tryParse("%H:%M:%S", new_text, lDt, lITzd) && lDt.hour() != 0)
				{
					lSNew = new_text;
				}
				else if (Poco::DateTimeParser::tryParse(new_text, lDt, lITzd) && lDt.year() != 0)
				{
					// Some random format that DTP recognizes?
					lSNew = DateTimeFormatter::format(lDt.timestamp(), DateTimeFormat::SORTABLE_FORMAT, 0);
				}

				if (lSNew != "")
				{
					// if its a valid time, then set.
					GtkTreeIter lGti;
					/* Modify a particular row */
					GtkTreePath *lGtp = gtk_tree_path_new_from_string (path);
					gtk_tree_model_get_iter (GTK_TREE_MODEL (gTs.mGls),
						&lGti,
						lGtp);
					gtk_tree_path_free(lGtp);
					gtk_list_store_set (gTs.mGls, &lGti,
						lI, lSNew.c_str(),
						-1);
					gTs.mBChanged = true;

					// put in the new stuff.
					gTs.FromListStore();
				}
				// show new data with regular formatting, or restore old data.
				gTs.ToListStore();
			}
			else
			{
				// Other columns are not allowed to be edited.
			}
		}
	}
}

void on_delete1_activate (GtkMenuItem *menuitem,
						  gpointer user_data)
{
	DeleteSelectedRecords();
}

void DeleteSelectedRecords()
{
	// Shouldn't happen!
//	if (gTs.mBCellEditingInProgress)
//		return;

	GtkTreeSelection *lGts = gtk_tree_view_get_selection(gTs.mGtvTimeLog);
	GtkTreeIter lGti;

	bool lBSingle(false);
	if (lBSingle)
	{
		if (gtk_tree_selection_get_selected(lGts, 0, &lGti))
		{
			// delete the selected row from the model and update the timekeeper obj.
			gtk_list_store_remove(gTs.mGls, &lGti);
			gTs.mBChanged = true;
			gTs.FromListStore();
		}
	}
	else
	{
		GList *lGlTheList = gtk_tree_selection_get_selected_rows(lGts, 0);

		GList *lGl = lGlTheList;

		// Go to the last element of the list.
		GList *lGlLast =0;
		while (lGl)
		{
			lGlLast = lGl;
			lGl = lGl->next;
		}

		lGl = lGlLast;

		// Loop through the list backwards so we delete the right stuff.
		// deleting the first things first means the other paths are no longer accurate.
		while (lGl)
		{
			// If we found the last item, select it.
			GtkTreeIter lGti;
			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(gTs.mGls),
										 &lGti,
										 static_cast<GtkTreePath*>(lGl->data)))
			{
				gtk_list_store_remove(gTs.mGls, &lGti);

				gTs.mBChanged = true;
			}

			// on to the next.
			lGl = lGl->prev;
		}

		// Free the list results.
		g_list_foreach (lGlTheList, (GFunc)gtk_tree_path_free, NULL);
		g_list_free (lGlTheList);

		gTs.FromListStore();
	}
}

void on_reg_hours_activate (GtkMenuItem *menuitem,
	gpointer user_data)
{
	if (gTs.mBDecimalHours != false)
	{
		gTs.mBDecimalHours = false;
		gTs.ToListStore();
		gTs.SavePrefs();
	}
}

void on_dec_hours_activate (GtkMenuItem *menuitem,
	gpointer user_data)
{
	if (gTs.mBDecimalHours != true)
	{
		gTs.mBDecimalHours = true;
		gTs.ToListStore();
		gTs.SavePrefs();
	}
}

void on_time_report_activate (GtkMenuItem *menuitem,
	gpointer user_data)
{
	gtk_entry_set_text(gTs.mGeFrom,
		DateTimeFormatter::format(gTs.mDtReportFrom, DateTimeFormat::SORTABLE_FORMAT, 0).c_str());
	gtk_entry_set_text(gTs.mGeTo,
		DateTimeFormatter::format(gTs.mDtReportTo, DateTimeFormat::SORTABLE_FORMAT, 0).c_str());
	ostringstream lOss;
	lOss << gTs.mDOTHoursThreshold;
	gtk_entry_set_text(gTs.mGeOTThreshold,
		lOss.str().c_str());

	gtk_window_present(gTs.mGwReportOptions);
}

void on_ReportOptions_Ok_clicked (GtkButton *button,
	gpointer user_data)
{
	gtk_widget_hide(GTK_WIDGET(gTs.mGwReportOptions));

	// Get report datetime from/to from the dialog, then run the report.
	istringstream lIss(gTs.mGeOTThreshold->text);
	lIss >> gTs.mDOTHoursThreshold;

	int lITzd;
	if (Poco::DateTimeParser::tryParse(gTs.mGeFrom->text, gTs.mDtReportFrom, lITzd) &&
		Poco::DateTimeParser::tryParse(gTs.mGeTo->text, gTs.mDtReportTo, lITzd))
	{
		gTs.SavePrefs();
		gTs.TimeReport();
	}
}
void on_ReportOptions_Cancel_clicked (GtkButton *button,
	gpointer user_data)
{
	// Hide the window.
	gtk_widget_hide(GTK_WIDGET(gTs.mGwReportOptions));
}

gboolean on_mGtvTimeLog_key_release_event ( GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	if (event->keyval == GDK_Delete && !gTs.mBCellEditingInProgress)
	{
		DeleteSelectedRecords();
		return true;
	}
	else
		return false;
}
