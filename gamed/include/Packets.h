/*
IntWars playground server for League of Legends protocol testing
Copyright (C) 2012  Intline9 <Intline9@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _PACKETS_H
#define _PACKETS_H

#include <time.h>
#include <cmath>
#include <set>

#include <intlib/general.h>
#include <enet/enet.h>

#include "common.h"
#include "Buffer.h"
#include "Client.h"
#include "Minion.h"
#include "Turret.h"
#include "LevelProp.h"

#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

/* New Packet Architecture */
class Packet {
protected:
   Buffer buffer;
   
public:
   const Buffer& getBuffer() const { return buffer; }
   Packet(uint8 cmd = 0) {
      buffer << cmd;
   }

};

class BasePacket : public Packet {
   
public:
   BasePacket(uint8 cmd = 0, uint32 netId = 0) : Packet(cmd) {
      buffer << netId;
   }

};

class GamePacket : public BasePacket {
   
public:
   GamePacket(uint8 cmd = 0, uint32 netId = 0) : BasePacket(cmd, netId) {
      buffer << (uint32)clock();
   }

};

/* Old Packet Architecture & Packets */

struct PacketHeader {
    PacketHeader() {
        netId = 0;
    }

    PacketCmd cmd;
    uint32 netId;
};

struct GameHeader {
    GameHeader() {
        netId = ticks = 0;
    }

    GameCmd cmd;
    uint32 netId;
    uint32 ticks;
};

/*typedef struct _SynchBlock {
    uint64 userId;
    uint16 unk;
    uint32 skill1;
    uint32 skill2;
    uint8 bot;
    uint32 teamId;
    uint8 name[64];
    uint8 type[64];
    uint8 rank[30];
} SynchBlock;*/

struct ClientReady {
    uint32 cmd;
    uint32 playerId;
    uint32 teamId;
};

class SynchVersionAns : public BasePacket {
public:

   SynchVersionAns(const std::vector<ClientInfo*>& players, const std::string& version, const std::string& gameMode) : BasePacket(PKT_S2C_SynchVersion) {
      buffer << (uint8)9; // unk
      buffer << (uint32)1; // mapId
      for(auto p : players) {
         buffer << p->userId;
         buffer << (uint16)0x1E; // unk
         buffer << p->summonerSkills[0];
         buffer << p->summonerSkills[1];
         buffer << (uint8)0; // bot boolean
         buffer << p->getTeam();
         buffer << p->getName();
         buffer.fill(0, 64-p->getName().length());
         buffer.fill(0, 64);
         buffer << p->getRank();
         buffer.fill(0, 28-p->getRank().length());
         buffer << (uint16)p->getRibbon();
      }
      
      for(int i = 0; i < 12-players.size(); ++i) {
         buffer << (int64)-1;
         buffer.fill(0, 173);
      }
            
        buffer << version;
        buffer.fill(0, 256-version.length());
        buffer << gameMode;
        buffer.fill(0, 128-gameMode.length());
        
        buffer.fill(0, 2574);
    }

   /* PacketHeader header;
    uint8 ok;
    uint32 mapId;
    SynchBlock players[12];
    uint8 version[256]; 
    uint8 gameMode[128];
    uint8 unk1[512];
    uint8 unk2[245];

    uint8 ekg1[256]; //ekg.riotgames.net
    uint8 msg1[256]; //"/messages"

    uint16 wUnk1; //0x50?
    uint8 ekg2[256]; //ekg.riotgames.net
    uint8 msg2[256]; //"/messages"

    uint16 wUnk2; //0x50?
    uint8 ekg3[256]; //ekg.riotgames.net
    uint8 msg3[256]; //"/messages"

    uint16 wUnk3; //0x50?
    uint32 dwUnk1;
    uint32 dwOpt; //0x377192
    uint8 bUnk1[0x100];
    uint8 bUnk2[11];*/
};

typedef struct _PingLoadInfo {
    PacketHeader header;
    uint32 unk1;
    uint64 userId;
    float loaded;
    float ping;
    float f3;
} PingLoadInfo;

uint8 *createDynamicPacket(uint8 *str, uint32 size);

