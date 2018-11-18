#include "Coap.h"

#define LOGGING

/****************************************************************
 * public 메소드
 ***************************************************************/

// 완성
Coap::Coap(UDP& udp) {
    this->udp = &udp;
}

// 완성
bool Coap::start() {
    this->start(COAP_DEFAULT_PORT);
    return true;
}

// 완성
bool Coap::start(int port) {
    this->udp->begin(port);
    return true;
}

// 완성
bool Coap::response(callback c) {
    if (c) responseCallback = c;
    else return false;
    
    return true;
}

// 완성
bool Coap::server(callback c, String url) {
    if (c && url) uri.add(c, url);
    else return false;
    
    return true;
}

// 완성
uint16_t Coap::get(IPAddress ip,
                   int port,
                   char *url) {
    return this->send(ip,
                      port,
                      url,
                      COAP_CON,
                      COAP_GET,
                      NULL,
                      0,
                      NULL,
                      0);
}

// 완성
uint16_t Coap::put(IPAddress ip,
                   int port,
                   char *url,
                   char *payload) {
    return this->send(ip,
                      port,
                      url,
                      COAP_CON,
                      COAP_PUT,
                      NULL,
                      0,
                      (uint8_t *)payload,
                      (uint32_t)strlen(payload));
}

// 완성
uint16_t Coap::put(IPAddress ip,
                   int port,
                   char *url,
                   char *payload,
                   int payloadlen) {
    return this->send(ip,
                      port,
                      url,
                      COAP_CON,
                      COAP_PUT,
                      NULL,
                      0,
                      (uint8_t *)payload,
                      payloadlen);
}

// 완성
uint16_t Coap::send(IPAddress ip,
                    int port,
                    char *url,
                    COAP_TYPE type,
                    COAP_METHOD method,
                    uint8_t *token,
                    uint8_t tokenlen,
                    uint8_t *payload,
                    uint32_t payloadlen) {

    // 패킷 만들기
    CoapPacket packet(ip, port, url, type, method, token, tokenlen, payload, payloadlen);

    // 패킷 보내기
    return this->sendPacket(packet, ip, port); /* 반환값은 message id */
}

// 완성
uint16_t Coap::sendResponse(IPAddress ip,
                            int port,
                            uint16_t messageid) {
    return this->sendResponse(ip,
                              port,
                              messageid,
                              NULL,
                              0,
                              COAP_CONTENT,
                              COAP_TEXT_PLAIN,
                              NULL,
                              0);
}

// 완성
uint16_t Coap::sendResponse(IPAddress ip,
                            int port,
                            uint16_t messageid,
                            char *payload) {
    return this->sendResponse(ip,
                              port,
                              messageid,
                              payload,
                              (uint32_t)strlen(payload),
                              COAP_CONTENT,
                              COAP_TEXT_PLAIN,
                              NULL, 0
                              );
}

// 완성
uint16_t Coap::sendResponse(IPAddress ip,
                            int port,
                            uint16_t messageid,
                            char *payload,
                            int payloadlen) {
    return this->sendResponse(ip,
                              port,
                              messageid,
                              payload,
                              payloadlen,
                              COAP_CONTENT,
                              COAP_TEXT_PLAIN,
                              NULL,
                              0);
}

// 완성
uint16_t Coap::sendResponse(IPAddress ip,
                            int port,
                            uint16_t messageid,
                            char *payload,
                            int payloadlen,
                            COAP_RESPONSE_CODE code,
                            COAP_CONTENT_TYPE type,
                            uint8_t *token,
                            int tokenlen) {
    
    // 패킷 만들기
    CoapPacket packet(ip, port, messageid, payload, payloadlen, code, type, token, tokenlen);

    // 패킷 보내기
    return this->sendPacket(packet, ip, port); /* 반환값은 message id */
}

