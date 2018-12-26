//
//  CoapServer.hpp
//  ESP-CoAP-Test
//
//  Created by POTADOS on 21/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#ifndef CoapServer_h
#define CoapServer_h

#include "src/Coap.h"

class CoapServer : public Coap {
private:
    virtual void    packetRecievedBehavior(CoapPacket &packet);

    /**
     * Send a response that says the resource requested doesn't exist
     **/
    uint16_t        sendResourceNotFoundResponse(CoapPacket &request, IPAddress ip, int port);
    
    /**
     * Send an empty response.
     * Used when responding to a ping request.
     **/
    uint16_t        sendEmptyResponse(CoapPacket &request, IPAddress ip, int port);
    
public:
    CoapServer(UDP &udp);
    
    /**
     * Add a resource and a callback for it.
     **/
    bool            addResource(callback c, String url);
    
    /**
     * Send a general response.
     * Called in a user callback.
     **/
    uint16_t        sendResponse(CoapPacket &request, IPAddress ip, int port, char *payload = NULL);
};

#endif /* CoapServer_h */