class LoadScreenInfo : public Packet {
public:
    LoadScreenInfo(const std::vector<ClientInfo*>& players) : Packet(PKT_S2C_LoadScreenInfo) {
        //Zero this complete buffer
        buffer << (uint32)6; // blueMax
        buffer << (uint32)6; // redMax
        
        uint32 currentBlue = 0;
        for(ClientInfo* player : players) {
           if(player->getTeam() == TEAM_BLUE) {
              buffer << player->userId;
              currentBlue++;
           }
        }
        
        for(int i = 0; i < 6-currentBlue; ++i) {
           buffer << (uint64)0;
        }
        
        buffer.fill(0, 144);
        
        uint32 currentPurple = 0;
        for(ClientInfo* player : players) {
           if(player->getTeam() == TEAM_PURPLE) {
              buffer << player->userId;
              currentPurple++;
           }
        }
        
        for(int i = 0; i < 6-currentPurple; ++i) {
           buffer << (uint64)0;
        }
        
        buffer.fill(0, 144);
        buffer << currentBlue;
        buffer << currentPurple;
    }

    /*uint8 cmd;
    uint32 blueMax;
    uint32 redMax;
    uint64 bluePlayerIds[6]; //Team 1, 6 players max
    uint8 blueData[144];
    uint64 redPlayersIds[6]; //Team 2, 6 players max
    uint8 redData[144];
    uint32 bluePlayerNo;
    uint32 redPlayerNo;*/
};

typedef struct _KeyCheck {
    _KeyCheck() {
        cmd = PKT_KeyCheck;
        playerNo = 0;
        checkId = 0;
        trash = trash2 = 0;
    }

    uint8 cmd;
    uint8 partialKey[3];   //Bytes 1 to 3 from the blowfish key for that client
    uint32 playerNo;
    uint64 userId;         //uint8 testVar[8];   //User id
    uint32 trash;
    uint64 checkId;        //uint8 checkVar[8];  //Encrypted testVar
    uint32 trash2;
} KeyCheck;

struct CameraLock {
    PacketHeader header;
    uint8 isLock;
    uint32 padding;
};

/*typedef struct _ViewReq {
    uint8 cmd;
    uint32 unk1;
    float x;
    float zoom;
    float y;
    float y2;		//Unk
    uint32 width;	//Unk
    uint32 height;	//Unk
    uint32 unk2;	//Unk
    uint8 requestNo;
} ViewReq;*/

/**
 * Change Target ??
 */
struct Unk {
   Unk(uint32 netId, float x, float y, uint32 targetNetId = 0) : unk1(0x0F), unk2(1), unk3(2), x(x), unk4(55), y(y), targetNetId(targetNetId) {
      header.cmd = PKT_S2C_UNK;
      header.netId = netId;
   }

   PacketHeader header;
   uint8 unk1, unk2, unk3;

   float x, unk4, y;
   uint32 targetNetId;
};

struct MinionSpawn {
   
   MinionSpawn(const Minion* m) : netId(m->getNetId()), netId2(m->getNetId()), netId3(m->getNetId()), unk(0x00150017), unk2(0x03), position(m->getPosition()), unk4(0xff), unk5_1(1), type(m->getType()), unk5_3(0), unk5_4(1), unk7(5), unk8(0x0ff84540f546f424) {
      header.cmd = PKT_S2C_MinionSpawn;
      header.netId = m->getNetId();
      memcpy(unk6, "\x0a\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x3f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x2c\x27\x00\x00\x06", 36);
   }

   PacketHeader header;
   
   uint32 unk;
   uint8 unk2;
   uint32 netId, netId2;
   uint32 position;
   uint8 unk4;
   uint8 unk5_1;
   uint8 type;
   uint8 unk5_3;
   uint8 unk5_4;
   uint8 unk6[36];
   uint32 netId3;
   uint8 unk7;
   uint64 unk8;
};

struct MovementReq {
    PacketHeader header;
    MoveType type;
    float x;
    float y;
    uint32 targetNetId;
    uint8 vectorNo;
    uint32 netId;
    uint8 moveData;
};

struct MovementAns {
    MovementAns() {
        header.cmd = PKT_S2C_MoveAns;
    }

    GameHeader header;
    uint16 nbUpdates;
    uint8 vectorNo;
    uint32 netId;
    uint8 moveData; //bitMasks + Move Vectors

    MovementVector *getVector(uint32 index) {
        if(index >= (uint8)vectorNo / 2)
        { return NULL; }
        MovementVector *vPoints = (MovementVector *)(&moveData + maskCount());
        return &vPoints[index];
    }

