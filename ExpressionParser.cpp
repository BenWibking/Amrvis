// ABOUTME: ExpressionParser implementation for AMReX math expression parsing
// ABOUTME: Handles compilation, validation, and evaluation of user expressions
// ---------------------------------------------------------------
// ExpressionParser.cpp
// ---------------------------------------------------------------

#include <ExpressionParser.H>
#include <AMReX_ParmParse.H>
#include <AMReX_Print.H>

#include <algorithm>
#include <sstream>
#include <cctype>

using namespace amrex;

// ===============================
// ExpressionParser Implementation
// ===============================

ExpressionParser::ExpressionParser()
    : currentExpression(""),
      expressionName(""),
      isValid(false),
      lastError("")
{
}

ExpressionParser::~ExpressionParser()
{
}

bool ExpressionParser::SetExpression(const string& expression, const string& name)
{
    currentExpression = expression;
    expressionName = name.empty() ? "user_expr" : name;
    lastError = "";
    
    if (expression.empty()) {
        isValid = false;
        lastError = "Empty expression";
        return false;
    }
    
    // Create mapped expression for parsing
    mappedExpression = CreateMappedExpression(expression);
    
    isValid = ValidateAndParse(mappedExpression);
    if (isValid) {
        ExtractUsedVariables();
        isValid = CheckVariableAvailability();
    }
    
    return isValid;
}

