#include "FriendSystem.h"

#include <assert.h>
#include <sqlite3.h>
#include <string.h>

FriendSystem::FriendSystem() { m_pDB = nullptr; }

FriendSystem::~FriendSystem() {
    // Nothing
}

bool FriendSystem::Attach(sqlite3 *pDB) {
    assert(nullptr == m_pDB);
    m_pDB = pDB;
    char *pErrMsg = nullptr;

    // Create tables
    const char *pszStmt =
        "CREATE TABLE IF NOT EXISTS friend_relation (id INTEGER PRIMARY KEY "
        "AUTOINCREMENT, name0 TEXT, name1 TEXT)";
    int nRet = sqlite3_exec(m_pDB, pszStmt, nullptr, nullptr, &pErrMsg);
    if (nRet != SQLITE_OK) {
        sqlite3_free(pErrMsg);
        pErrMsg = nullptr;
        return false;
    }
    // Create indexes
    pszStmt = "CREATE UNIQUE INDEX unique_name0_name1 ON friend_relation "
              "(name0, name1)";
    nRet = sqlite3_exec(m_pDB, pszStmt, nullptr, nullptr, &pErrMsg);
    if (nRet != SQLITE_OK) {
        if (nullptr == pErrMsg ||
            0 != strcmp(pErrMsg, "index unique_name0_name1 already exists")) {
            sqlite3_free(pErrMsg);
            pErrMsg = nullptr;
            return false;
        }
        sqlite3_free(pErrMsg);
        pErrMsg = nullptr;
    }

    return true;
}

static void sortName(const char *&szNameL, const char *&szNameR) {
    int nCmpRet = strcmp(szNameL, szNameR);
    if (0 == nCmpRet) {
        return;
    }
    if (nCmpRet > 0) {
        const char *tmp = szNameL;
        szNameL = szNameR;
        szNameR = tmp;
    }
}

bool FriendSystem::AddRelation(const char *szNameL, const char *szNameR) {
    if (0 == strcmp(szNameL, szNameR)) {
        return false;
    }
    sortName(szNameL, szNameR);

    char szStmt[256];
    snprintf(szStmt, sizeof(szStmt),
             "INSERT INTO friend_relation (name0, name1) VALUES ('%s', '%s')",
             szNameL, szNameR);
    char *pErrMsg = nullptr;
    int nRet = sqlite3_exec(m_pDB, szStmt, nullptr, nullptr, &pErrMsg);
    if (nRet != SQLITE_OK) {
        sqlite3_free(pErrMsg);
        pErrMsg = nullptr;
        return false;
    }
    return true;
}

struct GetFriendListCallArg {
    FriendList *pList;
    const char *pName;
};

bool FriendSystem::GetFriendList(const char *szName, FriendList &flist) {
    flist.clear();
    char szStmt[256];
    snprintf(szStmt, sizeof(szStmt),
             "SELECT name0, name1 FROM friend_relation WHERE name0 = '%s' OR "
             "name1 = '%s'",
             szName, szName);
    char *pErrMsg = nullptr;

    GetFriendListCallArg ca;
    ca.pName = szName;
    ca.pList = &flist;

    int nRet = sqlite3_exec(
        m_pDB, szStmt,
        [](void *pParam, int nColumnCount, char **pColumnValue,
           char **pColumnName) -> int {
            if (nColumnCount != 2) {
                return 1;
            }
            GetFriendListCallArg *pCa = (GetFriendListCallArg *)pParam;
            const char *pTarget = nullptr;
            if (0 == strcmp(pColumnValue[0], pCa->pName)) {
                pCa->pList->push_back(std::string(pColumnValue[1]));
            } else {
                pCa->pList->push_back(std::string(pColumnValue[0]));
            }
            return 0;
        },
        &ca, &pErrMsg);

    if (nRet != SQLITE_OK) {
        sqlite3_free(pErrMsg);
        pErrMsg = nullptr;
        return false;
    }
    return true;
}

bool FriendSystem::DelRelation(const char *szNameL, const char *szNameR) {
    if (0 == strcmp(szNameL, szNameR)) {
        return false;
    }
    sortName(szNameL, szNameR);

    char szStmt[256];
    snprintf(szStmt, sizeof(szStmt),
             "DELETE FROM friend_relation WHERE name0 = '%s' AND name1='%s'",
             szNameL, szNameR);
    char *pErrMsg = nullptr;
    int nRet = sqlite3_exec(m_pDB, szStmt, nullptr, nullptr, &pErrMsg);
    if (nRet != SQLITE_OK) {
        sqlite3_free(pErrMsg);
        pErrMsg = nullptr;
        return false;
    }
    // Get rows affected
    int nAffectedRows = sqlite3_changes(m_pDB);
    return (nAffectedRows == 1);
}