    int maskCount() {
        float fVal = (float)vectorNo / 2;
        return (int)std::ceil((fVal - 1) / 4);
    }

    static uint32 size(uint8 vectorNo) {
        float fVectors = vectorNo;
        int maskCount = (int)std::ceil((fVectors - 1) / 4);
        return sizeof(MovementAns) + (vectorNo * sizeof(MovementVector)) + maskCount; //-1 since struct already has first moveData byte
    }

    uint32 size() {
        return size(vectorNo / 2);
    }

    static MovementAns *create(uint32 vectorNo) {
        int nSize = size(vectorNo / 2);
        MovementAns *packet = (MovementAns *)new uint8[nSize];
        memset(packet, 0, nSize);
        packet->header.cmd = PKT_S2C_MoveAns;
        packet->header.ticks = clock();
        packet->vectorNo = vectorNo;
        return packet;
    }

    static void destroy(MovementAns *packet) {
        delete [](uint8 *)packet;
    }

};

/*typedef struct _ViewAns {
    _ViewAns() {
        cmd = PKT_S2C_ViewAns;
        unk1 = 0;
    }

    uint8 cmd;
    uint32 unk1;
    uint8 requestNo;
} ViewAns;*/


typedef struct _QueryStatus {
    _QueryStatus() {
        header.cmd = PKT_S2C_QueryStatusAns;
        ok = 1;
    }
    PacketHeader header;
    uint8 ok;
} QueryStatus;

typedef struct _SynchVersion {
    PacketHeader header;
    uint32 unk1;
    uint32 unk2;
    uint8 version[50]; //Dunno how big and when usefull data begins
} SynchVersion;

typedef struct _WorldSendGameNumber {
    _WorldSendGameNumber() {
        header.cmd = PKT_World_SendGameNumber;
        memset(data, 0, sizeof(data1));
        memset(data, 0, sizeof(data));
        gameId = 0;
    }

    PacketHeader header;
    uint64 gameId; //_0x0000
    uint8 server[5]; //0x0008
    uint8 data1[27]; //0x000D
    uint8 data[0x80];//0x0028
} WorldSendGameNumber;


struct CharacterStats {

   CharacterStats(uint8 masterMask, uint32 netId, uint32 mask, float value) : updateNo(1), masterMask(masterMask), netId(netId), mask(mask), size(4) {
      header.cmd = (GameCmd)PKT_S2C_CharStats;
      header.ticks = clock();
      this->value.fValue = value;
   }
   
   CharacterStats(uint8 masterMask, uint32 netId, uint32 mask, unsigned short value) : updateNo(1), masterMask(masterMask), netId(netId), mask(mask), size(2) {
      header.cmd = (GameCmd)PKT_S2C_CharStats;
      header.ticks = clock();
      this->value.sValue = value;
   }

   GameHeader header;
   uint8 updateNo;
   uint8 masterMask;
   uint32 netId;
   uint32 mask;
   uint8 size;
   union {
      unsigned short sValue;
      float fValue;
   } value;
};

struct ChatMessage {
    uint8 cmd;
    uint32 netId;
    uint32 unk1;
    uint8 unk2;

    uint32 playerNo;
    ChatType type;
    uint32 length;
    uint8 unk3[32];
    int8 msg;

    int8 *getMessage() {
        return &msg;
    }
    uint32 *getLength() {
       return &length;
    }
};

typedef struct _UpdateModel {
    _UpdateModel(uint32 netID, const char *szModel) {
        memset(this, 0, sizeof(_UpdateModel));
        header.cmd = (PacketCmd)0x97;
        header.netId = netID;
        id = netID & ~0x40000000;
        bOk = 1;
        unk1 = -1;
        strncpy((char *)szName, szModel, 32);
    }
    PacketHeader header;
    uint32 id;
    uint8 bOk;
    uint32 unk1;
    uint8 szName[32];
} UpdateModel;
typedef struct _StatePacket {
    _StatePacket(PacketCmd state) {
        header.cmd = state;
    }
    PacketHeader header;
} StatePacket;
typedef struct _StatePacket2 {
    _StatePacket2(PacketCmd state) {
        header.cmd = state;
        nUnk = 0;
    }
    PacketHeader header;
    int16 nUnk;
} StatePacket2;

