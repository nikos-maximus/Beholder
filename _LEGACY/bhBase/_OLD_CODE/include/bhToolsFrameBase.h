///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __BHTOOLSFRAMEBASE_H__
#define __BHTOOLSFRAMEBASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/gbsizer.h>
#include <wx/notebook.h>
#include <wx/statusbr.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class bhToolsFrameBase
///////////////////////////////////////////////////////////////////////////////
class bhToolsFrameBase : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* m_menubar1;
		wxMenu* m_menuFile;
		wxMenu* m_menuImport;
		wxNotebook* m_notebookMain;
		wxPanel* m_panelScene;
		wxPanel* m_panelSceneView;
		wxBoxSizer* bSizer_SceneView;
		wxPanel* m_panel6;
		wxTextCtrl* m_textCtrlSceneOutput;
		wxStatusBar* m_statusBar;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnFileNew( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFileOpen( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFileSave( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFileSaveAs( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFileExit( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnImportCollada( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnImportAssimp( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		bhToolsFrameBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Beholder Tools"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1052,706 ), long style = wxCAPTION|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxSYSTEM_MENU|wxTAB_TRAVERSAL );
		
		~bhToolsFrameBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class NewMapDialog
///////////////////////////////////////////////////////////////////////////////
class NewMapDialog : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText5;
		wxSpinCtrl* m_spinCtrlWidth;
		wxStaticText* m_staticText6;
		wxSpinCtrl* m_spinCtrlHeight;
		wxStaticText* m_staticText7;
		wxTextCtrl* m_textCtrlName;
		wxButton* m_buttonOK;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnOK( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		NewMapDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("New map"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~NewMapDialog();
	
};

#endif //__BHTOOLSFRAMEBASE_H__
