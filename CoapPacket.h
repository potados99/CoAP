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
private:
    static int parseOption(CoapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen);
    
public:
    uint8_t         version = COAP_VERSION; /* 현재 1 */
    uint8_t         type;
    uint8_t         code;
    uint8_t         *token;
    uint8_t         tokenlen;
    uint8_t         *payload;
    uint8_t         payloadlen;
    uint16_t        messageid;
    uint8_t         optionnum;
    CoapOption      options[MAX_OPTION_NUM];
    
    
    /****************************************************************
     * CoapPacket 생성자 그룹
     ****************************************************************/
    
    // 인자가 없는 생성자.
    CoapPacket();
    
    // 인자들로부터 패킷을 생성합니다.
    CoapPacket(IPAddress senderIp,
               char *url,
               COAP_TYPE type,
               COAP_METHOD method,
               uint8_t *token,
               uint8_t tokenlen,
               uint8_t *payload,
               uint32_t payloadlen);

    // 패킷의 내용들을 통신용 버퍼에 전달합니다. 반환값은 패킷의 크기입니다.
    uint16_t        exportToBuffer(uint8_t *destBuffer, uint32_t bufferLen);
    
    // 버퍼로부터 패킷을 파싱합니다.
    static bool     parseCoapPacket(CoapPacket &packet, uint8_t *buffer, uint32_t packetLen);
};

#endif /* CoapPacket_h */
