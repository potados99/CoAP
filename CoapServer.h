//
//  CoapServer.hpp
//  ESP-CoAP-Test
//
//  Created by POTADOS on 21/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#ifndef CoapServer_h
#define CoapServer_h

#include "Coap.h"

class CoapServer : public Coap {
private:
    virtual void    packetRecievedBehavior(CoapPacket &packet);

public:
    CoapServer(UDP &udp);
    
    /****************************************************************
     * Add a resource and a callback for it.
     ****************************************************************/
    bool            addResource(callback c, String url);
    
    /****************************************************************
     * Send a response.
     ****************************************************************/
    uint16_t        sendResponse(CoapPacket &request, IPAddress ip, int port, char *payload);
};

#endif /* CoapServer_h */
