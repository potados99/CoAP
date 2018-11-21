//
//  CoapClient.hpp
//  ESP-CoAP-Test
//
//  Created by POTADOS on 21/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#ifndef CoapClient_hpp
#define CoapClient_hpp

#include "Coap.h"

class CoapClient : public Coap {
private:
    callback        responseCallback; /* client side single callback */
    
    /****************************************************************
     * Launch callback when available.
     ****************************************************************/
    bool            launchCallback(CoapPacket &packet, IPAddress ip, int port);

    virtual void    packetRecievedBehavior(CoapPacket &packet);

public:
    CoapClient(UDP &udp);
    
    /****************************************************************
     * Register an end-to-end single callback.
     ****************************************************************/
    bool            response(callback c);
    
    /****************************************************************
     * Send a GET method.
     ****************************************************************/
    uint16_t        get(IPAddress ip, int port, char *url);

    /****************************************************************
     * Send a PUT method.
     ****************************************************************/
    uint16_t        put(IPAddress ip, int port, char *url, char *payload);
    
    /****************************************************************
     * Create and send a packet. Methods above are implemented on it.
     * Returns message id.
     ****************************************************************/
    uint16_t        send(IPAddress ip, int port, char *url,
                         COAP_TYPE type,
                         COAP_METHOD method,
                         uint8_t *token,
                         uint8_t tokenlen,
                         uint8_t *payload,
                         uint32_t payloadlen);
};

#endif /* CoapClient_h */
