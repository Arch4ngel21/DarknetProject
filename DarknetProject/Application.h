#include "wx/wx.h"
#include "wx/filectrl.h"
#include "wx/sizer.h"
#include "Darknet.h"

#ifndef wxUSE_FLECTRL
#define wxUSE_FILECTRL 1
#endif


class wxImagePanel : public wxPanel
{
	wxImage image;
	wxBitmap resized;
	wxImage* blank;
	int w, h;
	int b_w, b_h;
	bool shouldShow = false;

public:
	wxImagePanel(wxFrame* parent, wxBitmapType format);

	void paintEvent(wxPaintEvent& evt);
	void paintNow();
	void OnSize(wxSizeEvent& event);
	void render(wxDC& dc);

	void setShow(bool value);
	void loadNewImage(wxString file);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxImagePanel, wxPanel)
EVT_PAINT(wxImagePanel::paintEvent)
EVT_SIZE(wxImagePanel::OnSize)
END_EVENT_TABLE()


class cMain : public wxFrame
{

public:
	cMain();
	~cMain();

public:
	wxButton* m_btn1 = nullptr;
	wxFileCtrl* m_file_ctrl = nullptr;
	wxImagePanel* drawPanel = nullptr;
	wxChoice* modelChoice = nullptr;
	wxStaticText* error_text_field = nullptr;

	wxString selectedFile;
	wxString selectedFilePath;
	int selectedModel = -1;
	string resFilePath;

	bool showingRes = false;

	void onSelectionSwitched(wxFileCtrlEvent& event);
	void onButtonClicked(wxCommandEvent& event);
	void onChoiceChanged(wxCommandEvent& event);

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(cMain, wxFrame)
EVT_FILECTRL_SELECTIONCHANGED(10001, cMain::onSelectionSwitched)
EVT_BUTTON(10002, cMain::onButtonClicked)
EVT_CHOICE(10003, cMain::onChoiceChanged)
END_EVENT_TABLE()


class cApp : public wxApp
{
public:
	cApp();
	~cApp();

private:
	cMain* mainFrame = nullptr;

public:
	virtual bool OnInit();
	virtual int OnExit();
};


