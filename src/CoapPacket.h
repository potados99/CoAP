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
    /**
     * Parse options from buffer.
     **/
    static int      parseCoapOptions(CoapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen);

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

    CoapPacket();

    CoapPacket(IPAddress senderIp,
               char *url,
               COAP_TYPE type,
               COAP_METHOD method,
               uint8_t *token,
               uint8_t tokenlen,
               uint8_t *payload,
               uint32_t payloadlen);

    /**
     * Serialize packet to buffer.
     **/
    uint16_t        exportToBuffer(uint8_t *destBuffer, uint32_t bufferLen);

    /**
     * Create a packet for response.
     * The packet contains version, type, code, token(and len), message id, options(and len).
     **/
    CoapPacket      makeResponsePair(COAP_TYPE type, COAP_RESPONSE_CODE responseCode, uint8_t *responseOptionBuffer);

    /**
     * Get URI path from packet.
     **/
    String          getUriPath();

    /**
     * Parse Packet from buffer.
     **/
    static bool     parseCoapPacket(CoapPacket &packet, uint8_t *buffer, uint32_t packetLen);
};

#endif /* CoapPacket_h */
