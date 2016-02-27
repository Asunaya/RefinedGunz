#ifndef __NETBASE_H

#define __NETBASE_H





#include "NetManager.h"





NetManager* HBGetNetManager();





void	InitNetBase(const char *ConfigFileName);

void	DestroyNetBase();



int		GetGroupCount();

char*	GetGroupName(int index);



int		GetFriendsCount(int nGroup);

char*	GetFriendsName(int nGroup,int index);



void	Refresh();



void	AddGroup(const char *GroupName);

void	DeleteGroup(int index);



void	AddFriend(const char *FriendName);

void	DeleteFriend(int index);



#endif