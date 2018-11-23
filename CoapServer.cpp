//
//  CoapServer.cpp
//  ESP-CoAP-Test
//
//  Created by POTADOS on 21/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#include "CoapServer.h"

/****************************************************************
 * public
 ****************************************************************/

CoapServer::CoapServer(UDP &udp) : Coap(udp) { }

bool CoapServer::addResource(callback c, String url) {
    if (c && url) { this->uri.add(c, url); }
    else { return false; }
    
    return true;
}

uint16_t CoapServer::sendResponse(CoapPacket &request, IPAddress ip, int port, char *payload) {
    COAP_RESPONSE_CODE respCode;
    COAP_TYPE type;
    
    const char *load = NULL;
    uint8_t optionBuffer[2];
    memset(optionBuffer, 0, sizeof(optionBuffer));
    
    switch (request.type) {
        case COAP_CON:
            type = COAP_ACK;
            break;
            
        case COAP_NONCON:
            type = COAP_NONCON;
            break;
            
        default:
            return 0;
    }
    
    switch (request.code) {
        case COAP_GET:
            load = payload;
            respCode = COAP_CONTENT;
            break;
            
        case COAP_PUT:
            load = String("PUT OK").c_str();
            respCode = COAP_CHANGED;
            break;
            
        default:
            return 0;
    }
    
    CoapPacket response = request.makeResponsePair(type, respCode, optionBuffer);
    
    response.payload = (uint8_t *)load;
    response.payloadlen = strlen(load);
    
    return this->sendPacket(response, ip, port); /* returns message id */
}

/****************************************************************
 * private
 ****************************************************************/

void CoapServer::packetRecievedBehavior(CoapPacket &packet) {
    CoapPacket &request = packet;
    CoapPacket response;
    
    String url = request.getUriPath();
    
    // ping check
    if (request.type == COAP_CON && request.code == COAP_EMPTY) {
        sendEmptyResponse(request, this->udp->remoteIP(), this->udp->remotePort());
    }
    
    // GET, PUT methods
    else if (request.code == COAP_GET || request.code == COAP_PUT) {
        callback foundCallback = uri.find(url);
        
        if (foundCallback) {
            foundCallback(request,this->udp->remoteIP(),this->udp->remotePort());
        }
        else {
            sendResourceNotFoundResponse(request, this->udp->remoteIP(), this->udp->remotePort());
        }
    }
}

uint16_t CoapServer::sendResourceNotFoundResponse(CoapPacket &request, IPAddress ip, int port) {
    COAP_TYPE type;
    uint8_t optionBuffer[2];
    
    switch (request.type) {
        case COAP_CON:
            type = COAP_ACK;
            break;
            
        case COAP_NONCON:
            type = COAP_NONCON;
            break;
            
        default:
            return 0;
    }
    
    CoapPacket response = request.makeResponsePair(type, COAP_NOT_FOUND, optionBuffer);
    
    response.payload = NULL;
    response.payloadlen = 0;
    
    return this->sendPacket(response, ip, port);
}

uint16_t CoapServer::sendEmptyResponse(CoapPacket &request, IPAddress ip, int port) {
    CoapPacket response = request.makeResponsePair(COAP_RESET, COAP_EMPTY_MESSAGE, NULL);
    
    response.payload = NULL;
    response.payloadlen = 0;
    
    return this->sendPacket(response, this->udp->remoteIP(), this->udp->remotePort());
}