struct FogUpdate2 {
    FogUpdate2() {
        header.cmd = PKT_S2C_FogUpdate2;
        header.netId = 0x40000019;
    }
    PacketHeader header;
    float x;
    float y;
    uint32 radius;
    uint8 unk1;
};

struct Click {

   PacketHeader header;
   uint32 zero;
   uint32 targetNetId; // netId on which the player clicked

};

struct Unk2 {
   Unk2(uint32 sourceNetId, uint32 targetNetId)  : targetNetId(targetNetId) {
      header.cmd = PKT_S2C_Unk2;
      header.netId = sourceNetId;
   }

   PacketHeader header;
   uint32 targetNetId;
};

class HeroSpawn : public Packet {
public:
	HeroSpawn(ClientInfo* player, int playerId) : Packet(PKT_S2C_HeroSpawn) {
		buffer << (uint32)0; // ???
		buffer << (uint32)player->getChampion()->getNetId();
		buffer << (uint32)playerId; // +1 for each player ?
		buffer << (uint8)0; // netNodeID ?
		buffer << (uint8)1; // SkillLevel
      if(player->getTeam() == TEAM_BLUE) {
         buffer << (uint8)1; // teamIsOrder Blue=Order=1 Purple=Choas=0
      } else {
         buffer << (uint8)0; // teamIsOrder Blue=Order=1 Purple=Choas=0
      }
		buffer << (uint8)0; // isBot
		buffer << (uint8)0; // botRank
		buffer << (uint8)0; // spawnPosIndex ?
		buffer << (uint32)player->getSkinNo();
		buffer << player->getName();
		buffer.fill(0, 128-player->getName().length());
		buffer << player->getChampion()->getType();
		buffer.fill(0, 40-player->getChampion()->getType().length());
		buffer << (float)0.f; // deathDurationRemaining
		buffer << (float)0.f; // timeSinceDeath
		buffer << (uint8)0; // bitField
	}
};

struct HeroSpawn2 : public BasePacket {
    HeroSpawn2(Champion* p) : BasePacket(PKT_S2C_HeroSpawn2, p->getNetId()) {
        buffer.fill(0, 15);
        buffer << (uint8)0x80; // unk
        buffer << (uint8)0x3F; // unk
        buffer.fill(0, 13);
        buffer << (uint8)3; // unk
        buffer << (uint32)1; // unk
        buffer << p->getX();
        buffer << p->getY();
        buffer << (float)0x3F441B7D; // z ?
        buffer << (float)0x3F248DBB; // Rotation ?
    }
};

class TurretSpawn : public BasePacket {
public:
   TurretSpawn(Turret* t) : BasePacket(PKT_S2C_TurretSpawn) {
      buffer << t->getNetId();
      buffer << t->getName();
      buffer.fill(0, 28-t->getName().length()+42);
   }

   /*PacketHeader header;
   uint32 tID;
   uint8 name[28];
   uint8 type[42];*/
};

struct GameTimer {
    GameTimer(float fTime) {
        header.cmd = PKT_S2C_GameTimer;
        header.netId = 0;
        this->fTime = fTime;
    }
    PacketHeader header;
    float fTime;
};
struct GameTimerUpdate {
    GameTimerUpdate(float fTime) {
        header.cmd = PKT_S2C_GameTimerUpdate;
        header.netId = 0;
        this->fTime = fTime;
    }
    PacketHeader header;
    float fTime;
};
struct SpellSet {
    SpellSet(uint32 netID, uint32 _spellID, uint32 _level) {
        memset(this, 0, sizeof(SpellSet));
        header.cmd = PacketCmd(0x5A);
        header.netId = netID;
        spellID = _spellID;
        level = _level;
    }
    PacketHeader header;
    uint32 spellID;
    uint32 level;
};

struct Announce {
    PacketHeader header;
    uint8 msg;
    uint64 unknown;
    uint32 mapNo;
};

typedef struct _SkillUpPacket {
    PacketHeader header;
    uint8 skill;
} SkillUpPacket;

typedef struct _BuyItemReq {
    PacketHeader header;
    uint32 id;
} BuyItemReq;

typedef struct _BuyItemAns {
    _BuyItemAns() {
        header.cmd = (PacketCmd)PKT_S2C_BuyItemAns;
        unk1 = 0;
        unk2 = 0;
        unk3 = 0;
    }
    PacketHeader header;
    uint16 itemId;
    uint16 unk1;
    uint8 slotId;
    uint8 stack;
    uint16 unk2;
    uint8 unk3;
} BuyItemAns;

