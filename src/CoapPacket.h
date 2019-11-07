//
//  CoapPacket.h
//  CoAP-simple-library
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
     *
     * @param option                Parsed option data goes here.
     * @param running_delta         Used when processing options.
     * @param buf                   Buffer where data comes from.
     * @param buflen                Length of buffer.
     *
     * @return                      False when failed parsing option.
     */
    bool            parseCoapOptions(CoapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen);

public:
    uint8_t         version = COAP_VERSION; /* currently 1 */
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
     *
     * @param destBuffer            Destination buffer.
     * @param bufferLen             Length of buffer.
     *
     * @return                      Size of packet.
     */
    uint16_t        exportToBuffer(uint8_t *destBuffer, uint32_t bufferLen);

    /**
     * Create a packet for response.
     * The packet contains version, type, code, token(and len), message id, options(and len).
     *
     * @param type                  Type of response packet.
     * @param responseCode          Response code.
     * @param responseOptionBuffer  Option buffer to fill in.
     *
     * @return                      Created packet.
     */
    CoapPacket      makeResponsePair(COAP_TYPE type, COAP_RESPONSE_CODE responseCode, uint8_t *responseOptionBuffer);

    /**
     * Get URI path from packet.
     */
    String          getUriPath();

    /**
     * Parse Packet from buffer.
     *
     * @param buffer                Buffer containing serialized packet.
     * @param packetSize            Size of packet to parse.
     *
     * @return                      False when packet is invalid.
     */
    bool            importFromBuffer(uint8_t *buffer, uint32_t packetSize);
};

#endif /* CoapPacket_h */
