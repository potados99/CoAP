//
//  CoapPacket.hpp
//  ESP-CoAP-Test
//
//  Created by POTADOS on 18/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#ifndef CoapPacket_h
#define CoapPacket_h

#include "CoapOption.h"

class CoapPacket {
public:
    uint8_t version = COAP_VERSION; /* 현재 1 */
    uint8_t type;
    uint8_t code;
    uint8_t *token;
    uint8_t tokenlen;
    uint8_t *payload;
    uint8_t payloadlen;
    uint16_t messageid;
    uint8_t optionnum;
    
    CoapOption options[MAX_OPTION_NUM];
    
    CoapPacket();
    
    // 인자들로부터 패킷을 생성합니다.
    CoapPacket(IPAddress ip,
               int port,
               char *url,
               COAP_TYPE type,
               COAP_METHOD method,
               uint8_t *token,
               uint8_t tokenlen,
               uint8_t *payload,
               uint32_t payloadlen);
    
    // 인자들로부터 패킷을 생성합니다. (답장용)
    CoapPacket(IPAddress ip,
               int port,
               uint16_t messageid,
               char *payload,
               int payloadlen,
               COAP_RESPONSE_CODE code,
               COAP_CONTENT_TYPE type,
               uint8_t *token,
               int tokenlen);
    
    // 버퍼로부터 패킷을 생성합니다.
    CoapPacket(uint8_t *sourceBuffer,
               uint32_t packetLen);
    
    // 패킷의 내용들을 통신용 버퍼에 전달합니다. 반환값은 패킷의 크기입니다.
    uint16_t exportToBuffer(uint8_t *destBuffer, uint32_t bufferLen);
};

#endif /* CoapPacket_h */
