//
//  CoapPacket.cpp
//  ESP-CoAP-Test
//
//  Created by POTADOS on 18/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#include "CoapPacket.h"

// 완성
CoapPacket::CoapPacket(IPAddress ip,
                       int port,
                       char *url,
                       COAP_TYPE type,
                       COAP_METHOD method,
                       uint8_t *token,
                       uint8_t tokenlen,
                       uint8_t *payload,
                       uint32_t payloadlen) {
    /*
    패킷을 만듭니다.
     */
    
    // 멤버 초기화
    this->type         = type;
    this->code         = method;
    this->token        = token;
    this->tokenlen     = tokenlen;
    this->payload      = payload;
    this->payloadlen   = payloadlen;
    this->optionnum    = 0;
    this->messageid    = rand();
    
    // 호스트(발신자) 옵션 추가
    String ipaddress = "";
    ipaddress += String(ip[0]);
    ipaddress += String(".");
    ipaddress += String(ip[1]);
    ipaddress += String(".");
    ipaddress += String(ip[2]);
    ipaddress += String(".");
    ipaddress += String(ip[3]);
    
    this->options[this->optionnum].buffer = (uint8_t *)ipaddress.c_str();
    this->options[this->optionnum].length = ipaddress.length();
    this->options[this->optionnum].number = COAP_URI_HOST;
    this->optionnum++;
    
    // URI 옵션 추가
    int idx = 0;
    for (int i = 0; i < strlen(url); i++) {
        if (url[i] == '/') {
            this->options[this->optionnum].buffer = (uint8_t *)(url + idx);
            this->options[this->optionnum].length = i - idx;
            this->options[this->optionnum].number = COAP_URI_PATH;
            this->optionnum++;
            idx = i + 1;
        }
    }
    
    if (idx <= strlen(url)) {
        this->options[this->optionnum].buffer = (uint8_t *)(url + idx);
        this->options[this->optionnum].length = strlen(url) - idx;
        this->options[this->optionnum].number = COAP_URI_PATH;
        this->optionnum++;
    }
}

CoapPacket::CoapPacket(IPAddress ip,
                       int port,
                       uint16_t messageid,
                       char *payload,
                       int payloadlen,
                       COAP_RESPONSE_CODE code,
                       COAP_CONTENT_TYPE type,
                       uint8_t *token,
                       int tokenlen) {
    /*
     답장용 패킷을 만듭니다.
     */
    
    // 멤버 초기화
    this->type = COAP_ACK;
    this->code = code;
    this->token = token;
    this->tokenlen = tokenlen;
    this->payload = (uint8_t *)payload;
    this->payloadlen = payloadlen;
    this->optionnum = 0;
    this->messageid = messageid;
    
    // 옵션 추가
    char optionBuffer[2];
    optionBuffer[0] = ((uint16_t)type & 0xFF00) >> 8;
    optionBuffer[1] = ((uint16_t)type & 0x00FF) ;
    this->options[this->optionnum].buffer = (uint8_t *)optionBuffer;
    this->options[this->optionnum].length = 2;
    this->options[this->optionnum].number = COAP_CONTENT_FORMAT;
    this->optionnum++;
}

CoapPacket::CoapPacket(uint8_t *sourceBuffer,
                       uint32_t packetLen) {
    
    
}

uint16_t CoapPacket::exportToBuffer(uint8_t *destBuffer, uint32_t bufferLen) {
    uint8_t *bufPtr = destBuffer;
    uint16_t running_delta = 0;
    uint16_t packetSize = 0;
    
    // 패킷 베이스 헤더를 만듭니다.
    *bufPtr          = this->version << 6;
    *bufPtr          |= (this->type & 0x03) << 4;
    *bufPtr++        |= (this->tokenlen & 0x0F);
    *bufPtr++        = this->code;
    *bufPtr++        = (this->messageid >> 8);
    *bufPtr++        = (this->messageid & 0xFF);
    bufPtr           = destBuffer + COAP_HEADER_SIZE;
    packetSize       += 4; /* bytes */
    
    // 토큰을 만듭니다.
    if (this->token != NULL && this->tokenlen <= 0x0F) {
        memcpy(bufPtr, this->token, this->tokenlen);
        bufPtr += this->tokenlen;
        packetSize += this->tokenlen;
    }
    
    // 옵션 헤더를 만듭니다.
    for (int i = 0; i < this->optionnum; i++)  {
        uint32_t optdelta = 0;
        uint8_t len = 0;
        uint8_t delta = 0;
        
        if (packetSize + 5 + this->options[i].length >= bufferLen) {
            return 0;
        }
        
        optdelta = this->options[i].number - running_delta;
        COAP_OPTION_DELTA(optdelta, &delta);
        COAP_OPTION_DELTA((uint32_t)this->options[i].length, &len);
        
        *bufPtr++ = (0xFF & (delta << 4 | len));
        if (delta == 13) {
            *bufPtr++ = (optdelta - 13);
            packetSize++;
        }
        else if (delta == 14) {
            *bufPtr++ = ((optdelta - 269) >> 8);
            *bufPtr++ = (0xFF & (optdelta - 269));
            packetSize+=2;
        }
        
        if (len == 13) {
            *bufPtr++ = (this->options[i].length - 13);
            packetSize++;
        }
        else if (len == 14) {
            *bufPtr++ = (this->options[i].length >> 8);
            *bufPtr++ = (0xFF & (this->options[i].length - 269));
            packetSize+=2;
        }
        
        memcpy(bufPtr, this->options[i].buffer, this->options[i].length);
        bufPtr += this->options[i].length;
        packetSize += this->options[i].length + 1;
        running_delta = this->options[i].number;
    }
    
    // 페이로드를 만듭니다.
    if (this->payloadlen > 0) {
        
        if ((packetSize + 1 + this->payloadlen) >= bufferLen) {
            return 0;
        }
        
        *bufPtr++ = 0xFF;
        memcpy(bufPtr, this->payload, this->payloadlen);
        packetSize += (1 + this->payloadlen);
    }
    
    return packetSize;
}
