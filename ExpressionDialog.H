// ABOUTME: ExpressionDialog class for user-defined mathematical expression input
// ABOUTME: Provides Motif-based GUI for creating and managing derived fields
// ---------------------------------------------------------------
// ExpressionDialog.H
// ---------------------------------------------------------------
#ifndef _EXPRESSIONDIALOG_H_
#define _EXPRESSIONDIALOG_H_

#include <ExpressionParser.H>

#include <Xm/Xm.h>
#include <X11/Intrinsic.h>

#include <string>

using std::string;

// Forward declarations
class PltApp;

class ExpressionDialog {

public:
    ExpressionDialog(Widget parent, PltApp* pltApp);
    ~ExpressionDialog();

    // Dialog management
    void Show();
    void Hide();
    bool IsVisible() const { return isVisible; }
    
    // Expression management
    void SetAvailableVariables(const amrex::Vector<string>& variables);
    void RefreshVariableList();

private:
    // Parent widgets and app reference
    Widget parentWidget;
    PltApp* parentApp;
    
    // Main dialog widgets
    Widget wDialogShell;
    Widget wMainForm;
    Widget wTopForm;
    Widget wBottomForm;
    
    // Expression input area
    Widget wExpressionFrame;
    Widget wExpressionForm;
    Widget wExpressionLabel;
    Widget wExpressionText;
    Widget wNameLabel;
    Widget wNameText;
    
    // Variable browser area
    Widget wVariableFrame;
    Widget wVariableForm;
    Widget wVariableLabel;
    Widget wVariableList;
    Widget wVariableScrolled;
    
    // User expressions management area
    Widget wUserExprFrame;
    Widget wUserExprForm;
    Widget wUserExprLabel;
    Widget wUserExprList;
    Widget wUserExprScrolled;
    Widget wUserExprButtonForm;
    Widget wLoadButton;
    Widget wModifyButton;
    Widget wDeleteButton;
    
    // Status and validation area
    Widget wStatusFrame;
    Widget wStatusForm;
    Widget wStatusLabel;
    Widget wStatusText;
    
    // Button area
    Widget wButtonForm;
    Widget wValidateButton;
    Widget wAddButton;
    Widget wClearButton;
    Widget wCloseButton;
    
    // State
    bool isVisible;
    ExpressionManager* expressionManager;
    amrex::Vector<string> availableVariables;
    string selectedExpressionName;
    
    // Helper methods
    void CreateDialog();
    void CreateExpressionArea();
    void CreateVariableArea();
    void CreateUserExpressionArea();
    void CreateStatusArea();
    void CreateButtons();
    void UpdateStatus(const string& message, bool isError = false);
    void PopulateVariableList();
    void PopulateUserExpressionList();
    void RefreshUserExpressionList();
    
    // Callback methods
    static void CBValidateExpression(Widget w, XtPointer clientData, XtPointer callData);
    static void CBAddExpression(Widget w, XtPointer clientData, XtPointer callData);
    static void CBClearExpression(Widget w, XtPointer clientData, XtPointer callData);
    static void CBCloseDialog(Widget w, XtPointer clientData, XtPointer callData);
    static void CBVariableSelected(Widget w, XtPointer clientData, XtPointer callData);
    static void CBExpressionChanged(Widget w, XtPointer clientData, XtPointer callData);
    static void CBUserExpressionSelected(Widget w, XtPointer clientData, XtPointer callData);
    static void CBLoadExpression(Widget w, XtPointer clientData, XtPointer callData);
    static void CBModifyExpression(Widget w, XtPointer clientData, XtPointer callData);
    static void CBDeleteExpression(Widget w, XtPointer clientData, XtPointer callData);
    
    // Internal callback implementations
    void ValidateExpression();
    void AddExpression();
    void ClearExpression();
    void CloseDialog();
    void VariableSelected(int selectedIndex);
    void ExpressionChanged();
    void UserExpressionSelected(int selectedIndex);
    void LoadExpression();
    void ModifyExpression();
    void DeleteExpression();
    
    // Utility methods
    string GetExpressionText() const;
    string GetNameText() const;
    void SetExpressionText(const string& text);
    void SetNameText(const string& text);
    void InsertVariableAtCursor(const string& varName);
    string GetSelectedUserExpression() const;
    void EnableUserExpressionButtons(bool enable);
};

#endif