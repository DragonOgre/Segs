/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "CRUDP_Packet.h"

#include <unordered_map>
#include <deque>
#include <list>
#include <ace/Thread_Mutex.h>
#include <ace/INET_Addr.h>

template <size_t size>
struct FixedSizePacketQueue
{
    using iterator = std::deque<CrudP_Packet *>::iterator;
    std::deque<CrudP_Packet *> m_storage;
    bool                       isFull() const { return m_storage.size() >= size; }
    void                       push_back(CrudP_Packet *v)
    {
        if (!isFull())
            m_storage.push_back(v);
    }
    void          pop_front() { m_storage.pop_front(); }
    CrudP_Packet *front() { return m_storage.empty() ? nullptr : m_storage.front(); }
    bool          empty() const { return m_storage.empty(); }
    iterator      begin() { return m_storage.begin(); }
    iterator      end() { return m_storage.end(); }
    iterator      erase(iterator v) { return m_storage.erase(v); }
    void          clear() { m_storage.clear(); }
};
class PacketCodecNull;
class CrudP_Protocol
{
private:
        typedef std::deque<CrudP_Packet *> pPacketStorage;
        typedef pPacketStorage::iterator ipPacketStorage ;
        typedef std::unordered_map<uint32_t,pPacketStorage> hmSibStorage;
        friend void PacketSibDestroyer(const std::pair<int, pPacketStorage> &a);
        static constexpr const int max_packet_data_size = 1272;

        uint32_t            send_seq=0;
        uint32_t            recv_seq=0;
        uint32_t            sibling_id=0;

        PacketCodecNull *   m_codec = nullptr;
        pPacketStorage      avail_packets;
        pPacketStorage      unsent_packets;
        std::list<uint32_t> recv_acks; // each successful receive will store it's ack here
        hmSibStorage        sibling_map; // we need to lookup mPacketGroup quickly, and insert ordered packets into mPacketGroup
        ACE_Thread_Mutex    m_packets_mutex;
        bool                m_compression_allowed=false;
        CrudP_Packet *      mergeSiblings(uint32_t id);
        bool                insert_sibling(CrudP_Packet *pkt);
static  bool                PacketSeqCompare(const CrudP_Packet *a,const CrudP_Packet *b);
static  bool                PacketSibCompare(const CrudP_Packet *a,const CrudP_Packet *b);
        bool                allSiblingsAvailable(uint32_t sibid);
public:
                            ~CrudP_Protocol();
        void                setCodec(PacketCodecNull *codec){m_codec= codec;}
        PacketCodecNull *   getCodec() const {return m_codec;}

        size_t              UnsentPackets()    const {return unsent_packets.size();}
        size_t              AvailablePackets() const {return avail_packets.size();}
        size_t              UnackedPacketCount() const { return recv_acks.size(); }
        size_t              GetUnsentPackets(std::list<CrudP_Packet *> &);
        void                ReceivedBlock(BitStream &bs); // bytes received, will create some packets in avail_packets

        void                SendPacket(CrudP_Packet *p); // this might split packet 'p' into a few packets
        CrudP_Packet *      RecvPacket(bool disregard_seq);
protected:
        void                sendLargePacket(CrudP_Packet *p);
        void                sendSmallPacket(CrudP_Packet *p);
        void                parseAcks(BitStream &src,CrudP_Packet *tgt);
        void                storeAcks(BitStream &bs);
        void                PushRecvPacket(CrudP_Packet *a); // this will try to join packet 'a' with it's siblings
        void                PacketAck(uint32_t);
        void                clearQueues(bool recv,bool clear_send_queue); // clears out the recv/send queues
};
