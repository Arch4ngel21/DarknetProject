#include "Application.h"

wxIMPLEMENT_APP(cApp);


// -------------------------- Application --------------------------
cApp::cApp()
{
	
}

cApp::~cApp()
{
	
}

bool cApp::OnInit()
{
	wxInitAllImageHandlers();

	mainFrame = new cMain();

	mainFrame->Show();
	// predict();
	return true;
}


// -------------------------- Main Frame --------------------------

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "DarkNet Prediction", wxPoint(30, 30), wxSize(1000, 600))
{
	drawPanel = new wxImagePanel(this, wxT("test_images/puppy.jpg"), wxBITMAP_TYPE_JPEG);
	wxPanel *fileSearchPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(300, 500));
	fileSearchPanel->SetBackgroundColour(wxColor(125, 125, 125));

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(drawPanel, 1, wxEXPAND | wxALL, 10);
	sizer->Add(fileSearchPanel, 1, wxEXPAND | wxALL, 10);

	wxPanel *buttonPanel = new wxPanel(fileSearchPanel, wxID_ANY, wxDefaultPosition, wxSize(300, 100));
	m_btn1 = new wxButton(buttonPanel, 10002, "Predict", wxDefaultPosition, wxSize(150, 30));
	m_file_ctrl = new wxFileCtrl(fileSearchPanel, 10001, wxString(".\\test_images"), wxEmptyString, wxString::FromAscii(wxFileSelectorDefaultWildcardStr), wxFC_OPEN, wxDefaultPosition, wxSize(300, 300));


	wxPanel* choicePanel = new wxPanel(fileSearchPanel, wxID_ANY, wxDefaultPosition, wxSize(300, 100));
	wxString choices[] = { wxT("Large"), wxT("Medium"), wxT("Small"), wxT("Nano")};
	modelChoice = new wxChoice(choicePanel, 10003, wxDefaultPosition, wxSize(150, 30), 4, choices);

	// wxPanel* errorPanel = new wxPanel(fileSearchPanel, wxID_ANY, wxDefaultPosition, wxSize(300, 100));
	// textCtrl = new wxTextCtrl(errorPanel, 10004, "Error message", wxDefaultPosition, wxSize(300, 100));

	wxBoxSizer* fileSearchSizer = new wxBoxSizer(wxVERTICAL);
	fileSearchSizer->Add(m_file_ctrl, 1, wxEXPAND | wxALL, 10);
	fileSearchSizer->Add(choicePanel, 1, wxEXPAND | wxALL, 10);
	fileSearchSizer->Add(buttonPanel, 1, wxEXPAND | wxALL, 10);
	// fileSearchSizer->Add(errorPanel, 1, wxEXPAND | wxALL, 10);

	fileSearchPanel->SetSizer(fileSearchSizer);

	this->SetSizer(sizer);
}

cMain::~cMain()
{
	
}

void cMain::onSelectionSwitched(wxFileCtrlEvent& event)
{
	wxString slash("\\");
	selectedFilePath = event.GetDirectory() + slash + event.GetFile();
	selectedFile = event.GetFile();
	drawPanel->setShow(true);
	drawPanel->loadNewImage(selectedFilePath);

	if (remove(resFilePath.c_str()) != 0)
	{
		perror("Error deleting file");
	}

	showingRes = false;
}

void cMain::onChoiceChanged(wxCommandEvent& event)
{
	selectedModel = event.GetSelection();
}


void cMain::onButtonClicked(wxCommandEvent& event)
{
	if (selectedFilePath.empty())
	{
		// textCtrl->WriteText(wxString("File path is empty!\n"));
		return;
	}

	if (selectedModel == -1)
	{
		// textCtrl->WriteText(wxString("Model not selected!\n"));
		return;
	}

	string selectedFilePathString = string(selectedFilePath.mb_str());
	string selectedFileString = string(selectedFile.mb_str());
	predict(selectedFilePathString, selectedFileString, selectedModel);

	showingRes = true;
	wxString resFile = wxString("temp/") + selectedFile;
	drawPanel->loadNewImage(resFile);
	resFilePath = string(resFile.mb_str());

	// char buffer[10];
	// textCtrl->WriteText(wxString(selectedFilePathString + " - " + selectedFileString + " - " + string(itoa(selectedModel, buffer, 10)) + " - "));
	// textCtrl->WriteText(wxString("Success!\n"));
}



// -------------------------- Image Viewer --------------------------
wxImagePanel::wxImagePanel(wxFrame* parent, wxString file, wxBitmapType format) :
	wxPanel(parent)
{
	image.LoadFile(file, format);
	blank = new wxImage(wxSize(100, 100), true);		// blank, when no image is selected

	// image current dimensions
	w = -1;
	h = -1;

	// blank image current dimensions
	b_w = -1;
	b_h = -1;
}

// Paint when event is registered
inline void wxImagePanel::paintEvent(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
	render(dc);
}

// Manually evoked paint
inline void wxImagePanel::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}

// Image render function - prints image or blank to 'dc'
void wxImagePanel::render(wxDC& dc)
{
	if (!shouldShow)
	{
		int img_w, img_h;
		dc.GetSize(&img_w, &img_h);

		if (b_w != img_w || b_h != img_h)
		{
			resized = wxBitmap(blank->Scale(img_w, img_h));
			b_w = img_w;
			b_h = img_h;
		}

		dc.DrawBitmap(resized, 0, 0, true);
		return;
	}


	int neww, newh;
	dc.GetSize(&neww, &newh);

	if (neww != w || newh != h)
	{
		resized = wxBitmap(image.Scale(neww, newh));
		w = neww;
		h = newh;
	}
	dc.DrawBitmap(resized, 0, 0, true);
}


// Handler for resize event
void wxImagePanel::OnSize(wxSizeEvent& event)
{
	Refresh();
	event.Skip();
}

void wxImagePanel::setShow(bool value)
{
	shouldShow = value;
}

// Function for changing images, when another is selected
void wxImagePanel::loadNewImage(wxString file)
{
	string converted_string = string(file.mb_str());
	wxBitmapType format;

	if (converted_string.compare(converted_string.size() - 4, 4, ".jpg") == 0 || converted_string.compare(converted_string.size() - 5, 5, ".jpeg") == 0)
	{
		format = wxBITMAP_TYPE_JPEG;
	}
	else
	{
		if (converted_string.compare(converted_string.size() - 4, 4, ".png") == 0)
		{
			format = wxBITMAP_TYPE_PNG;
		}
		else
		{
			cout << "Wrong file format!" << endl;
			return;
		}
	}

	image.LoadFile(file, format);
	w = -1;
	h = -1;
	paintNow();
}



