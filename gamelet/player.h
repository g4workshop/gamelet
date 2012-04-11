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

class bufferevent;
class evbuffer;

class Group;
// Player connection data
struct Player{
    Player();
    bool isNPC();
    bool attributeMatch(Player *other);
   	// The bufferedevent for this player.
	struct bufferevent *bev;
	// The recieved command buffer.
	struct evbuffer *commandBuffer;
    
    // is the player belong a game group
    Group *group;
    
    // player attributes
    std::string userid;
    std::string gameid;
    bool NPC;
    time_t loginTime;
    std::map<std::string, std::string> attributes;
};

struct Group{
    Group();
    
    bool add(Player *player);
    bool remove(Player *player);
    bool isEmpty();
    // if it's enough player(included NPC) return true
    bool isEnoughToPlay();
    // if is enough player(not inclued NPC) return true;
    bool isEnoughPlayer();
    bool isFull();
    
    std::list<Player*> players;
    int minimum;
    int maxima;
    time_t createdTime;
};

class PlayerManager{
public:
    static PlayerManager &instance();
    Player *newPlayer(struct event_base *base, evutil_socket_t fd);
    void deletePlayer(Player *player);
    bool login(Player *player);
    bool logout(Player *player);
    
    Group *matchGroup(Player* player);
    bool leaveGroup(Player *player);
    
private:
    PlayerManager();
    ~PlayerManager();
    
private:
    std::set<Player *>players;
    std::set<Group *>groups;
};

#endif