bool ExpressionParser::IsValidExpression(const string& expression) const
{
    if (expression.empty()) {
        return false;
    }
    
    try {
        Parser testParser(expression);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void ExpressionParser::ClearExpression()
{
    currentExpression = "";
    expressionName = "";
    mappedExpression = "";
    isValid = false;
    lastError = "";
    usedVariables.clear();
    originalToMapped.clear();
    mappedToOriginal.clear();
}

void ExpressionParser::SetAvailableVariables(const Vector<string>& varNames)
{
    availableVariables = varNames;
    
    // Create variable mapping for non-standard identifiers
    CreateVariableMapping();
    
    // Re-validate current expression if we have one
    if (!currentExpression.empty()) {
        SetExpression(currentExpression, expressionName);
    }
}

const Vector<string>& ExpressionParser::GetAvailableVariables() const
{
    return availableVariables;
}

const Vector<string>& ExpressionParser::GetUsedVariables() const
{
    return usedVariables;
}

bool ExpressionParser::EvaluateExpression(const MultiFab& inputData,
                                         const Vector<string>& inputVarNames,
                                         MultiFab& result,
                                         int destComp) const
{
    if (!isValid) {
        lastError = "Expression not valid";
        return false;
    }
    
    if (usedVariables.size() == 0) {
        lastError = "No variables used in expression";
        return false;
    }
    
    try {
        // Create variable mapping
        Vector<int> varIndices(usedVariables.size(), -1);
        for (int i = 0; i < usedVariables.size(); ++i) {
            for (int j = 0; j < inputVarNames.size(); ++j) {
                if (usedVariables[i] == inputVarNames[j]) {
                    varIndices[i] = j;
                    break;
                }
            }
            if (varIndices[i] == -1) {
                lastError = "Variable '" + usedVariables[i] + "' not found in input data";
                return false;
            }
        }
        
        // Create a new parser for evaluation with mapped expression and mapped variable names
        Parser evalParser(mappedExpression);
        
        // Create mapped variable names for registration
        Vector<string> mappedUsedVars;
        for (const auto& var : usedVariables) {
            auto it = originalToMapped.find(var);
            if (it != originalToMapped.end()) {
                mappedUsedVars.push_back(it->second);
            } else {
                mappedUsedVars.push_back(var);  // coordinate or standard variable
            }
        }
        
        evalParser.registerVariables(mappedUsedVars);
        
        // Get compiled parser executor with correct number of variables
        const int numVars = static_cast<int>(usedVariables.size());
        std::cout << "DEBUG ExpressionParser::EvaluateExpression: numVars = " << numVars << std::endl;
        std::cout << "DEBUG ExpressionParser::EvaluateExpression: inputData.nComp() = " << inputData.nComp() << std::endl;
        std::cout << "DEBUG ExpressionParser::EvaluateExpression: inputVarNames.size() = " << inputVarNames.size() << std::endl;
        
        // Evaluate expression for each grid
        for (MFIter mfi(result); mfi.isValid(); ++mfi) {
            const Box& box = mfi.validbox();
            const auto& resultFab = result.array(mfi);
            
            // Get input arrays
            Vector<Array4<Real const>> inputArrays(usedVariables.size());
            for (int i = 0; i < usedVariables.size(); ++i) {
                inputArrays[i] = inputData.const_array(mfi, varIndices[i]);
            }
            
            // Use template dispatch based on number of variables
            if (numVars == 0) {
                auto executor = evalParser.compile<0>();
                amrex::ParallelFor(box, [=] AMREX_GPU_DEVICE (int i, int j, int k) {
                    resultFab(i, j, k, destComp) = executor();
                });
            } else if (numVars == 1) {
                auto executor = evalParser.compile<1>();
                amrex::ParallelFor(box, [=] AMREX_GPU_DEVICE (int i, int j, int k) {
                    resultFab(i, j, k, destComp) = executor(inputArrays[0](i, j, k));
                });
            } else if (numVars == 2) {
                auto executor = evalParser.compile<2>();
                amrex::ParallelFor(box, [=] AMREX_GPU_DEVICE (int i, int j, int k) {
                    resultFab(i, j, k, destComp) = executor(inputArrays[0](i, j, k), 
                                                           inputArrays[1](i, j, k));
                });
            } else if (numVars == 3) {
                auto executor = evalParser.compile<3>();
                amrex::ParallelFor(box, [=] AMREX_GPU_DEVICE (int i, int j, int k) {
                    resultFab(i, j, k, destComp) = executor(inputArrays[0](i, j, k), 
                                                           inputArrays[1](i, j, k),
                                                           inputArrays[2](i, j, k));
                });
            } else if (numVars == 4) {
                auto executor = evalParser.compile<4>();
                amrex::ParallelFor(box, [=] AMREX_GPU_DEVICE (int i, int j, int k) {
                    resultFab(i, j, k, destComp) = executor(inputArrays[0](i, j, k), 
                                                           inputArrays[1](i, j, k),
                                                           inputArrays[2](i, j, k),
                                                           inputArrays[3](i, j, k));
                });
            } else {
                lastError = "Too many variables in expression (max 4 supported)";
                return false;
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        lastError = string("Evaluation error: ") + e.what();
        return false;
    }
}

Real ExpressionParser::EvaluateAt(const Vector<Real>& varValues) const
{
    if (!isValid) {
        return 0.0;
    }
    
    try {
        // Create a new parser for evaluation with mapped expression and mapped variable names
        Parser evalParser(mappedExpression);
        
        // Create mapped variable names for registration
        Vector<string> mappedUsedVars;
        for (const auto& var : usedVariables) {
            auto it = originalToMapped.find(var);
            if (it != originalToMapped.end()) {
                mappedUsedVars.push_back(it->second);
            } else {
                mappedUsedVars.push_back(var);  // coordinate or standard variable
            }
        }
        
        evalParser.registerVariables(mappedUsedVars);
        
        const int numVars = static_cast<int>(varValues.size());
        
        if (numVars == 0) {
            auto executor = evalParser.compile<0>();
            return executor();
        } else if (numVars == 1) {
            auto executor = evalParser.compile<1>();
            return executor(varValues[0]);
        } else if (numVars == 2) {
            auto executor = evalParser.compile<2>();
            return executor(varValues[0], varValues[1]);
        } else if (numVars == 3) {
            auto executor = evalParser.compile<3>();
            return executor(varValues[0], varValues[1], varValues[2]);
        } else if (numVars == 4) {
            auto executor = evalParser.compile<4>();
            return executor(varValues[0], varValues[1], varValues[2], varValues[3]);
        } else {
            lastError = "Too many variables for point evaluation (max 4 supported)";
            return 0.0;
        }
        
    } catch (const std::exception& e) {
        lastError = string("Point evaluation error: ") + e.what();
        return 0.0;
    }
}

bool ExpressionParser::ValidateAndParse(const string& expression)
{
    try {
        parser = Parser(expression);
        
        // Register available variables if we have them
        if (!availableVariables.empty()) {
            parser.registerVariables(availableVariables);
        }
        
        return true;
    } catch (const std::exception& e) {
        lastError = string("Parse error: ") + e.what();
        return false;
    }
}

void ExpressionParser::ExtractUsedVariables()
{
    usedVariables.clear();
    
    try {
        // Use AMReX Parser's symbol extraction on the mapped expression
        std::set<std::string> symbols = parser.symbols();
        
        // Convert mapped variable names back to original names
        for (const auto& symbol : symbols) {
            auto it = mappedToOriginal.find(symbol);
            if (it != mappedToOriginal.end()) {
                // This is a mapped variable, use the original name
                usedVariables.push_back(it->second);
            } else {
                // This is a regular variable (coordinate or standard identifier)
                usedVariables.push_back(symbol);
            }
        }
        
    } catch (const std::exception& e) {
        lastError = string("Variable extraction error: ") + e.what();
    }
}

bool ExpressionParser::CheckVariableAvailability() const
{
    for (const auto& var : usedVariables) {
        if (std::find(availableVariables.begin(), availableVariables.end(), var) == availableVariables.end()) {
            lastError = "Variable '" + var + "' is not available";
            return false;
        }
    }
    return true;
}

bool ExpressionParser::IsValidIdentifier(const string& name) const
{
    if (name.empty()) return false;
    
    // Must start with letter or underscore
    if (!std::isalpha(name[0]) && name[0] != '_') {
        return false;
    }
    
    // Rest must be alphanumeric or underscore
    for (size_t i = 1; i < name.length(); ++i) {
        if (!std::isalnum(name[i]) && name[i] != '_') {
            return false;
        }
    }
    
    return true;
}

void ExpressionParser::CreateVariableMapping()
{
    originalToMapped.clear();
    mappedToOriginal.clear();
    
    int varCounter = 0;
    for (const auto& var : availableVariables) {
        if (!IsValidIdentifier(var)) {
            string mappedName = "var_" + std::to_string(varCounter++);
            originalToMapped[var] = mappedName;
            mappedToOriginal[mappedName] = var;
        }
    }
}

string ExpressionParser::CreateMappedExpression(const string& expression) const
{
    string result = expression;
    
    // Replace original variable names with mapped names
    // Process longer names first to avoid partial replacements
    vector<std::pair<string, string>> sortedMappings(originalToMapped.begin(), originalToMapped.end());
    std::sort(sortedMappings.begin(), sortedMappings.end(), 
              [](const std::pair<string, string>& a, const std::pair<string, string>& b) {
                  return a.first.length() > b.first.length();
              });
    
    for (const auto& mapping : sortedMappings) {
        const string& original = mapping.first;
        const string& mapped = mapping.second;
        
        size_t pos = 0;
        while ((pos = result.find(original, pos)) != std::string::npos) {
            // Verify it's a complete variable name, not part of another word
            bool isStartOk = (pos == 0) || (!std::isalnum(result[pos-1]) && result[pos-1] != '_');
            bool isEndOk = (pos + original.length() == result.length()) || 
                          (!std::isalnum(result[pos + original.length()]) && result[pos + original.length()] != '_');
            
            if (isStartOk && isEndOk) {
                result.replace(pos, original.length(), mapped);
                pos += mapped.length();
            } else {
                pos++;
            }
        }
    }
    
    return result;
}

// ===================================
// UserDerivedField Implementation
// ===================================

UserDerivedField::UserDerivedField(const string& name, const string& expression)
    : fieldName(name), fieldExpression(expression)
{
    expressionParser.SetExpression(expression, name);
}

UserDerivedField::~UserDerivedField()
{
}

bool UserDerivedField::Evaluate(const MultiFab& inputData,
                               const Vector<string>& inputVarNames,
                               MultiFab& result) const
{
    return expressionParser.EvaluateExpression(inputData, inputVarNames, result);
}

// ===================================
// ExpressionManager Implementation
// ===================================

ExpressionManager::ExpressionManager()
    : dataServicesPtr(nullptr)
{
}

ExpressionManager::~ExpressionManager()
{
    ClearAllExpressions();
}

bool ExpressionManager::AddExpression(const string& name, const string& expression)
{
    // Remove existing expression with same name
    RemoveExpression(name);
    
    // Create new expression
    UserDerivedField* newField = new UserDerivedField(name, expression);
    newField->GetParser().SetAvailableVariables(availableVariables);
    
    if (!newField->IsValid()) {
        delete newField;
        return false;
    }
    
    expressions[name] = newField;
    UpdateExpressionNames();
    
    return true;
}

bool ExpressionManager::RemoveExpression(const string& name)
{
    auto it = expressions.find(name);
    if (it != expressions.end()) {
        delete it->second;
        expressions.erase(it);
        UpdateExpressionNames();
        return true;
    }
    return false;
}

void ExpressionManager::ClearAllExpressions()
{
    for (auto& pair : expressions) {
        delete pair.second;
    }
    expressions.clear();
    expressionNames.clear();
}

void ExpressionManager::SetDataServices(DataServices* dataServices)
{
    dataServicesPtr = dataServices;
    UpdateAvailableVariables();
}

void ExpressionManager::UpdateAvailableVariables()
{
    availableVariables.clear();
    
    if (dataServicesPtr != nullptr) {
        // Get variable names from DataServices
        const Vector<string>& varNames = dataServicesPtr->PlotVarNames();
        for (const auto& name : varNames) {
            availableVariables.push_back(name);
        }
        
        // Note: Derived variables are already included in PlotVarNames()
    }
    
    UpdateVariableContext();
}

const Vector<string>& ExpressionManager::GetExpressionNames() const
{
    return expressionNames;
}

const UserDerivedField* ExpressionManager::GetExpression(const string& name) const
{
    auto it = expressions.find(name);
    return (it != expressions.end()) ? it->second : nullptr;
}

UserDerivedField* ExpressionManager::GetExpression(const string& name)
{
    auto it = expressions.find(name);
    return (it != expressions.end()) ? it->second : nullptr;
}

bool ExpressionManager::EvaluateExpression(const string& name,
                                          const MultiFab& inputData,
                                          MultiFab& result) const
{
    const UserDerivedField* field = GetExpression(name);
    if (field == nullptr) {
        return false;
    }
    
    // Get the variables used by this specific expression
    const Vector<string>& usedVars = field->GetParser().GetUsedVariables();
    
    std::cout << "DEBUG ExpressionManager::EvaluateExpression: usedVars.size() = " << usedVars.size() << std::endl;
    std::cout << "DEBUG ExpressionManager::EvaluateExpression: inputData.nComp() = " << inputData.nComp() << std::endl;
    
    return field->Evaluate(inputData, usedVars, result);
}

bool ExpressionManager::ValidateExpression(const string& expression) const
{
    ExpressionParser testParser;
    testParser.SetAvailableVariables(availableVariables);
    return testParser.IsValidExpression(expression);
}

const Vector<string>& ExpressionManager::GetAvailableVariables() const
{
    return availableVariables;
}

void ExpressionManager::UpdateExpressionNames()
{
    expressionNames.clear();
    for (const auto& pair : expressions) {
        expressionNames.push_back(pair.first);
    }
}

void ExpressionManager::UpdateVariableContext()
{
    // Update all expressions with new variable context
    for (auto& pair : expressions) {
        pair.second->GetParser().SetAvailableVariables(availableVariables);
    }
}