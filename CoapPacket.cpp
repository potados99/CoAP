//
//  CoapPacket.cpp
//  ESP-CoAP-Test
//
//  Created by POTADOS on 18/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#include "CoapPacket.h"

/****************************************************************
 * public
 ****************************************************************/

CoapPacket::CoapPacket() {
    type = 0;
    code = 0;
    token = NULL;
    tokenlen = 0;
    payload = NULL;
    payloadlen = 0;
    messageid = 0;
    optionnum = 0;
    
    memset(options, 0, MAX_OPTION_NUM);
}

CoapPacket::CoapPacket(IPAddress senderIP,
                       char *url,
                       COAP_TYPE type,
                       COAP_METHOD method,
                       uint8_t *token,
                       uint8_t tokenlen,
                       uint8_t *payload,
                       uint32_t payloadlen) {

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
    ipaddress += String(senderIP[0]);
    ipaddress += String(".");
    ipaddress += String(senderIP[1]);
    ipaddress += String(".");
    ipaddress += String(senderIP[2]);
    ipaddress += String(".");
    ipaddress += String(senderIP[3]);
    
    this->options[this->optionnum].buffer = (uint8_t *)ipaddress.c_str();
    this->options[this->optionnum].length = ipaddress.length();
    this->options[this->optionnum].number = COAP_URI_HOST;
    this->optionnum ++;
    
    // URI 옵션 추가
    int idx = 0;
    for (int i = 0; i < strlen(url); i++) {
        if (url[i] == '/') {
            this->options[this->optionnum].buffer = (uint8_t *)(url + idx);
            this->options[this->optionnum].length = i - idx;
            this->options[this->optionnum].number = COAP_URI_PATH;
            this->optionnum ++;
            idx = i + 1;
        }
    }
    
    if (idx <= strlen(url)) {
        this->options[this->optionnum].buffer = (uint8_t *)(url + idx);
        this->options[this->optionnum].length = strlen(url) - idx;
        this->options[this->optionnum].number = COAP_URI_PATH;
        this->optionnum ++;
    }
}

// 완성
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


CoapPacket CoapPacket::makeResponsePair(COAP_TYPE type, COAP_RESPONSE_CODE responseCode, uint8_t *responseOptionBuffer) {
    CoapPacket response;
    
    response.version = this->version;
    response.type = type;
    response.code = responseCode;
    
    response.token = this->token;
    response.tokenlen = this->tokenlen;
    response.messageid = this->messageid;
    
    if (responseOptionBuffer) {
        responseOptionBuffer[0] = ((uint16_t)COAP_TEXT_PLAIN  & 0xFF00) >> 8;
        responseOptionBuffer[1] = ((uint16_t)COAP_TEXT_PLAIN  & 0x00FF) ;
        response.options[response.optionnum].buffer = responseOptionBuffer;
        response.options[response.optionnum].length = 2;
        response.options[response.optionnum].number = COAP_CONTENT_FORMAT;
        response.optionnum ++;
    }
    
    return response;
}


String CoapPacket::getUriPath() {
    String url = "";
    
    for (int i = 0; i < this->optionnum; i++) {
        // 옵션을 하나씩 뒤지면서
        if (this->options[i].number == COAP_URI_PATH && this->options[i].length > 0) {
            // 옵션이 URI_PATH이고 그 옵션 길이가 0보다 클 때
            // 그 옵션 길이보다 1 큰 배열을 만들어줌.
            char urlname[this->options[i].length + 1];
            
            // 그 크기만큼 옵션 버퍼에서 배열로 복사해줌.
            memcpy(urlname, this->options[i].buffer, this->options[i].length);
            
            // 마지막은 NULL terminate 시켜줌.
            urlname[this->options[i].length] = NULL;
            
            // 찾은 url이 처음이면 넘어가고, 다음부터는 / 붙인다.
            if(url.length() > 0) {
                url += "/";
            }
            
            // url에 붙여준다.
            url += urlname;
        }
    }
    
    return url;
}

