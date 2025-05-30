// ABOUTME: ExpressionDialog implementation for mathematical expression input GUI
// ABOUTME: Creates Motif-based dialog for user-defined derived field creation
// ---------------------------------------------------------------
// ExpressionDialog.cpp
// ---------------------------------------------------------------

#include <ExpressionDialog.H>
#include <PltApp.H>

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/PushB.h>
#include <Xm/DialogS.h>
#include <Xm/MessageB.h>
#include <Xm/Separator.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>

#include <iostream>
#include <sstream>

using std::cout;
using std::cerr;
using std::endl;

using namespace amrex;

// ===============================
// ExpressionDialog Implementation
// ===============================

ExpressionDialog::ExpressionDialog(Widget parent, PltApp* pltApp)
    : parentWidget(parent),
      parentApp(pltApp),
      wDialogShell(None),
      isVisible(false),
      expressionManager(nullptr)
{
    CreateDialog();
}

ExpressionDialog::~ExpressionDialog()
{
    if (wDialogShell != None) {
        XtDestroyWidget(wDialogShell);
    }
}

void ExpressionDialog::Show()
{
    if (wDialogShell != None) {
        XtManageChild(wMainForm);
        XtPopup(wDialogShell, XtGrabNone);
        
        // Ensure widgets are realized before populating the list
        if (!XtIsRealized(wDialogShell)) {
            XtRealizeWidget(wDialogShell);
        }
        
        RefreshVariableList();
        isVisible = true;
        UpdateStatus("Enter a mathematical expression using available variables", false);
    }
}

void ExpressionDialog::Hide()
{
    if (wDialogShell != None && isVisible) {
        XtPopdown(wDialogShell);
        XtUnmanageChild(wMainForm);
        isVisible = false;
    }
}

void ExpressionDialog::SetAvailableVariables(const Vector<string>& variables)
{
    availableVariables = variables;
    if (isVisible) {
        PopulateVariableList();
    }
}

void ExpressionDialog::RefreshVariableList()
{
    if (parentApp && parentApp->GetExpressionManager()) {
        parentApp->GetExpressionManager()->UpdateAvailableVariables();
        availableVariables = parentApp->GetExpressionManager()->GetAvailableVariables();
        PopulateVariableList();
    }
}

void ExpressionDialog::CreateDialog()
{
    Arg args[20];
    int n;
    
    // Create main dialog shell
    n = 0;
    XtSetArg(args[n], XmNtitle, "User-Defined Expression"); n++;
    XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); n++;
    XtSetArg(args[n], XmNwidth, 600); n++;
    XtSetArg(args[n], XmNheight, 500); n++;
    wDialogShell = XmCreateDialogShell(parentWidget, const_cast<char*>("expressionDialog"), args, n);
    
    // Set up close protocol
    Display* display = XtDisplay(wDialogShell);
    Atom WM_DELETE_WINDOW = XmInternAtom(display, 
                                         const_cast<char*>("WM_DELETE_WINDOW"), False);
    XmAddWMProtocols(wDialogShell, &WM_DELETE_WINDOW, 1);
    XmAddWMProtocolCallback(wDialogShell, WM_DELETE_WINDOW,
                           (XtCallbackProc) CBCloseDialog, (XtPointer) this);
    
    // Create main form
    n = 0;
    wMainForm = XmCreateForm(wDialogShell, const_cast<char*>("mainForm"), args, n);
    
    CreateExpressionArea();
    CreateVariableArea();
    CreateStatusArea();
    CreateButtons();
    
    // Don't manage the main form here - only manage it when showing the dialog
}

