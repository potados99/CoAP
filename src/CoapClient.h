//
//  CoapClient.h
//  CoAP-simple-library
//
//  Created by POTADOS on 21/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#ifndef CoapClient_hpp
#define CoapClient_hpp

#include "Coap.h"

class CoapClient : public Coap {
private:
    /**
     * Client side single callback.
     */
    callback        responseCallback;

    /**
     * Launch callback when available.
     *
     * @param packet        Incomming packet.
     * @param ip            Sender IP.
     * @param port          Sender port.
     *
     * @return          False when callback doesn't exist.
     */
    bool            launchCallback(CoapPacket &packet, IPAddress ip, int port);

    virtual void    packetRecievedBehavior(CoapPacket &request);

public:
    CoapClient(UDP &udp);

    /**
     * Register an end-to-end single callback.
     *
     * @param c             Callback to register.
     *
     * @return              False when c is null.
     */
    bool            registerCallback(callback c);

    /**
     * Send a packet with GET method.
     *
     * @param ip            Receiver IP.
     * @param port          Receiver port.
     * @param url           Url of this client.
     *
     * @return              Message id of sent packet.
     */
    uint16_t        get(IPAddress ip, int port, char *url);

    /**
     * Send a packet with PUT method.
     *
     * @param ip            Receiver IP.
     * @param port          Receiver port.
     * @param url           Url of this client.
     *
     * @return          Message id of sent packet.
     */
    uint16_t        put(IPAddress ip, int port, char *url, char *payload);

    /**
     * Create and send a packet. Methods above are implemented on it.
     * Returns message id.
     *
     * @param ip            Receiver IP.
     * @param port          Receiver port.
     * @param url           Url of this client.
     * @param type          Type of packet to send.
     * @param method        Method to use.
     * @param token         Token of packet.
     * @param tokenlen      Length of token.
     * @param payload       Payload to send.
     * @param payloadlen    Length of payload.
     *
     * @return          Message id of sent packet.
     **/
    uint16_t        send(IPAddress ip, int port, char *url,
                         COAP_TYPE type,
                         COAP_METHOD method,
                         uint8_t *token,
                         uint8_t tokenlen,
                         uint8_t *payload,
                         uint32_t payloadlen);
};

#endif /* CoapClient_h */
