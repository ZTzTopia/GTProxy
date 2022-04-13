//  ***************************************************************
//  SharedDB - Creation date: 04/11/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#pragma once
#include <functional>
#include <unordered_map>
#include <vector>

#include "../util/Variant.h"

#define C_VARIANT_DB_FILE_VERSION 1

class FunctionObject {
public:
    std::function<void(VariantList*)> sig_function;
};

typedef std::unordered_map<std::string, Variant*> dataList;
typedef std::unordered_map<std::string, FunctionObject*> functionList;

class VariantDB {
public:
    VariantDB();
    virtual ~VariantDB();

    VariantDB(const VariantDB &ref) {
        // assert(!"Warning, slow operation!");
        // Our special copy won't include the sig crap, that stuff can't be copied
        *this = ref;
    }

    FunctionObject *GetFunction(const std::string &keyName); // Created it needed, this is usually what you want
    FunctionObject *GetFunctionIfExists(const std::string &keyName); // Returns null if not created yet
    void CallFunctionIfExists(const std::string &keyName, VariantList *pVList);

    Variant * GetVar(const std::string &keyName);  // Created it needed, this is usually what you want
    Variant * GetVarIfExists(const std::string &keyName);  // Returns null if not created yet
    Variant * GetVarWithDefault(const std::string &keyName, const Variant &vDefault);
    int DeleteVarsStartingWith(const std::string &deleteStr); // Returns how many were deleted
    int DeleteVar(const std::string& keyName); // Returns how many were deleted (0 or 1..)

    // You can load and save the variables in the DB on the fly.  (Does nothing with the functions)
    bool Save(const std::string &fileName, bool bAddBasePath = true);
    bool Load(const std::string &fileName, bool *pFileExistedOut = nullptr, bool bAddBasePath = true);

    std::string DumpAsString();
    void Print(); // Same as above, but sends to LogMsg()
    void DeleteAll();
    int GetVarCount() { return (int) m_data.size(); }

    void Clear();
    // To get each var in our db manually, do this:
    void ResetNext(); // Call before starting a search
    Variant* GetNext(std::string &keyOut); // Call this in a loop until it returns NULL to signal the finish, callResetNext() before using!
    int AddVarPointersToVector(std::vector<std::pair<const std::string *, Variant *>> *varListOut, std::string keyMustStartWithThisText = "");


    VariantDB &operator=(const VariantDB &rhs) {
        // m_functionData = rhs.m_functionData;
        auto itor = rhs.m_data.begin();
        while (itor != rhs.m_data.end()) {
            m_data[itor->first] = new Variant(*itor->second);
            itor++;
        }
        return *this;
    }

private:
    dataList m_data;
    functionList m_functionData;
    dataList::iterator m_nextItor;
};