typedef struct _EmotionPacket {
    PacketHeader header;
    uint8 id;
} EmotionPacket;

typedef struct _EmotionResponse {
    _EmotionResponse() {
        header.cmd = PKT_S2C_Emotion;
    }
    PacketHeader header;
    uint8 id;
} EmotionResponse;

/* New Style Packets */

class DamageDone : public BasePacket {
public:
   DamageDone(Unit* source, Unit* target, float amount) : BasePacket(PKT_S2C_DamageDone, target->getNetId()) {
      buffer << (uint8)4; // damage type ? 4 = physical ?
      buffer << target->getNetId();
      buffer << source->getNetId();
      buffer << amount;
   }
};

class LoadScreenPlayerName : public Packet {
public:
   LoadScreenPlayerName(const ClientInfo& player) : Packet(PKT_S2C_LoadName) {
      buffer << player.userId;
      buffer << (uint32)0;
      buffer << (uint32)player.getName().length()+1;
      buffer << player.getName();
      buffer << (uint8)0;
   }

    /*uint8 cmd;
    uint64 userId;
    uint32 skinId;
    uint32 length;
    uint8* description;*/
};

class LoadScreenPlayerChampion : public Packet {
public:
   LoadScreenPlayerChampion(const ClientInfo& player) : Packet(PKT_S2C_LoadHero) {
      buffer << player.userId;
      buffer << player.skinNo;
      buffer << (uint32)player.getChampion()->getType().length()+1;
      buffer << player.getChampion()->getType();
      buffer << (uint8)0;
   }

    /*uint8 cmd;
    uint64 userId;
    uint32 skinId;
    uint32 length;
    uint8* description;*/
};

struct AttentionPing {
    AttentionPing() {
    }
    AttentionPing(AttentionPing *ping) {
        cmd = ping->cmd;
        unk1 = ping->unk1;
        x = ping->x;
        y = ping->y;
        z = ping->z;
        type = ping->type;
    }

    uint8 cmd;
    uint32 unk1;
    float x;
    float y;
    float z;
    uint8 type;
};

class AttentionPingAns : public Packet {
public:
   AttentionPingAns(ClientInfo *player, AttentionPing *ping) : Packet(PKT_S2C_AttentionPing){
      buffer << (uint32)0; //unk1
      buffer << ping->x;
      buffer << ping->y;
      buffer << ping->z;
      buffer << (uint32)player->getChampion()->getNetId();
      switch (ping->type)
      {
         case 0:
            buffer << (uint8)0xb0;
            break;
         case 1:
            buffer << (uint8)0xb1;
            break;
         case 2:
            buffer << (uint8)0xb2; // Danger
            break;
         case 3:
            buffer << (uint8)0xb3; // Enemy Missing
            break;
         case 4:
            buffer << (uint8)0xb4; // On My Way
            break;
         case 5:
            buffer << (uint8)0xb5; // Retreat / Fall Back
            break;
         case 6:
            buffer << (uint8)0xb6; // Assistance Needed
            break;            
      }
   }
};

class AutoAttack : public BasePacket {
public:
   AutoAttack(Unit* attacker, Unit* attacked) : BasePacket(PKT_S2C_AutoAttack, attacker->getNetId()) {
      buffer << attacked->getNetId();
      buffer << (uint16)0xd580; // unk
      buffer << 12.f; // unk
      buffer << attacker->getX() << attacker->getY();
   }
};

class SetTarget : public BasePacket {
public:
   SetTarget(Unit* attacker, Unit* attacked) : BasePacket(PKT_S2C_SetTarget, attacker->getNetId()) {
      buffer << attacked->getNetId();
   }

};

class SetHealth : public BasePacket {
public:
   SetHealth(Unit* u) : BasePacket(PKT_S2C_SetHealth, u->getNetId()) {
      buffer << (uint16)0x0000; // unk
      buffer << u->getStats().getMaxHealth();
      buffer << u->getStats().getCurrentHealth();
   }
};

class SkillUpResponse : public BasePacket {
public:
    SkillUpResponse(uint32 netId, uint8 skill, uint8 level, uint8 pointsLeft) : BasePacket(PKT_S2C_SkillUp, netId) {
        buffer << skill << level << pointsLeft;
    }
};

