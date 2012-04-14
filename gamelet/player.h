//
//  Player_manager.h
//  Player manager
//
//  Created by Dawen Rie on 12-4-9.
//  Copyright (c) 2012年 G4 Workshop. All rights reserved.
//

#ifndef gamelet_player_manager_h
#define gamelet_player_manager_h

#include <event2/util.h>

#include <string>
#include <map>
#include <set>
#include <list>

#include "cmd.h"

class bufferevent;
class evbuffer;

class Group;

// Player connection data
struct Player{
    Player();
    const char* desc();
    bool isNPC();
    bool attributeMatch(Player *other);
    void handleCommand();
    void forwardToPlayer(std::string &userid, short length);
    void forwardToGroup(short length);
    
    void login(Command &cmd);
    void logout(Command &cmd);
    void match(Command &cmd);
    void leaveMatch(Command &cmd);
    
   	// The bufferedevent for this player.
	struct bufferevent *bev;
	// The recieved command buffer.
	struct evbuffer *commandBuffer;
    
    // is the player belong a game group
    Group *group;
    
    // player attributes
    std::string userid;
    std::string passwd;
    std::string gameid;
    bool NPC;
    time_t loginTime;
    std::map<std::string, std::string> attributes;
};

struct Group{
    Group();
    const char *desc();
    bool add(Player *player);
    bool remove(Player *player);
    bool isEmpty();
    // if it's enough player(included NPC) return true
    bool isEnoughToPlay();
    // if is enough player(not inclued NPC) return true;
    bool isEnoughPlayer();
    bool isFull();
    
    void notify(unsigned short cmd);
    void notifyPlayerMatched();
    void notifyGameStart();
    void notifyGameStop();
    
    std::list<Player*> players;
    unsigned int minimum;
    unsigned int maxima;
    time_t createdTime;
};

class PlayerManager{
public:
    static PlayerManager &instance();
    Player *newPlayer(struct event_base *base, evutil_socket_t fd);
    void deletePlayer(Player *player);
    Group *newGroup(int min, int max);
    void deleteGroup(Group *group);
    
    bool login(Player *player, std::string &userid, std::string &passwd);
    bool logout(Player *player);
    
    Group *matchGroup(Player* player, unsigned int min, unsigned int max);
    bool leaveGroup(Player *player);
    
private:
    PlayerManager();
    ~PlayerManager();
    
private:
    std::set<Player *> players;
    std::set<Group *> groups;
};

#endif
