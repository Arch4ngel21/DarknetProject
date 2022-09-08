#include "Application.h"

wxIMPLEMENT_APP(cApp);

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

int cApp::OnExit()
{
	if(mainFrame->showingRes)
	{
		if (remove(mainFrame->resFilePath.c_str()) != 0)
		{
			perror("Error deleting file");
			return -1;
		}
	}
	return 0;
}

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "DarkNet Prediction", wxPoint(30, 30), wxSize(1000, 600))
{
	// --------------------------------------- Image Panel ---------------------------------------
	drawPanel = new wxImagePanel(this, wxBITMAP_TYPE_JPEG);

	// ------------------------------------ File Search Panel -----------------------------------
	wxPanel *fileSearchPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(300, 500));
	fileSearchPanel->SetBackgroundColour(wxColor(100, 100, 100));

	// -------------------------------- Sizer for horizontal split -------------------------------
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(drawPanel, 1, wxEXPAND | wxALL, 10);
	sizer->Add(fileSearchPanel, 1, wxEXPAND | wxALL, 10);

	// ------------------------------------ Button Panel -----------------------------------
	wxPanel *buttonPanel = new wxPanel(fileSearchPanel, wxID_ANY, wxDefaultPosition, wxSize(300, 100));
	m_btn1 = new wxButton(buttonPanel, 10002, "Predict", wxDefaultPosition, wxSize(150, 30));
	error_text_field = new wxStaticText(buttonPanel, wxID_ANY, wxString(""), wxPoint(160, 0), wxSize(350, 30));
	error_text_field->SetForegroundColour(wxColour(255, 0, 0));
	error_text_field->SetFont(error_text_field->GetFont().Scale(1.5));

	// ------------------------------------ File select -----------------------------------
	m_file_ctrl = new wxFileCtrl(fileSearchPanel, 10001, wxString(".\\test_images"), wxEmptyString, wxString::FromAscii(wxFileSelectorDefaultWildcardStr), wxFC_OPEN, wxDefaultPosition, wxSize(300, 300));

	// ------------------------------------ Model select -----------------------------------
	wxPanel* choicePanel = new wxPanel(fileSearchPanel, wxID_ANY, wxDefaultPosition, wxSize(300, 100));
	wxString choices[] = { wxT("Large"), wxT("Medium"), wxT("Small"), wxT("Nano")};
	modelChoice = new wxChoice(choicePanel, 10003, wxDefaultPosition, wxSize(150, 30), 4, choices);
	wxStaticText *text_field = new wxStaticText(choicePanel, wxID_ANY, wxString("(NN Model)"), wxPoint(160, 5), wxSize(150, 30));


	// ------------------------------------ Sizer for right side -----------------------------------
	wxBoxSizer* fileSearchSizer = new wxBoxSizer(wxVERTICAL);
	fileSearchSizer->Add(m_file_ctrl, 1, wxEXPAND | wxALL, 10);
	fileSearchSizer->Add(choicePanel, 1, wxEXPAND | wxALL, 10);
	fileSearchSizer->Add(buttonPanel, 1, wxEXPAND | wxALL, 10);

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
		error_text_field->SetLabel(wxString("File not selected!"));
		return;
	}

	if (selectedModel == -1)
	{
		error_text_field->SetLabel(wxString("Model not selected!"));
		return;
	}

	if (selectedFilePath.compare(selectedFilePath.size() - 4, 4, wxString(".png")) == 0 ||
		selectedFilePath.compare(selectedFilePath.size() - 4, 4, wxString(".jpg")) == 0 ||
		selectedFilePath.compare(selectedFilePath.size() - 5, 5, wxString(".jpeg")) == 0)
	{
		error_text_field->SetLabel(wxEmptyString);
		string selectedFilePathString = string(selectedFilePath.mb_str());
		string selectedFileString = string(selectedFile.mb_str());
		predict(selectedFilePathString, selectedFileString, selectedModel);

		showingRes = true;
		wxString resFile = wxString("temp/") + selectedFile;
		drawPanel->loadNewImage(resFile);
		resFilePath = string(resFile.mb_str());
	}
	else
	{
		error_text_field->SetLabel(wxString("Selected file has unsupported format!"));
	}
}



// -------------------------- Image Viewer --------------------------
wxImagePanel::wxImagePanel(wxFrame* parent, wxBitmapType format) :
	wxPanel(parent)
{
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
			return;
		}
	}

	image.LoadFile(file, format);
	w = -1;
	h = -1;
	paintNow();
}



