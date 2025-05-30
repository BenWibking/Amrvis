// ABOUTME: ExpressionParser wrapper class for AMReX math expression parsing
// ABOUTME: Provides user-defined derived field functionality for Amrvis
// ---------------------------------------------------------------
// ExpressionParser.H
// ---------------------------------------------------------------
#ifndef _EXPRESSIONPARSER_H_
#define _EXPRESSIONPARSER_H_

#include <AMReX_Parser.H>
#include <AMReX_DataServices.H>
#include <AMReX_MultiFab.H>
#include <AMReX_Vector.H>

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

using amrex::Real;

// Forward declarations
class PltApp;

class ExpressionParser {

public:
    ExpressionParser();
    ~ExpressionParser();

    // Expression management
    bool SetExpression(const string& expression, const string& name = "");
    bool IsValidExpression(const string& expression) const;
    void ClearExpression();
    
    // Variable management
    void SetAvailableVariables(const amrex::Vector<string>& varNames);
    const amrex::Vector<string>& GetAvailableVariables() const;
    const amrex::Vector<string>& GetUsedVariables() const;
    
    // Expression evaluation
    bool EvaluateExpression(const amrex::MultiFab& inputData,
                           const amrex::Vector<string>& inputVarNames,
                           amrex::MultiFab& result,
                           int destComp = 0) const;
    
    // Single point evaluation for validation
    Real EvaluateAt(const amrex::Vector<Real>& varValues) const;
    
    // Accessors
    const string& GetExpression() const { return currentExpression; }
    const string& GetExpressionName() const { return expressionName; }
    bool HasValidExpression() const { return isValid; }
    
    // Error reporting
    const string& GetLastError() const { return lastError; }

private:
    string currentExpression;
    string expressionName;
    bool isValid;
    mutable string lastError;
    
    amrex::Vector<string> availableVariables;
    amrex::Vector<string> usedVariables;
    
    // AMReX parser instance
    mutable amrex::Parser parser;
    
    // Helper methods
    bool ValidateAndParse(const string& expression);
    void ExtractUsedVariables();
    bool CheckVariableAvailability() const;
};

// User-defined derived field management
class UserDerivedField {
public:
    UserDerivedField(const string& name, const string& expression);
    ~UserDerivedField();
    
    const string& GetName() const { return fieldName; }
    const string& GetExpression() const { return fieldExpression; }
    
    bool IsValid() const { return expressionParser.HasValidExpression(); }
    bool Evaluate(const amrex::MultiFab& inputData,
                  const amrex::Vector<string>& inputVarNames,
                  amrex::MultiFab& result) const;
    
    const ExpressionParser& GetParser() const { return expressionParser; }
    ExpressionParser& GetParser() { return expressionParser; }

private:
    string fieldName;
    string fieldExpression;
    ExpressionParser expressionParser;
};

// Manager for multiple user expressions
class ExpressionManager {
public:
    ExpressionManager();
    ~ExpressionManager();
    
    // Expression management
    bool AddExpression(const string& name, const string& expression);
    bool RemoveExpression(const string& name);
    void ClearAllExpressions();
    
    // Variable context
    void SetDataServices(amrex::DataServices* dataServices);
    void UpdateAvailableVariables();
    
    // Accessors
    const amrex::Vector<string>& GetExpressionNames() const;
    const UserDerivedField* GetExpression(const string& name) const;
    UserDerivedField* GetExpression(const string& name);
    
    // Evaluation
    bool EvaluateExpression(const string& name,
                           const amrex::MultiFab& inputData,
                           amrex::MultiFab& result) const;
    
    // Validation
    bool ValidateExpression(const string& expression) const;
    const amrex::Vector<string>& GetAvailableVariables() const;

private:
    map<string, UserDerivedField*> expressions;
    amrex::Vector<string> expressionNames;
    amrex::Vector<string> availableVariables;
    
    amrex::DataServices* dataServicesPtr;
    
    void UpdateExpressionNames();
    void UpdateVariableContext();
};

#endif