bool CoapPacket::parseCoapPacket(CoapPacket &packet, uint8_t *buffer, uint32_t packetlen) {
    // 기초 예외처리. 패킷이 헤더보다 작을 때
    if (packetlen < COAP_HEADER_SIZE) {
        return false;
    }
    
    // 버퍼로부터 패킷에 대입.
    packet.version      = (buffer[0] & 0xC0) >> 6;
    packet.type         = (buffer[0] & 0x30) >> 4;
    packet.tokenlen     = buffer[0] & 0x0F;
    packet.code         = buffer[1];
    packet.messageid    = 0xFF00 & (buffer[2] << 8);
    packet.messageid    |= 0x00FF & buffer[3];
    
    
    /****************************************************************
     * 대략 가져온 정보들로 예외처리하는 그룹
     ****************************************************************/
    
    // 버전 예외처리.
    if (packet.version != COAP_VERSION) {
        // 버전이 안 맞으면 새 패킷 가져오고 다음 루프로 점프
        return false;
    }
    
    // 토큰 예외처리.
    if (packet.tokenlen < 0 || packet.tokenlen > 8) {
        // 토큰 길이가 0 미만이거나 8 초과이면 새 패킷 가져오고 다음 루프로 점프.
        return false;
    }
    
    // 토큰 처리
    if (packet.tokenlen == 0)  {
        packet.token = NULL;
    }
    else if (packet.tokenlen <= 8) {
        packet.token = buffer + COAP_HEADER_SIZE;
    }
    
    // 패킷 사이즈 예외처리
    if (COAP_HEADER_SIZE + packet.tokenlen >= packetlen) {
        // 헤더에 토큰 길이만 더했는데 끝났을 때, 거른다.
        return false;
    }
    
    // 옵션 다루기
    int optionIndex = 0;
    uint16_t delta = 0;
    uint8_t *end = buffer + packetlen; /* 버퍼의 끝 */
    uint8_t *p = buffer + COAP_HEADER_SIZE + packet.tokenlen; /* 헤더와 토큰 지난 다음 */
    
    // 옵션의 수가 범위 내이고, p가 끝이 아니며 p의 값이 255가 아닐 때 반복!
    while(optionIndex < MAX_OPTION_NUM && *p != 0xFF && p < end) {
        //packet.options[optionIndex];
        
        // 옵션을 파싱하는데 리턴이 0이 아니면
        if (parseCoapOptions(&packet.options[optionIndex], &delta, &p, end-p) != 0) {
            // 가망이 없어 그냥 리턴시킨다.
            return false;
        }
        
        // 옵션 파싱이 성공이므로 인덱스를 하나 늘린다.
        optionIndex++;
    }
    
    // 예외: 옵션이 너무 많아서 while문을 빠져나왔을 때
    if (optionIndex >= MAX_OPTION_NUM) {
        // 노답이다. 다시 가야 한다.
        return false;
    }
    
    // 모든 옵션을 파싱했으니 패킷의 옵션 수도 정해짐.
    packet.optionnum = optionIndex;
    
    // p에 하나 더해도 끝이 아니고, p의 값이 255이면 (while문을 나와 예외처리에서 살아남으면, p가 마커(255)를 마주쳤거나 메시지가 끝난 두 경우 밖에 없다.)
    if (p+1 < end && *p == 0xFF) {
        // 페이로드는 p+1부터 끝까지이다.
        packet.payload = p+1;
        packet.payloadlen = end-(p+1);
    } else {
        // 그게 안되면 페이로드는 없다.
        packet.payload = NULL;
        packet.payloadlen= 0;
    }
    
    return true;
}


/****************************************************************
 * private
 ****************************************************************/

int CoapPacket::parseCoapOptions(CoapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen) {
    uint8_t *p = *buf;
    uint8_t headlen = 1;
    uint16_t len, delta;
    
    if (buflen < headlen) { return -1; }
    
    delta = (p[0] & 0xF0) >> 4;
    len = p[0] & 0x0F;
    
    if (delta == 13) {
        headlen += 1;
        if (buflen < headlen) { return -1; }
        delta = p[1] + 13;
        p += 1;
    }
    else if (delta == 14) {
        headlen += 2;
        if (buflen < headlen) { return -1; }
        delta = ((p[1] << 8) | p[2]) + 269;
        p += 2;
    }
    else if (delta == 15) { return -1; }
    
    if (len == 13) {
        headlen += 1;
        if (buflen < headlen) { return -1; }
        len = p[1] + 13;
        p += 1;
    }
    else if (len == 14) {
        headlen += 2;
        if (buflen < headlen) { return -1; }
        len = ((p[1] << 8) | p[2]) + 269;
        p += 2;
    }
    else if (len == 15) { return -1; }
    
    if ((p + 1 + len) > (*buf + buflen)) { return -1; }
    
    option->number = delta + (*running_delta);
    option->buffer = p + 1;
    option->length = len;
    *buf = p + 1 + len;
    *running_delta += delta;
    
    return 0;
}
