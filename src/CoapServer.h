//
//  Coap.cpp
//  CoAP-simple-library
//
//  Created by POTADOS on 21/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#ifndef CoapServer_h
#define CoapServer_h

#include "Coap.h"

class CoapServer : public Coap {
private:
    virtual void    packetRecievedBehavior(CoapPacket &request);

    /**
     * Send a response that says the resource requested doesn't exist
     *
     * @param request       Incomming packet.
     * @param ip            Sender IP.
     * @param port          Sender port.
     *
     * @return              Message id of sent packet.
     */
    uint16_t        sendResourceNotFoundResponse(CoapPacket &request, IPAddress ip, int port);

    /**
     * Send an empty response.
     * Used when responding to a ping request.
     *
     * @param request       Incomming packet.
     * @param ip            Sender IP.
     * @param port          Sender port.
     *
     * @return              Message id of sent packet.
     */
    uint16_t        sendEmptyResponse(CoapPacket &request, IPAddress ip, int port);

public:
    CoapServer(UDP &udp);

    /**
     * Add a resource and a callback for it.
     *
     * @param url           Resource url.
     * @param c             Callback for resource.
     *
     * @return              False when c or url is null.
     */
    bool            addResource(String url, callback c);

    /**
     * Send a general response with a payload.
     * Called in a user callback.
     *
     * @param request       Incomming packet.
     * @param ip            Sender IP.
     * @param port          Sender port.
     * @param payload       Payload to send.
     *
     * @return              Message id of sent packet.
     */
    uint16_t        sendResponse(CoapPacket &request, IPAddress ip, int port, char *payload = NULL);
};

#endif /* CoapServer_h */
