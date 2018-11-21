/*
 CoAP library for Arduino.
 This software is released under the MIT License.
 Copyright (c) 2014 Hirotaka Niisato
 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/****************************************************************
 * Coap internal function call tree:
 
 loop()
    |
    packetRecievedBehavior(CoapPacket &packet)
        |
        [User defined callback]
 
 ****************************************************************/

#ifndef Coap_h
#define Coap_h

#include "CoapUri.h"

class Coap {
protected:
    UDP             *udp; /* udp communication */
    CoapUri         uri; /* store resources */
    int             port; /* udp port */
    
    /****************************************************************
     * Send a packet to specific host.
     ****************************************************************/
    uint16_t        sendPacket(CoapPacket &packet, IPAddress ip, int port = COAP_DEFAULT_PORT);
    
    /****************************************************************
     * Define a behavior when a complete packet arrives.
     ****************************************************************/
    virtual void    packetRecievedBehavior(CoapPacket &packet) = 0;
    
public:
    Coap(UDP &udp);
    
    /****************************************************************
     * Start udp communication.
     ****************************************************************/
    void            start(int port = COAP_DEFAULT_PORT);
    
    /****************************************************************
     * Define repeated tasks.
     ****************************************************************/
    bool            loop();
};

#endif /* Coap_h */