//
bool Coap::loop() {
    
    uint8_t buffer[BUF_MAX_SIZE];
    int32_t packetlen = udp->parsePacket();
    
    while (packetlen > 0) {
        packetlen = udp->read(buffer, packetlen >= BUF_MAX_SIZE ? BUF_MAX_SIZE : packetlen);
        
        CoapPacket packet;
        
        // parse coap packet header
        if (packetlen < COAP_HEADER_SIZE || (((buffer[0] & 0xC0) >> 6) != 1)) {
            packetlen = udp->parsePacket();
            continue;
        }
        
        packet.type = (buffer[0] & 0x30) >> 4;
        packet.tokenlen = buffer[0] & 0x0F;
        packet.code = buffer[1];
        packet.messageid = 0xFF00 & (buffer[2] << 8);
        packet.messageid |= 0x00FF & buffer[3];
        
        if (packet.tokenlen == 0)  packet.token = NULL;
        else if (packet.tokenlen <= 8)  packet.token = buffer + 4;
        else {
            packetlen = udp->parsePacket();
            continue;
        }
        
        // parse packet options/payload
        if (COAP_HEADER_SIZE + packet.tokenlen < packetlen) {
            int optionIndex = 0;
            uint16_t delta = 0;
            uint8_t *end = buffer + packetlen;
            uint8_t *p = buffer + COAP_HEADER_SIZE + packet.tokenlen;
            while(optionIndex < MAX_OPTION_NUM && *p != 0xFF && p < end) {
                packet.options[optionIndex];
                if (0 != parseOption(&packet.options[optionIndex], &delta, &p, end-p))
                    return false;
                optionIndex++;
            }
            packet.optionnum = optionIndex;
            
            if (p+1 < end && *p == 0xFF) {
                packet.payload = p+1;
                packet.payloadlen = end-(p+1);
            } else {
                packet.payload = NULL;
                packet.payloadlen= 0;
            }
        }
        
        if (packet.type == COAP_ACK) {
            // call response function
            responseCallback(packet, udp->remoteIP(), udp->remotePort());
            
        } else {
            
            String url = "";
            // call endpoint url function
            for (int i = 0; i < packet.optionnum; i++) {
                if (packet.options[i].number == COAP_URI_PATH && packet.options[i].length > 0) {
                    char urlname[packet.options[i].length + 1];
                    memcpy(urlname, packet.options[i].buffer, packet.options[i].length);
                    urlname[packet.options[i].length] = NULL;
                    if(url.length() > 0)
                        url += "/";
                    url += urlname;
                }
            }
            
            if (!uri.find(url)) {
                sendResponse(udp->remoteIP(), udp->remotePort(), packet.messageid, NULL, 0,
                             COAP_NOT_FOUNT, COAP_NONE, NULL, 0);
            } else {
                uri.find(url)(packet, udp->remoteIP(), udp->remotePort());
            }
        }
        
        /* this type check did not use.
         if (packet.type == COAP_CON) {
         // send response
         sendResponse(_udp->remoteIP(), _udp->remotePort(), packet.messageid);
         }
         */
        
        // next packet
        packetlen = udp->parsePacket();
    }
    
    return true;
}


/****************************************************************
 * private 메소드
 ***************************************************************/

// 완성
uint16_t Coap::sendPacket(CoapPacket &packet, IPAddress ip) {
    return this->sendPacket(packet, ip, COAP_DEFAULT_PORT);
}

// 완성
uint16_t Coap::sendPacket(CoapPacket &packet, IPAddress ip, int port) {
    uint8_t buffer[BUF_MAX_SIZE] = {0, }; /* 버퍼 초기화 */
    
    uint16_t packetSize = packet.exportToBuffer(buffer, BUF_MAX_SIZE);
    /* 패킷을 집어넣은 버퍼의 크기가 packetSize. */
    
    udp->beginPacket(ip, port);
    udp->write(buffer, packetSize);
    udp->endPacket();
    
    return packet.messageid;
}

int Coap::parseOption(CoapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen) {
    uint8_t *p = *buf;
    uint8_t headlen = 1;
    uint16_t len, delta;
    
    if (buflen < headlen) return -1;
    
    delta = (p[0] & 0xF0) >> 4;
    len = p[0] & 0x0F;
    
    if (delta == 13) {
        headlen++;
        if (buflen < headlen) return -1;
        delta = p[1] + 13;
        p++;
    } else if (delta == 14) {
        headlen += 2;
        if (buflen < headlen) return -1;
        delta = ((p[1] << 8) | p[2]) + 269;
        p+=2;
    } else if (delta == 15) return -1;
    
    if (len == 13) {
        headlen++;
        if (buflen < headlen) return -1;
        len = p[1] + 13;
        p++;
    } else if (len == 14) {
        headlen += 2;
        if (buflen < headlen) return -1;
        len = ((p[1] << 8) | p[2]) + 269;
        p+=2;
    } else if (len == 15)
        return -1;
    
    if ((p + 1 + len) > (*buf + buflen))  return -1;
    option->number = delta + *running_delta;
    option->buffer = p+1;
    option->length = len;
    *buf = p + 1 + len;
    *running_delta += delta;
    
    return 0;
}
