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
    
    // 수신 데이터 저장할 버퍼 ** 계속 살아있어야 한다!!.
    uint8_t buffer[BUF_MAX_SIZE];
    
    // 가져올 패킷 크기를 미리 알아냄. BUF_MAX_SIZE보다 작아야 할 것으로 추정
    int32_t packetlen = udp->parsePacket();
    
    // 가져올 것이 있는 경우에만 반복!
    while (packetlen > 0) {
        // 비우기는 기본.
        memset(buffer, 0, sizeof(buffer));
        
        // 제대로 읽어서 버퍼에다가 넣는데, 가져올 것이 버퍼 사이즈보다 크면 BUF_MAX_SIZE로 잘라버림
        packetlen = udp->read(buffer, packetlen >= BUF_MAX_SIZE ? BUF_MAX_SIZE : packetlen);
        
        // 가져온 버퍼를 패킷에다가 넣을 것이므로 패킷 생성.
        CoapPacket packet;
        
        // 파싱 고
        bool success = CoapPacket::parsePacket(packet, buffer, packetlen);
        if (! success) { return false; }
     
        /****************************************************************
         * 정상적으로 파싱 완료
         ****************************************************************/
        
        // 요청 타입에 따라 처리
        
        if (packet.type == COAP_ACK) {
            // 만약 ACK가 날아오면 콜백을 실행
            launchCallback(packet, udp->remoteIP(), udp->remotePort());
        }
        else {
            // 패킷 타입이 CON | NONCON | RESET 일 때
            String url = "";

            for (int i = 0; i < packet.optionnum; i++) {
                // 옵션을 하나씩 뒤지면서
                if (packet.options[i].number == COAP_URI_PATH && packet.options[i].length > 0) {
                    // 옵션이 URI_PATH이고 그 옵션 길이가 0보다 클 때
                    // 그 옵션 길이보다 1 큰 배열을 만들어줌.
                    char urlname[packet.options[i].length + 1];
                    
                    // 그 크기만큼 옵션 버퍼에서 배열로 복사해줌.
                    memcpy(urlname, packet.options[i].buffer, packet.options[i].length);
                    
                    // 마지막은 NULL terminate 시켜줌.
                    urlname[packet.options[i].length] = NULL;
                    
                    // 찾은 url이 처음이면 넘어가고, 다음부터는 / 붙인다.
                    if(url.length() > 0) {
                        url += "/";
                    }
                    
                    // url에 붙여준다.
                    url += urlname;
                }
            }
            
            // 그 url로 리소스를 찾아보았는데 안 나오면
            if (! uri.find(url)) {
                // 응답을 보낸다. COAP_NOT_FOUND라고.
                sendResponse(udp->remoteIP(),
                             udp->remotePort(),
                             packet.messageid,
                             NULL,
                             0,
                             COAP_NOT_FOUND,
                             COAP_NONE,
                             NULL,
                             0);
                
            }
            else {
                // 나오면? 콜백을 실행한다.
                callback foundCallback = uri.find(url);
                foundCallback(packet, udp->remoteIP(), udp->remotePort());
            } /* 리소스 찾는 if 끝 */
            
        } /* 패킷 타입이 CON | NONCON | RESET 인 if 끝 */
        
        /* this type check did not use.
         if (packet.type == COAP_CON) {
         // send response
         sendResponse(_udp->remoteIP(), _udp->remotePort(), packet.messageid);
         }
         */
        
        // 다음 패킷으로 가즈아
        packetlen = udp->parsePacket();
    }
    
    // 가져올 것이 없을 때!
    return true;
}


/****************************************************************
 * private 메소드
 ***************************************************************/

// 완성
bool Coap::launchCallback(CoapPacket& packet, IPAddress ip, int port) {
    if (responseCallback) {
        responseCallback(packet, ip, port);
        return true;
    }
    return false;
}

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


