/******************************************************************************/
// Utility routines.
/******************************************************************************/
/** @file GameSettings.cpp
 *     A frame which allows changing game settings file.
 * @par Purpose:
 *     Loads config file, allows changing it via GUI and then saves.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Jun 2011 - 12 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "GameSettings.hpp"

#include <cstddef>
#include <cstring>
#include <vector>
#include <sys/stat.h>
#include <stdexcept>
#include <wx/wx.h>
#include <wx/log.h>
#include <wx/filefn.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>
#include <wx/tokenzr.h>
#include <wx/valnum.h>
#include "wxCheckRadioBox.hpp"

wxString supported_boolean_code[] = {
    _T("OFF"),
    _T("ON"),
};

wxString supported_scrshotfmt_code[] = {
    _T("BMP"),
    _T("HSI"),
};

wxString supported_scrshotfmt_text[] = {
    _T("Windows bitmap (BMP)"),
    _T("HSI 'mhwanh' (RAW)"),
};

wxString supported_languages_code[] = {
    _T("ENG"),
    _T("FRE"),
    _T("GER"),
    _T("ITA"),
    _T("SPA"),
    _T("SWE"),
    _T("POL"),
    _T("DUT"),
    //_T("HUN"),
    //_T("AUS"),
    //_T("DAN"),
    //_T("NOR"),
    //_T("CES"),
    //_T("MAG"),
    _T("RUS"),
    _T("JAP"),
    _T("CHI"),
    _T("CHT"),
};

wxString supported_languages_text[] = {
    _T("English"),
    _T("Français"),
    _T("Deutsch"),
    _T("Italiano"),
    _T("Español"),
    _T("Svenska"),
    _T("Polski"),
    _T("Nederlands"),
    //_T("Magyar"),
    //_T("Aus"),
    //_T("Dansk"),
    //_T("Norsk"),
    //_T("Česky"),
    //_T("Mag"),
    _T("Русский"),
    _T("にほんご"),
    _T("简化中国"),
    _T("傳統的中國")
};

wxString tooltips_eng[] = {
    _T(""),
    _T("Select up to four resolutions. Note that original resolutions are 320x200 and 640x400; select 640x480x32 for best gaming experience. Switch between selected resolutions by pressing Alt+R during the game."),//1
    _T("Here you can type your own resolution. Use \"WIDTHxHEIGHTxCOLOUR\" scheme. Replace last \"x\" with \"w\" for windowed mode. You can select max. 4 resolutions."),//2
    _T("Screen resolution at which movies (ie. intro) will be played. Original is 320x200, and any higher resolution will make movie window smaller. Still, 640x480x32 is recommended, for compatibility."),
    _T("Screen resolution at which game menu is displayed. Original is 640x480 and it is recommended to select 640x480x32. Larger resolutions will make the menu smaller at center of the screen."),
    _T("Resolution used in case of screen setup failure. If the game cannot enter one of the selected resolutions (ie. in-game resolution), it will try to fall back into this resolution. 640x480x32 is recommended."),
    _T("Here you can select your language translation. This will affect the in-game messages, but also speeches during the game. Note that some campaigns may not support your language; in this case default one will be used."),
    _T("Enabling censorship will make only evil creatures to have blood, and will restrict death effect with exploding flesh. Originally, this was enabled in german language version."),
    _T("Increasing sensitivity will speed up the mouse in the game. This may also make the mouse less accurate, so be careful! Default value is 100; you can increase or decrease it at your will, but sometimes it may be better to change this setting in your OS."),
    _T("Captured screens can be saved in \"scrshots\" folder by pressing Shift+C during the game or inside menu. The HSI format isn't very popular nowadays, so you probably want to select BMP, as most graphics tools can open it."),
    _T("Write changes to \"keeperfx.cfg\" file."),
    _T("Abandon changes and close the window."),
};

wxString resolutions_ingame_init[] = {
    _T("320x200x8"),
    _T("640x400x8"),
    //_T("640x480x8"),
    _T("320x200x32"),
    _T("640x400x32"),
    _T("640x480x32"),
    _T("1024x768x32"),
    //_T("1280x800x32"),
    _T("1280x960x32"),
    _T("1280x1024x32"),
    _T("1440x900x32"),
    _T("1600x900x32"),
    //_T("1600x1200x32"),
    _T("1680x1050x32"),
    _T("1920x1080x32"),
    _T("1920x1200x32"),
    _T("2560x1440x32"),
    _T("2560x1600x32"),
};

wxString resolutions_movies_init[] = {
    _T("320x200x8"),
    _T("640x480x8"),
    _T("320x200x32"),
    _T("640x480x32"),
    _T("1024x768x32"),
};

wxString resolutions_menu_init[] = {
    _T("640x480x8"),
    _T("640x480x32"),
    _T("1024x768x32"),
    _T("1280x800x32"),
    _T("1280x960x32"),
};

wxString resolutions_failsafe_init[] = {
    _T("320x200x8"),
    _T("640x480x8"),
    _T("320x200x32"),
    _T("640x480x32"),
    _T("1024x768x32"),
};

int optionIndexInArray(const wxString * arr, size_t arr_len, const wxString &option)
{
    size_t i;
    for (i=0; i < arr_len; i++)
    {
        if (option.CmpNoCase(arr[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

BEGIN_EVENT_TABLE(GameSettings, wxDialog)
    EVT_CLOSE(GameSettings::OnClose)
    EVT_SHOW(GameSettings::OnShow)
    EVT_BUTTON(eventID_Save, GameSettings::OnSave)
    EVT_BUTTON(eventID_Cancel, GameSettings::OnCancel)
END_EVENT_TABLE()

GameSettings::GameSettings(wxFrame *parent)
    : wxDialog (parent, -1, wxT("Settings editor"), wxDefaultPosition, wxSize(460, 480))

{
    topsizer = new wxBoxSizer( wxVERTICAL );

    resIngameBox = new wxCheckRadioBox(this, wxID_ANY, wxT("In-game resolutions"), resolutions_ingame_init, WXSIZEOF(resolutions_ingame_init), 3 );
    resIngameBox->SetToolTip(tooltips_eng[1],tooltips_eng[2]);
    wxStaticBoxSizer* resIngameBoxSizer = new wxStaticBoxSizer( resIngameBox, wxHORIZONTAL );
    resIngameBoxSizer->Add(resIngameBox->rbPanel, 1, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance
    topsizer->Add(resIngameBoxSizer, 1, wxEXPAND);

    wxPanel *resOtherPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer *resOtherPanelSizer = new wxBoxSizer( wxHORIZONTAL );
    {
        wxStaticBox *resMenuBox = new wxStaticBox(resOtherPanel, wxID_ANY, wxT("Menu resolution") );
        resMenuBox->SetToolTip(tooltips_eng[4]);
        wxStaticBoxSizer* resMenuBoxSizer = new wxStaticBoxSizer(resMenuBox, wxVERTICAL);
        {
            resMenuCombo = new wxComboBox(resOtherPanel, wxID_ANY, resolutions_menu_init[0], wxDefaultPosition, wxDefaultSize,
                WXSIZEOF(resolutions_menu_init), resolutions_menu_init, wxCB_DROPDOWN);
            resMenuCombo->SetToolTip(tooltips_eng[4]);
            resMenuBoxSizer->Add(resMenuCombo, 1, wxEXPAND);
        }
        resOtherPanelSizer->Add(resMenuBoxSizer, 1, wxALIGN_CENTER);

        wxStaticBox *resMovieBox = new wxStaticBox(resOtherPanel, wxID_ANY, wxT("Movies resolution") );
        resMovieBox->SetToolTip(tooltips_eng[3]);
        wxStaticBoxSizer* resMovieBoxSizer = new wxStaticBoxSizer(resMovieBox, wxHORIZONTAL);
        {
            resMovieCombo = new wxComboBox(resOtherPanel, wxID_ANY, resolutions_movies_init[0], wxDefaultPosition, wxDefaultSize,
                WXSIZEOF(resolutions_movies_init), resolutions_movies_init, wxCB_DROPDOWN);
            resMovieCombo->SetToolTip(tooltips_eng[3]);
            resMovieBoxSizer->Add(resMovieCombo, 1, wxEXPAND);
        }
        resOtherPanelSizer->Add(resMovieBoxSizer, 1, wxALIGN_CENTER);

        wxStaticBox *resFailBox = new wxStaticBox(resOtherPanel, wxID_ANY, wxT("Failure resolution") );
        resFailBox->SetToolTip(tooltips_eng[5]);
        wxStaticBoxSizer* resFailBoxSizer = new wxStaticBoxSizer(resFailBox, wxVERTICAL);
        {
            resFailCombo = new wxComboBox(resOtherPanel, wxID_ANY, resolutions_failsafe_init[0], wxDefaultPosition, wxDefaultSize,
                WXSIZEOF(resolutions_failsafe_init), resolutions_failsafe_init, wxCB_DROPDOWN);
            resFailCombo->SetToolTip(tooltips_eng[5]);
            resFailBoxSizer->Add(resFailCombo, 1, wxEXPAND);
        }
        resOtherPanelSizer->Add(resFailBoxSizer, 1, wxALIGN_CENTER);
    }
    resOtherPanel->SetSizer(resOtherPanelSizer);
    topsizer->Add(resOtherPanel, 0, wxEXPAND);

    langRadio = new wxRadioBox( this, wxID_ANY, wxT("Language"), wxDefaultPosition, wxSize(-1, -1),
        WXSIZEOF(supported_languages_text), supported_languages_text, 4, wxRA_SPECIFY_COLS);
    langRadio->SetToolTip(tooltips_eng[6]);
    topsizer->Add(langRadio, 0, wxEXPAND);

    wxStaticBox *otherSettingsBox = new wxStaticBox( this, wxID_ANY, wxT("Other settings") );
    wxStaticBoxSizer* otherSettingsSizer = new wxStaticBoxSizer( otherSettingsBox, wxVERTICAL );
    {
        {
            censorChkBx = new wxCheckBox(this, wxID_ANY, wxT("Censorship (limit amount of blood and flesh)"));
            censorChkBx->SetToolTip(tooltips_eng[7]);
            censorChkBx->SetValue(false);
            otherSettingsSizer->Add(censorChkBx, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
        }
        {
            wxPanel *mouseSensitivityPanel = new wxPanel(this, wxID_ANY);
            wxBoxSizer *mouseSensitivityPanelSizer = new wxBoxSizer( wxHORIZONTAL );
            {
                wxStaticText *statTxt = new wxStaticText(mouseSensitivityPanel, wxID_ANY, wxT("Mouse sensitivity"));
                statTxt->SetToolTip(tooltips_eng[8]);
                mouseSensitivityPanelSizer->Add(statTxt, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
                mouseSensitivityPanelSizer->AddSpacer(16);
                wxIntegerValidator<long> mouseSensitivityVal(&mouseSensitivity, wxNUM_VAL_THOUSANDS_SEPARATOR);
                mouseSensitivityVal.SetRange(-10000,10000);
                mouseSensitvTxtCtrl = new wxTextCtrl(mouseSensitivityPanel, wxID_ANY, wxT("100"), wxDefaultPosition, wxSize(64, -1), 0, mouseSensitivityVal);
                mouseSensitvTxtCtrl->SetToolTip(tooltips_eng[8]);
                mouseSensitivityPanelSizer->Add(mouseSensitvTxtCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
            }
            mouseSensitivityPanel->SetSizer(mouseSensitivityPanelSizer);
            otherSettingsSizer->Add(mouseSensitivityPanel, 1, wxEXPAND);
        }
        otherSettingsSizer->SetMinSize(386, 64);
    }
    topsizer->Add(otherSettingsSizer, 0, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance

    scrshotRadio = new wxRadioBox( this, wxID_ANY, wxT("Screenshots"), wxDefaultPosition, wxDefaultSize,
        WXSIZEOF(supported_scrshotfmt_text), supported_scrshotfmt_text, 2, wxRA_SPECIFY_COLS );
    scrshotRadio->SetToolTip(tooltips_eng[9]);
    topsizer->Add(scrshotRadio, 0, wxEXPAND);

    wxPanel *dlgBottomPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer *dlgBottomPanelSizer = new wxBoxSizer( wxHORIZONTAL );
    {
        dlgBottomPanelSizer->AddStretchSpacer(1);
        wxButton *saveBtn = new wxButton(dlgBottomPanel, eventID_Save, _T("&Save") );
        saveBtn->SetToolTip(tooltips_eng[10]);
        dlgBottomPanelSizer->Add(saveBtn, 0, wxEXPAND);
        dlgBottomPanelSizer->AddStretchSpacer(1);
        wxButton *exitBtn = new wxButton(dlgBottomPanel, eventID_Cancel, _T("&Cancel") );
        exitBtn->SetToolTip(tooltips_eng[11]);
        dlgBottomPanelSizer->Add(exitBtn, 0, wxEXPAND);
        dlgBottomPanelSizer->AddStretchSpacer(1);
    }
    dlgBottomPanel->SetSizer(dlgBottomPanelSizer);
    topsizer->Add(dlgBottomPanel, 0, wxEXPAND);

    SetSizer(topsizer);
    Centre(wxBOTH);

}

GameSettings::~GameSettings()
{
    delete conf;
    conf = NULL;
}

void GameSettings::OnShow(wxShowEvent& event)
{
    if (event.IsShown()) {
        readConfiguration();
        return;
    }
}

void GameSettings::OnClose(wxCloseEvent& event)
{
    int msgRet;
    msgRet = wxMessageBox(_T("You've made changes to the configuration options. Do you want to store these changes?\n")
            _T("If you answer Yes, the previous configuration file will be replaced with the new settings you've selected."),
            _T("Save KeeperFX configuration"), wxYES_NO | wxCANCEL | wxYES_DEFAULT | wxICON_QUESTION, this);
    switch (msgRet)
    {
      case wxYES:      // Save, then destroy, quitting app
          writeConfiguration();
          wxLogMessage(wxT("Configuration saved."));
        break;
      case wxNO:       // Don't save; just destroy, quitting app
          wxLogMessage(wxT("Changed discarded."));
        break;
      case wxCANCEL:   // Do nothing - so don't quit app.
      default:
        if (event.CanVeto()) {
            event.Veto();     // Notify the calling code that we didn't delete the frame.
            return;
        }
        break;
    }

    GetParent()->Enable(true);

    event.Skip();
    EndModal(1);
}

void GameSettings::OnSave(wxCommandEvent& WXUNUSED(event))
{
    // Generate OnClose event
    Close();
}

void GameSettings::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    // End without generating OnClose event
    EndModal(0);
}

void GameSettings::readConfiguration()
{
    wxString value;
    int index;
    // Open the Configfile
    conf = new wxFileConfig(wxEmptyString, wxEmptyString, wxT("keeperfx.cfg"),
        wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH);

    //value = conf->Read(wxT("INSTALL_PATH"), wxT("./"));
    //value = conf->Read(wxT("INSTALL_TYPE"), wxT("MAX"));

    value = conf->Read(wxT("LANGUAGE"), supported_languages_code[0]);
    index = optionIndexInArray(supported_languages_code, WXSIZEOF(supported_languages_code), value);
    langRadio->SetSelection((index>=0)?index:0);

    //value = conf->Read(wxT("KEYBOARD"), wxT("101"));

    value = conf->Read(wxT("SCREENSHOT"), supported_scrshotfmt_code[0]);
    index = optionIndexInArray(supported_scrshotfmt_code, WXSIZEOF(supported_scrshotfmt_code), value);
    scrshotRadio->SetSelection((index>=0)?index:0);

    value = conf->Read(wxT("FRONTEND_RES"), wxT("640x480x32 640x480x32 640x480x32"));
    {
        wxStringTokenizer tokenz(value, wxT(" \t\r\n"));
        index = 0;
        while ( tokenz.HasMoreTokens() )
        {
            wxString param = tokenz.GetNextToken();
            switch (index)
            {
            case 0:
                resFailCombo->SetValue(param);
                break;
            case 1:
                resMovieCombo->SetValue(param);
                break;
            case 2:
                resMenuCombo->SetValue(param);
                break;
            }
            index++;
        }
    }

    value = conf->Read(wxT("INGAME_RES"), wxT("640x480x32 1024x768x32"));
    {
        wxString selected_resolutions[5];
        size_t selected_num;
        wxString disabled_resolution;
        wxStringTokenizer tokenz(value, wxT(" \t\r\n"));
        disabled_resolution = resFailCombo->GetValue();
        selected_num=0;
        while ( tokenz.HasMoreTokens() && (selected_num < 5) )
        {
            wxString param = tokenz.GetNextToken();
            if (param.CmpNoCase(disabled_resolution) != 0)
            {
                selected_resolutions[selected_num] = param;
                selected_num++;
            }
        }
        resIngameBox->SetSelected(4, selected_resolutions, selected_num);

    }

    index = conf->Read(wxT("POINTER_SENSITIVITY"), 100);
    value = wxString::Format(wxT("%d"), (int)index);
    mouseSensitvTxtCtrl->SetValue(value);

    value = conf->Read(wxT("CENSORSHIP"), supported_boolean_code[0]);
    index = optionIndexInArray(supported_boolean_code, WXSIZEOF(supported_boolean_code), value);
    censorChkBx->SetValue((index>=0)?index:0);
}

void GameSettings::writeConfiguration()
{
    wxString strValue;
    conf->Write(wxT("LANGUAGE"), supported_languages_code[langRadio->GetSelection()]);
    conf->Write(wxT("SCREENSHOT"), supported_scrshotfmt_code[scrshotRadio->GetSelection()]);
    strValue = wxT("640x480x32 640x480x32 640x480x32");
    conf->Write(wxT("FRONTEND_RES"), strValue);
    strValue = wxT("640x480x32 1024x768x32");
    conf->Write(wxT("INGAME_RES"), strValue);
    conf->Write(wxT("POINTER_SENSITIVITY"), mouseSensitivity);
    conf->Write(wxT("CENSORSHIP"), supported_boolean_code[censorChkBx->GetValue()]);
    //conf->Save(); -- saving is automatic when the object is destroyed
}


/******************************************************************************/