void ExpressionDialog::CreateExpressionArea()
{
    Arg args[20];
    int n;
    
    // Expression frame
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginWidth, 5); n++;
    XtSetArg(args[n], XmNmarginHeight, 5); n++;
    wExpressionFrame = XmCreateFrame(wMainForm, const_cast<char*>("expressionFrame"), args, n);
    
    // Expression form inside frame
    n = 0;
    wExpressionForm = XmCreateForm(wExpressionFrame, const_cast<char*>("expressionForm"), args, n);
    
    // Expression label
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginTop, 10); n++;
    XtSetArg(args[n], XmNmarginLeft, 10); n++;
    XmString labelStr = XmStringCreateSimple(const_cast<char*>("Expression:"));
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    wExpressionLabel = XmCreateLabel(wExpressionForm, const_cast<char*>("expressionLabel"), args, n);
    XmStringFree(labelStr);
    
    // Expression text field
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, wExpressionLabel); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginLeft, 10); n++;
    XtSetArg(args[n], XmNmarginRight, 10); n++;
    XtSetArg(args[n], XmNmarginTop, 5); n++;
    XtSetArg(args[n], XmNrows, 3); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, True); n++;
    XtSetArg(args[n], XmNscrollHorizontal, False); n++;
    XtSetArg(args[n], XmNscrollVertical, True); n++;
    wExpressionText = XmCreateScrolledText(wExpressionForm, const_cast<char*>("expressionText"), args, n);
    XtAddCallback(wExpressionText, XmNmodifyVerifyCallback, CBExpressionChanged, (XtPointer) this);
    
    // Name label
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, XtParent(wExpressionText)); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginTop, 10); n++;
    XtSetArg(args[n], XmNmarginLeft, 10); n++;
    labelStr = XmStringCreateSimple(const_cast<char*>("Name:"));
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    wNameLabel = XmCreateLabel(wExpressionForm, const_cast<char*>("nameLabel"), args, n);
    XmStringFree(labelStr);
    
    // Name text field
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, wNameLabel); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginLeft, 10); n++;
    XtSetArg(args[n], XmNmarginRight, 10); n++;
    XtSetArg(args[n], XmNmarginTop, 5); n++;
    XtSetArg(args[n], XmNmarginBottom, 10); n++;
    wNameText = XmCreateText(wExpressionForm, const_cast<char*>("nameText"), args, n);
    
    XtManageChild(wExpressionLabel);
    XtManageChild(wExpressionText);
    XtManageChild(wNameLabel);
    XtManageChild(wNameText);
    XtManageChild(wExpressionForm);
    XtManageChild(wExpressionFrame);
}

void ExpressionDialog::CreateVariableArea()
{
    Arg args[20];
    int n;
    
    // Variable frame
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, wExpressionFrame); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginWidth, 5); n++;
    XtSetArg(args[n], XmNmarginHeight, 5); n++;
    XtSetArg(args[n], XmNmarginTop, 5); n++;
    wVariableFrame = XmCreateFrame(wMainForm, const_cast<char*>("variableFrame"), args, n);
    
    // Variable form inside frame
    n = 0;
    wVariableForm = XmCreateForm(wVariableFrame, const_cast<char*>("variableForm"), args, n);
    
    // Variable label
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginTop, 10); n++;
    XtSetArg(args[n], XmNmarginLeft, 10); n++;
    XmString labelStr = XmStringCreateSimple(const_cast<char*>("Available Variables (double-click to insert):"));
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    wVariableLabel = XmCreateLabel(wVariableForm, const_cast<char*>("variableLabel"), args, n);
    XmStringFree(labelStr);
    
    // Variable list (scrolled)
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, wVariableLabel); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginLeft, 10); n++;
    XtSetArg(args[n], XmNmarginRight, 10); n++;
    XtSetArg(args[n], XmNmarginTop, 5); n++;
    XtSetArg(args[n], XmNmarginBottom, 10); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 8); n++;
    XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT); n++;
    wVariableList = XmCreateScrolledList(wVariableForm, const_cast<char*>("variableList"), args, n);
    XtAddCallback(wVariableList, XmNdefaultActionCallback, CBVariableSelected, (XtPointer) this);
    
    XtManageChild(wVariableLabel);
    XtManageChild(wVariableList);
    XtManageChild(wVariableForm);
    XtManageChild(wVariableFrame);
}

void ExpressionDialog::CreateStatusArea()
{
    Arg args[20];
    int n;
    
    // Status frame
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, wVariableFrame); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginWidth, 5); n++;
    XtSetArg(args[n], XmNmarginHeight, 5); n++;
    XtSetArg(args[n], XmNmarginTop, 5); n++;
    wStatusFrame = XmCreateFrame(wMainForm, const_cast<char*>("statusFrame"), args, n);
    
    // Status form inside frame
    n = 0;
    wStatusForm = XmCreateForm(wStatusFrame, const_cast<char*>("statusForm"), args, n);
    
    // Status label
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginTop, 10); n++;
    XtSetArg(args[n], XmNmarginLeft, 10); n++;
    XmString labelStr = XmStringCreateSimple(const_cast<char*>("Status:"));
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    wStatusLabel = XmCreateLabel(wStatusForm, const_cast<char*>("statusLabel"), args, n);
    XmStringFree(labelStr);
    
    // Status text (read-only)
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, wStatusLabel); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginLeft, 10); n++;
    XtSetArg(args[n], XmNmarginRight, 10); n++;
    XtSetArg(args[n], XmNmarginTop, 5); n++;
    XtSetArg(args[n], XmNmarginBottom, 10); n++;
    XtSetArg(args[n], XmNeditable, False); n++;
    XtSetArg(args[n], XmNrows, 2); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, True); n++;
    wStatusText = XmCreateText(wStatusForm, const_cast<char*>("statusText"), args, n);
    
    XtManageChild(wStatusLabel);
    XtManageChild(wStatusText);
    XtManageChild(wStatusForm);
    XtManageChild(wStatusFrame);
}

