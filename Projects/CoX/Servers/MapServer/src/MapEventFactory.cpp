/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include "MapEvents.h"

MapLinkEvent *MapEventFactory::EventFromStream(BitStream &bs)
{
    size_t read_pos = bs.GetReadPos();

    MapLinkEvent *ev = CRUD_EventFactory::EventFromStream(bs);
    if(ev) // base class created the event
        return ev;

    bs.SetReadPos(read_pos); // rewind the stream and retry
    u8 opcode = bs.GetPackedBits(1);
    switch(opcode)
    {
    case 1: return new ConnectRequest;
    case 9: return new NewEntity;
    case 2: return new InputState;
    case 4: return new ShortcutsRequest;
    case 3: return new SceneRequest;
    case 5: return new EntitiesRequest;
    default: return new MapUnknownRequest;
    }
    return 0;
}