class TeleportRequest : public BasePacket {
    
public:
    TeleportRequest(int netId,float x, float y, bool first) : BasePacket(PKT_S2C_MoveAns, (uint32) 0x0){
      buffer << (uint32) clock();//not 100% sure
      buffer << (uint8) 0x01;
      buffer << (uint8) 0x00;
      if(first == true){
      buffer << (uint8) 0x02;
      }else{
          buffer << (uint8) 0x03;
      }///      }//seems to be id, 02 = before teleporting, 03 = do teleport
      buffer << (uint32)netId;
      if(first == false){
          static uint8 a = 0x01;
          
          buffer << (uint8) a; // if it is the second part, send 0x01 before coords
          a++;
      }
      buffer << (uint16)x;
      buffer << (uint16)y;
    }
    
};




struct CastSpell {
    PacketHeader header;
    uint8 spellSlot; // 2 first bits seem to be unknown flags
    float x, y;
    float x2, y2;
    uint32 targetNetId; // If 0, use coordinates, else use target net id
};

class CastSpellAns : public GamePacket {
public:
   CastSpellAns(Spell* s, float x, float y) : GamePacket(PKT_S2C_CastSpellAns, s->getOwner()->getNetId()) {
      printf("%08X ; %08X ; %f\n", s->getId(), s->getOwner()->getChampionHash(), s->getCastTime());
      buffer << (uint8)0 << (uint8)0x66 << (uint8)0x00; // unk
      buffer << s->getId(); // Spell hash, for example hash("EzrealMysticShot")
      buffer << (uint32)0x400001f6; // a net ID, but what for..
      buffer << (uint8)0 << (uint8)0 << (uint8)0;
      buffer << (uint16)0x3f80; // unk
      buffer << s->getOwner()->getNetId() << s->getOwner()->getNetId();
      buffer << (uint32)s->getOwner()->getChampionHash();
      buffer << (uint32)0x400001f5; // Another net ID..
      buffer << x << 55.f << y;
      buffer << x << 55.f << y;
      buffer << (uint8)0;
      buffer << s->getCastTime();
      buffer << (float)0.f; // unk
      buffer << (float)1.0f; // unk
      buffer << s->getCooldown();
      buffer << (float)0.f; // unk
      buffer << (uint8)0; // unk
      buffer << s->getSlot(); 
      buffer << (uint16)0; // unk
      buffer << (uint16)0x41e0; // unk
      buffer << s->getOwner()->getX() << 55.f << s->getOwner()->getY();
      buffer << (uint64)1; // unk
}
};

class PlayerInfo : public BasePacket{

public:

