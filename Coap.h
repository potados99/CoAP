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

#ifndef Coap_h
#define Coap_h

#include "CoapUri.h"

class Coap {
private:
    UDP         *udp; /* 통신에 사용할 udp 객체의 포인터. UDP를 구현한 WiFiUdp 사용하면 됨. */
    CoapUri     uri; /* 리소스를 저장할 CoapUri 객체. */
    callback    responseCallback; /* 요청 응답시 호출될 콜백함수. */
    int         port; /* 통신에 사용할 포트. */
    
    // 콜백 메소드를 실행하여 성공 여부를 반환합니다.
    bool        launchCallback(CoapPacket& packet, IPAddress ip, int port);

    /**********************************************************************
     * sendPacket 오버로드 그룹
     *
     * 패킷을 지정한 ip와 포트로 보냅니다. 반환값은 message id입니다.
     **********************************************************************/
    uint16_t    sendPacket(CoapPacket &packet, IPAddress ip);
    uint16_t    sendPacket(CoapPacket &packet, IPAddress ip, int port);
    
public:
    Coap(UDP& udp);
    
    bool        start();
    bool        start(int port);
    
    // 콜백을 등록합니다.
    bool        response(callback c);
    
    // 리소스와 해당 콜백을 등록합니다.
    bool        server(callback c, String url);
    
    // get 메소드 요청을 보냅니다. 반환값은 message id입니다.
    uint16_t    get(IPAddress ip,
                    int port,
                    char *url);
    
    /**********************************************************************
     * put 오버로드 그룹
     *
     * put 메소드 요청을 보냅니다. 반환값은 message id입니다.
     **********************************************************************/
    uint16_t    put(IPAddress ip,
                    int port,
                    char *url,
                    char *payload);
    uint16_t    put(IPAddress ip,
                    int port,
                    char *url,
                    char *payload,
                    int payloadlen);
    
    // 패킷을 만들어서 보냅니다. 반환값은 message id입니다. 패킷을 보낼 때 반드시 호출됩니다.
    uint16_t    send(IPAddress ip,
                     int port,
                     char *url,
                     COAP_TYPE type,
                     COAP_METHOD method,
                     uint8_t *token,
                     uint8_t tokenlen,
                     uint8_t *payload,
                     uint32_t payloadlen);
    
    /**********************************************************************
     * sendResponse 오버로드 그룹
     *
     * 응답을 보냅니다. 반환값은 message id입니다.
     **********************************************************************/
    uint16_t    sendResponse(IPAddress ip,
                             int port,
                             uint16_t messageid);
    uint16_t    sendResponse(IPAddress ip,
                             int port,
                             uint16_t messageid,
                             char *payload);
    uint16_t    sendResponse(IPAddress ip,
                             int port,
                             uint16_t messageid,
                             char *payload,
                             int payloadlen);
    uint16_t    sendResponse(IPAddress ip,
                             int port,
                             uint16_t messageid,
                             char *payload,
                             int payloadlen,
                             COAP_RESPONSE_CODE code,
                             COAP_CONTENT_TYPE type,
                             uint8_t *token,
                             int tokenlen);
    
    bool        loop();
};

#endif /* Coap_h */

