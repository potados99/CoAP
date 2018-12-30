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

    // add option: host address
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

    // add option: URI
    size_t idx = 0;
    for (size_t i = 0; i < strlen(url); i++) {
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

uint16_t CoapPacket::exportToBuffer(uint8_t *destBuffer, uint32_t bufferLen) {
    uint8_t *bufPtr = destBuffer;
    uint16_t running_delta = 0;
    uint16_t packetSize = 0;

    // make base header
    *bufPtr          = this->version << 6;
    *bufPtr          |= (this->type & 0x03) << 4;
    *bufPtr++        |= (this->tokenlen & 0x0F);
    *bufPtr++        = this->code;
    *bufPtr++        = (this->messageid >> 8);
    *bufPtr++        = (this->messageid & 0xFF);
    bufPtr           = destBuffer + COAP_HEADER_SIZE;
    packetSize       += 4; /* bytes */

    // make token
    if (this->token != NULL && this->tokenlen <= 0x0F) {
        memcpy(bufPtr, this->token, this->tokenlen);
        bufPtr += this->tokenlen;
        packetSize += this->tokenlen;
    }

    // make option header
    for (uint8_t i = 0; i < this->optionnum; ++ i)  {
        uint32_t optdelta = 0;
        uint8_t len = 0;
        uint8_t delta = 0;

        if ((uint32_t)(packetSize + 5 + this->options[i].length) >= bufferLen) { return 0; }

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

    // make payload
    if (this->payload != NULL && this->payloadlen > 0) {
        if ((uint32_t)(packetSize + 1 + this->payloadlen) >= bufferLen) { return 0; }

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
        // iterate through options
        if (this->options[i].number == COAP_URI_PATH && this->options[i].length > 0) {
            // when found URI_PATH,
            // create an array to store that
            char urlname[this->options[i].length + 1];

            // copy from option buffer to urlname array
            memcpy(urlname, this->options[i].buffer, this->options[i].length);

            // last character must be '\0'.
            urlname[this->options[i].length] = '\0';

            // if the found one is the only one, pass this statement
	    // else, add slash
            if(url.length() > 0) {
                url += "/";
            }

            // attach the array to url string
            url += urlname;
        }
    }

    return url;
}

bool CoapPacket::importFromBuffer(uint8_t *buffer, uint32_t packetSize) {
    // E: packet is smaller than header
    if (packetSize < COAP_HEADER_SIZE) { return false; }

    this->version      = (buffer[0] & 0xC0) >> 6;
    this->type         = (buffer[0] & 0x30) >> 4;
    this->tokenlen     = buffer[0] & 0x0F;
    this->code         = buffer[1];
    this->messageid    = 0xFF00 & (buffer[2] << 8);
    this->messageid    |= 0x00FF & buffer[3];

    // E: version does not match
    if (this->version != COAP_VERSION) { return false; }

    // E: token length invalid
    if (this->tokenlen > 8) { return false; }
    // **unsigned 8-bit integer cannot be under zero.

    if (this->tokenlen == 0) {
        this->token = NULL;
    }
    else if (this->tokenlen <= 8) {
        this->token = buffer + COAP_HEADER_SIZE;
    }

    // E: packet size is unexpectedly short
    if ((uint32_t)(COAP_HEADER_SIZE + this->tokenlen) >= packetSize) { return false; }

    // processing options
    int optionIndex = 0;
    uint16_t delta = 0;
    uint8_t *end = buffer + packetSize; /* end of buffer */
    uint8_t *p = buffer + COAP_HEADER_SIZE + this->tokenlen; /* after header and token */

    // number of options is in range &&
    // p is not end &&
    // p is not 255
    while(optionIndex < MAX_OPTION_NUM && *p != 0xFF && p < end) {
        int parseResult = parseCoapOptions(&this->options[optionIndex], &delta, &p, end-p);
        if (parseResult != 0) { return false; }

        optionIndex ++;
    }

    // broke while state because of optionIndex
    if (optionIndex >= MAX_OPTION_NUM) { return false; }

    this->optionnum = optionIndex;

    // p + 1 is not end &&
    // p is 255
    //
    // if survived from while loop and exception handling,
    // there are only two cases, where p faced marker(255) or the packet is end.
    if (p+1 < end && *p == 0xFF) {
        this->payload = p + 1;             /* payload is from p + 1 to end. */
        this->payloadlen = end - (p + 1);
    } else {
        this->payload = NULL;
        this->payloadlen= 0;
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