   PlayerInfo(ClientInfo* player) : BasePacket(PKT_S2C_PlayerInfo, player->getChampion()->getNetId()){
   
   buffer << (uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x83  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xD7  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xD7  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xD7  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00;

   buffer << player->summonerSkills[0];
   buffer << player->summonerSkills[1];
 
   buffer <<(uint8) 0x41  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x42  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x04  <<(uint8) 0x52  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x03  <<(uint8) 0x61  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x62  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x64  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x03  <<(uint8) 0x71  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x72  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x03  <<(uint8) 0x82  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x03  <<(uint8) 0x92  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x41  <<(uint8) 0x75  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x42  <<(uint8) 0x75  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x02  <<(uint8) 0x43  <<(uint8) 0x75  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x02  <<(uint8) 0x52  <<(uint8) 0x75  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x03  <<(uint8) 0x62  <<(uint8) 0x75  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x1E  <<(uint8) 0x00;
   
   }


};

class SpawnProjectile : public BasePacket {
public:
   SpawnProjectile(Projectile* p) : BasePacket(PKT_S2C_SpawnProjectile, p->getNetId()) {
      buffer << p->getX() << 150.f << p->getY();
      buffer << p->getX() << 150.f << p->getY();
      buffer << (uint64)0x000000003f510fe2; // unk
      buffer << (float)0.577f; // unk
      buffer << p->getTarget()->getX() << 150.f << p->getTarget()->getY();
      buffer << p->getX() << 150.f << p->getY();
      buffer << p->getTarget()->getX() << 150.f << p->getTarget()->getY();
      buffer << p->getX() << 150.f << p->getY();
      buffer << uint32(0); // unk
      buffer << 2000.f; // Projectile speed
      buffer << (uint64)0x00000000d5002fce; // unk
      buffer << (uint32)0x7f7fffff; // unk
      buffer << (uint8)0 << (uint8)0x66 << (uint8)0;
      buffer << (uint32)0x0a0fe625; // unk (projectile ID)
      buffer << (uint32)0; // Second net ID
      buffer << (uint8)0; // unk
      buffer << (uint32)0x3f800000; // unk (1.0f)
      buffer << p->getOwner()->getNetId() << p->getOwner()->getNetId();
      
      Champion* c = dynamic_cast<Champion*>(p->getOwner());
      if(c) {
         buffer << (uint32)c->getChampionHash();
      } else {
         buffer << (uint32)0;
      }
      
      buffer << p->getNetId();
      buffer << p->getTarget()->getX() << 150.f << p->getTarget()->getY();
      buffer << p->getTarget()->getX() << 150.f << p->getTarget()->getY();
      buffer << (uint32)0x80000000; // unk
      buffer << (uint32)0x000000bf; // unk
      buffer << (uint32)0x80000000; // unk
      buffer << (uint32)0x2fd5843f; // unk
      buffer << (uint32)0x00000000; // unk
      buffer << (uint16)0x0000; // unk
      buffer << (uint8)0x2f; // unk
      buffer << (uint32)0x00000000; // unk
      buffer << p->getX() << 150.f << p->getY();
      buffer << (uint64)0x0000000000000000; // unk
   }

};

class UpdateStats : public GamePacket {
public:
   UpdateStats(Unit* u) : GamePacket(PKT_S2C_CharStats, u->getNetId()) {
      const std::multimap<uint8, uint32>& stats = u->getStats().getUpdatedStats();
      
      std::set<uint8> masks;
      uint8 masterMask = 0;
      
      for(auto& p : stats) {
         masterMask |= p.first;
         masks.insert(p.first);
      }
      
      buffer << (uint8)1;
      buffer << masterMask;
      buffer << u->getNetId();
      
      for(uint8 m : masks) {
         uint32 mask = 0;
         
         for(auto it = stats.lower_bound(m); it != stats.upper_bound(m); ++it) {
            mask |= it->second;
         }
         
         buffer << mask;
         buffer << (uint8)(stats.count(m)*4);
         
         for(int i = 0; i < 32; ++i) {
            uint32 tmpMask = (1 << i);
            if(tmpMask & mask) {
               buffer << u->getStats().getStat(m, tmpMask);
            }
         }
      }
   }
};

class LevelPropSpawn : public BasePacket {
    public:
        LevelPropSpawn(LevelProp* lp) : BasePacket(PKT_S2C_LevelPropSpawn) {
            buffer << lp->getNetId();
            buffer << (uint32)0x00000040; // unk
            buffer << (uint8)0; // unk
            buffer << lp->getX() << lp->getZ() << lp->getY();
            buffer.fill(0, 41); // unk
            buffer << lp->getName();
            buffer.fill(0, 64-lp->getName().length());
            buffer << lp->getType();
            buffer.fill(0, 64-lp->getType().length());
        }
        
        // TODO : remove this once we find a better solution for jungle camp spawning command
        LevelPropSpawn(uint32 netId, const std::string& name, const std::string& type, float x, float y, float z) : BasePacket(PKT_S2C_LevelPropSpawn) {
            buffer << netId;
            buffer << (uint32)0x00000040; // unk
            buffer << (uint8)0; // unk
            buffer << x << z << y;
            buffer.fill(0, 41); // unk
            buffer << name;
            buffer.fill(0, 64-name.length());
            buffer << type;
            buffer.fill(0, 64-type.length());
        }

};

struct ViewRequest {
    uint8 cmd;
    uint32 unk1;
    float x;
    float zoom;
    float y;
    float y2;		//Unk
    uint32 width;	//Unk
    uint32 height;	//Unk
    uint32 unk2;	//Unk
    uint8 requestNo;
};

class ViewAnswer : public Packet {
public:
   ViewAnswer(ViewRequest *request) : Packet(PKT_S2C_ViewAns) {
      buffer << request->unk1;
   }
   void setRequestNo(uint8 requestNo){
      buffer << requestNo;
   }
};
/* End New Packets */

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

#endif
