#ifndef _INC_FRIENDSYSTEM_
#define _INC_FRIENDSYSTEM_

#include <list>
#include <string>

struct sqlite3;

typedef std::list<std::string> FriendList;

class FriendSystem
{
public:
    FriendSystem();
    ~FriendSystem();

public:
    bool Attach(sqlite3 *pDB);

    bool GetFriendList(const char *szName, FriendList &flist);
    bool AddRelation(const char *szNameL, const char *szNameR);
    bool DelRelation(const char *szNameL, const char *szNameR);

private:
    sqlite3 *m_pDB;
};

#endif