void ExpressionDialog::CreateButtons()
{
    Arg args[20];
    int n;
    
    // Button form
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, wStatusFrame); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNmarginWidth, 10); n++;
    XtSetArg(args[n], XmNmarginHeight, 10); n++;
    XtSetArg(args[n], XmNmarginTop, 10); n++;
    wButtonForm = XmCreateForm(wMainForm, const_cast<char*>("buttonForm"), args, n);
    
    // Validate button
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNwidth, 80); n++;
    XmString labelStr = XmStringCreateSimple(const_cast<char*>("Validate"));
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    wValidateButton = XmCreatePushButton(wButtonForm, const_cast<char*>("validateButton"), args, n);
    XmStringFree(labelStr);
    XtAddCallback(wValidateButton, XmNactivateCallback, CBValidateExpression, (XtPointer) this);
    
    // Add button
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, wValidateButton); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNwidth, 80); n++;
    labelStr = XmStringCreateSimple(const_cast<char*>("Add"));
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    wAddButton = XmCreatePushButton(wButtonForm, const_cast<char*>("addButton"), args, n);
    XmStringFree(labelStr);
    XtAddCallback(wAddButton, XmNactivateCallback, CBAddExpression, (XtPointer) this);
    
    // Clear button
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, wAddButton); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNwidth, 80); n++;
    labelStr = XmStringCreateSimple(const_cast<char*>("Clear"));
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    wClearButton = XmCreatePushButton(wButtonForm, const_cast<char*>("clearButton"), args, n);
    XmStringFree(labelStr);
    XtAddCallback(wClearButton, XmNactivateCallback, CBClearExpression, (XtPointer) this);
    
    // Close button
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNwidth, 80); n++;
    labelStr = XmStringCreateSimple(const_cast<char*>("Close"));
    XtSetArg(args[n], XmNlabelString, labelStr); n++;
    wCloseButton = XmCreatePushButton(wButtonForm, const_cast<char*>("closeButton"), args, n);
    XmStringFree(labelStr);
    XtAddCallback(wCloseButton, XmNactivateCallback, CBCloseDialog, (XtPointer) this);
    
    XtManageChild(wValidateButton);
    XtManageChild(wAddButton);
    XtManageChild(wClearButton);
    XtManageChild(wCloseButton);
    XtManageChild(wButtonForm);
}

void ExpressionDialog::UpdateStatus(const string& message, bool isError)
{
    if (wStatusText != None) {
        XmTextSetString(wStatusText, const_cast<char*>(message.c_str()));
        
        // Change background color based on error status
        Pixel color;
        if (isError) {
            XtVaGetValues(wStatusText, XmNselectColor, &color, NULL);
        } else {
            XtVaGetValues(wStatusText, XmNbackground, &color, NULL);
        }
        XtVaSetValues(wStatusText, XmNbackground, color, NULL);
    }
}

void ExpressionDialog::PopulateVariableList()
{
    if (wVariableList == None) return;
    
    // Clear existing items
    XmListDeleteAllItems(wVariableList);
    
    // Add variables to list
    for (const auto& var : availableVariables) {
        XmString xmStr = XmStringCreateSimple(const_cast<char*>(var.c_str()));
        XmListAddItem(wVariableList, xmStr, 0);
        XmStringFree(xmStr);
    }
}

// ===============================
// Callback Methods
// ===============================

void ExpressionDialog::CBValidateExpression(Widget w, XtPointer clientData, XtPointer callData)
{
    amrex::ignore_unused(w, callData);
    ExpressionDialog* dialog = static_cast<ExpressionDialog*>(clientData);
    dialog->ValidateExpression();
}

void ExpressionDialog::CBAddExpression(Widget w, XtPointer clientData, XtPointer callData)
{
    amrex::ignore_unused(w, callData);
    ExpressionDialog* dialog = static_cast<ExpressionDialog*>(clientData);
    dialog->AddExpression();
}

void ExpressionDialog::CBClearExpression(Widget w, XtPointer clientData, XtPointer callData)
{
    amrex::ignore_unused(w, callData);
    ExpressionDialog* dialog = static_cast<ExpressionDialog*>(clientData);
    dialog->ClearExpression();
}

