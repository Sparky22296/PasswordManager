#include "PasswordManager/GUI/AccountEditor.h"

#include "PasswordManager/Data/Encryptor.h"
#include "PasswordManager/GUI/RandomiserDialog.h"

namespace PM
{
    AccountEditor::AccountEditor(wxWindow* parent) : wxDialog(parent, wxID_ANY, "Create Account", wxDefaultPosition, wxSize(500, 400)), m_IdentifierName("Username")
    {
        SetupGUI();
    }

    AccountEditor::AccountEditor(wxWindow* parent, const Account& account) : wxDialog(parent, wxID_ANY, "Edit Account", wxDefaultPosition, wxSize(500, 400)), m_Account(account), m_IdentifierName(account.GetIdentifierName())
    {
        SetupGUI();

        for (std::size_t i = 0; i < m_Account.GetFields().size(); ++i)
        {
            AddField(m_Account.GetFields()[i].first, m_Account.GetFields()[i].second);
        }
    }

    void AccountEditor::SetupGUI()
    {
        SetSizer(new wxBoxSizer(wxVERTICAL));

        wxScrolledWindow* contentWindow = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
        contentWindow->SetSizer(new wxBoxSizer(wxVERTICAL));
        contentWindow->SetWindowStyle(wxBORDER_DOUBLE);
        contentWindow->SetScrollRate(5, 5);
        contentWindow->Scroll(wxPoint(0, 0));
        GetSizer()->Add(contentWindow, 1, wxEXPAND | wxALL, 10);

        wxPanel* fieldsPanel = new wxPanel(contentWindow);
        fieldsPanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
        contentWindow->GetSizer()->Add(fieldsPanel, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 10);

        m_LabelsPanel = new wxPanel(fieldsPanel);
        m_LabelsPanel->SetSizer(new wxBoxSizer(wxVERTICAL));
        m_LabelsPanel->GetSizer()->AddSpacer(3);
        fieldsPanel->GetSizer()->Add(m_LabelsPanel, 0, wxEXPAND | wxRIGHT, 10);

        m_ValuesPanel = new wxPanel(fieldsPanel);
        m_ValuesPanel->SetSizer(new wxBoxSizer(wxVERTICAL));
        fieldsPanel->GetSizer()->Add(m_ValuesPanel, 1, wxEXPAND, 0);

        // System Field

        wxStaticText* systemLabel = new wxStaticText(m_LabelsPanel, wxID_ANY, "System");
        m_LabelsPanel->GetSizer()->Add(systemLabel, 0, wxBOTTOM, 10);
        m_SystemField = new wxTextCtrl(m_ValuesPanel, wxID_ANY, m_Account.GetSystem());
        m_ValuesPanel->GetSizer()->Add(m_SystemField, 0, wxEXPAND | wxBOTTOM, 3);

        // Username Field

        wxStaticText* usernameLabel = new wxStaticText(m_LabelsPanel, wxID_ANY, m_Account.GetIdentifierName() == "" ? "Username" : m_Account.GetIdentifierName());
        m_LabelsPanel->GetSizer()->Add(usernameLabel, 0, wxBOTTOM, 10);

        wxPanel* fieldPanel = new wxPanel(m_ValuesPanel);
        fieldPanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
        m_ValuesPanel->GetSizer()->Add(fieldPanel, 0, wxEXPAND | wxBOTTOM, 3);

        m_IdentifierField = new wxTextCtrl(fieldPanel, wxID_ANY, m_Account.GetIdentifier());
        fieldPanel->GetSizer()->Add(m_IdentifierField, 1, wxBOTTOM, 3);

        wxButton* renameButton = new wxButton(fieldPanel, wxID_ANY, "Rename");
        renameButton->Bind(wxEVT_LEFT_UP, [this, usernameLabel](wxMouseEvent& evt)
        {
            evt.Skip();

            wxTextEntryDialog dialog(this, "Enter a new name for the primary identifier.", "Rename Primary Identifier");

            if (dialog.ShowModal() == wxID_OK && dialog.GetValue().length() > 0)
            {
                usernameLabel->SetLabel(dialog.GetValue());
                m_IdentifierName = dialog.GetValue().ToStdString();

                Layout();
            }
        });
        fieldPanel->GetSizer()->Add(renameButton, 0, wxLEFT, 5);

        m_LabelsPanel->GetSizer()->AddSpacer(20);
        m_ValuesPanel->GetSizer()->AddSpacer(20);

        // Add Button

        wxButton* addFieldButton = new wxButton(contentWindow, wxID_ANY, "Add Field");
        addFieldButton->Bind(wxEVT_LEFT_UP, [this](wxMouseEvent& evt)
        {
            evt.Skip();

            wxTextEntryDialog dialog(this, "Please enter a name for the new field.", "Field Input");

            if (dialog.ShowModal() == wxID_OK)
            {
                AddField(dialog.GetValue().ToStdString());
            }
        });
        contentWindow->GetSizer()->Add(addFieldButton, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 10);
        
        // Buttons

        wxPanel* buttonPanel = new wxPanel(this);
        buttonPanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
        GetSizer()->Add(buttonPanel, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 10);

        wxButton* cancelButton = new wxButton(buttonPanel, wxID_ANY, "Cancel");
        cancelButton->Bind(wxEVT_LEFT_UP, [this](wxMouseEvent& evt)
        {
            Close(ReturnCode::CANCELLED);
        });
        buttonPanel->GetSizer()->Add(cancelButton);
        
        wxButton* submitButton = new wxButton(buttonPanel, wxID_ANY, "Submit");
        submitButton->Bind(wxEVT_LEFT_UP, [this](wxMouseEvent& evt)
        {
            Close(ReturnCode::EDITED);
        });
        buttonPanel->GetSizer()->Add(submitButton);

        // Binds

        Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& evt)
        {
            Close(ReturnCode::CANCELLED);
        });
    }

    void AccountEditor::ResolveAccount()
    {
        Account account;

        account.SetSystem(m_SystemField->GetValue().ToStdString());
        account.SetIdentifierName(m_IdentifierName);
        account.SetIdentifier(m_IdentifierField->GetValue().ToStdString());

        for (auto it = m_Fields.begin(); it != m_Fields.end(); ++it)
        {
            account.AddField(it->first, it->second->GetValue().ToStdString());
        }

        m_Account = account;
    }

    void AccountEditor::AddField(const std::string& fieldName, const std::string& value)
    {
        wxStaticText* fieldLabel = new wxStaticText(m_LabelsPanel, wxID_ANY, fieldName.c_str());
        m_LabelsPanel->GetSizer()->Add(fieldLabel, 0, wxBOTTOM, 10);

        wxPanel* fieldPanel = new wxPanel(m_ValuesPanel);
        fieldPanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
        m_ValuesPanel->GetSizer()->Add(fieldPanel, 0, wxEXPAND | wxBOTTOM, 3);

        wxTextCtrl* fieldInput = new wxTextCtrl(fieldPanel, wxID_ANY, value.c_str());
        fieldPanel->GetSizer()->Add(fieldInput, 1, 0, 0);

        wxPanel* buttonPanel = new wxPanel(fieldPanel);
        buttonPanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
        fieldPanel->GetSizer()->Add(buttonPanel, 0, wxLEFT, 5);

        wxButton* deleteButton = new wxButton(buttonPanel, wxID_ANY, "Delete");
        deleteButton->Bind(wxEVT_LEFT_UP, [this, fieldPanel, fieldLabel](wxMouseEvent& evt)
        {
            evt.Skip();
            
            for (std::size_t i = 0; i < m_Fields.size(); ++i)
            {
                if (m_Fields[i].first == fieldLabel->GetLabel().ToStdString())
                {
                    m_Fields.erase(m_Fields.begin() + i);
                    break;
                }
            }

            // Can't Destroy() these for some reason.
            fieldPanel->Hide();
            fieldLabel->Hide();

            Layout();
        });
        buttonPanel->GetSizer()->Add(deleteButton);

        wxButton* randomiseButton = new wxButton(buttonPanel, wxID_ANY, "Randomise");
        randomiseButton->Bind(wxEVT_LEFT_UP, [this, fieldInput](wxMouseEvent& evt)
        {
            evt.Skip();

            RandomiserDialog dialog(this);
            if (dialog.ShowModal() == RandomiserDialog::ReturnCode::OK)
            {
                fieldInput->SetValue(std::string((const char*)dialog.GetString().c_str(), dialog.GetString().length()));
            }
        });
        buttonPanel->GetSizer()->Add(randomiseButton);

        Layout();

        m_Fields.push_back(std::make_pair(fieldName, fieldInput));
    }

    void AccountEditor::Close(ReturnCode code)
    {
        if (code == ReturnCode::CANCELLED)
        {
            Account account = m_Account;
            ResolveAccount();

            if (account != m_Account)
            {
                wxMessageDialog dialog(this, "Are you sure you want to discard your changes?", "Confirm", wxOK | wxCANCEL);
                if (dialog.ShowModal() != wxID_OK)
                {
                    m_Account = account;
                    return;
                }
            }
        }
        else
        {
            ResolveAccount();
        }

        EndModal(code);
    }
}