void ExpressionDialog::CBCloseDialog(Widget w, XtPointer clientData, XtPointer callData)
{
    amrex::ignore_unused(w, callData);
    ExpressionDialog* dialog = static_cast<ExpressionDialog*>(clientData);
    dialog->CloseDialog();
}

void ExpressionDialog::CBVariableSelected(Widget w, XtPointer clientData, XtPointer callData)
{
    amrex::ignore_unused(w);
    ExpressionDialog* dialog = static_cast<ExpressionDialog*>(clientData);
    XmListCallbackStruct* cbs = static_cast<XmListCallbackStruct*>(callData);
    dialog->VariableSelected(cbs->item_position - 1); // Convert to 0-based index
}

void ExpressionDialog::CBExpressionChanged(Widget w, XtPointer clientData, XtPointer callData)
{
    amrex::ignore_unused(w, callData);
    ExpressionDialog* dialog = static_cast<ExpressionDialog*>(clientData);
    dialog->ExpressionChanged();
}

// ===============================
// Internal Callback Implementations
// ===============================

void ExpressionDialog::ValidateExpression()
{
    string expr = GetExpressionText();
    if (expr.empty()) {
        UpdateStatus("Enter an expression to validate", true);
        return;
    }
    
    if (parentApp && parentApp->GetExpressionManager()) {
        bool isValid = parentApp->GetExpressionManager()->ValidateExpression(expr);
        if (isValid) {
            UpdateStatus("Expression is valid", false);
        } else {
            UpdateStatus("Invalid expression syntax", true);
        }
    } else {
        UpdateStatus("Expression manager not available", true);
    }
}

void ExpressionDialog::AddExpression()
{
    string expr = GetExpressionText();
    string name = GetNameText();
    
    if (expr.empty()) {
        UpdateStatus("Enter an expression first", true);
        return;
    }
    
    if (name.empty()) {
        UpdateStatus("Enter a name for the expression", true);
        return;
    }
    
    if (parentApp && parentApp->GetExpressionManager()) {
        bool success = parentApp->GetExpressionManager()->AddExpression(name, expr);
        if (success) {
            UpdateStatus("Expression '" + name + "' added successfully", false);
            ClearExpression();
            // Notify parent app to refresh derived variable menu
            parentApp->RefreshDerivedMenu();
        } else {
            UpdateStatus("Failed to add expression - check syntax and variable names", true);
        }
    } else {
        UpdateStatus("Expression manager not available", true);
    }
}

void ExpressionDialog::ClearExpression()
{
    SetExpressionText("");
    SetNameText("");
    UpdateStatus("Cleared", false);
}

void ExpressionDialog::CloseDialog()
{
    Hide();
}

void ExpressionDialog::VariableSelected(int selectedIndex)
{
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(availableVariables.size())) {
        const string& varName = availableVariables[selectedIndex];
        InsertVariableAtCursor(varName);
    }
}

void ExpressionDialog::ExpressionChanged()
{
    UpdateStatus("Expression modified", false);
}

// ===============================
// Utility Methods
// ===============================

string ExpressionDialog::GetExpressionText() const
{
    if (wExpressionText == None) return "";
    
    char* text = XmTextGetString(wExpressionText);
    string result(text);
    XtFree(text);
    return result;
}

string ExpressionDialog::GetNameText() const
{
    if (wNameText == None) return "";
    
    char* text = XmTextGetString(wNameText);
    string result(text);
    XtFree(text);
    return result;
}

void ExpressionDialog::SetExpressionText(const string& text)
{
    if (wExpressionText != None) {
        XmTextSetString(wExpressionText, const_cast<char*>(text.c_str()));
    }
}

void ExpressionDialog::SetNameText(const string& text)
{
    if (wNameText != None) {
        XmTextSetString(wNameText, const_cast<char*>(text.c_str()));
    }
}

void ExpressionDialog::InsertVariableAtCursor(const string& varName)
{
    if (wExpressionText == None) return;
    
    XmTextPosition cursorPos = XmTextGetCursorPosition(wExpressionText);
    XmTextInsert(wExpressionText, cursorPos, const_cast<char*>(varName.c_str()));
    
    // Move cursor to end of inserted text
    XmTextSetCursorPosition(wExpressionText, cursorPos + varName.length());
    XmTextSetHighlight(wExpressionText, cursorPos, cursorPos + varName.length(), XmHIGHLIGHT_SELECTED);
    
    UpdateStatus("Variable '" + varName + "' inserted", false);
}