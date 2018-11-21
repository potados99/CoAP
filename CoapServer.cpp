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
    CoapPacket response;
    
    String str;
    const char *load = NULL;
    char optionBuffer[2];
    
    switch (request.code)
case COAP_GET: {
    load = payload;
    response.code = COAP_CONTENT;
    break;
    
case COAP_PUT:
    str = "PUT OK";
    load = str.c_str();
    response.code = COAP_CHANGED;
    break;
    
case COAP_POST:
    str = "Post changed";
    load = str.c_str();
    response.code = COAP_CHANGED;
    
default:
    break;
}
    
    response.payload = (uint8_t *)load;
    response.payloadlen = strlen(load);
    
    optionBuffer[0] = ((uint16_t)COAP_TEXT_PLAIN & 0xFF00) >> 8;
    optionBuffer[1] = ((uint16_t)COAP_TEXT_PLAIN & 0x00FF);
    
    response.options[response.optionnum].buffer = (uint8_t *)optionBuffer;
    response.options[response.optionnum].length = 2;
    response.options[response.optionnum].number = COAP_CONTENT_FORMAT;
    
    response.optionnum ++;
    
    // send packet
    return this->sendPacket(response, ip, port); /* 반환값은 message id */
}

/****************************************************************
 * private
 ****************************************************************/

void CoapServer::packetRecievedBehavior(CoapPacket &packet) {
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
    
    CoapPacket &request = packet;
    CoapPacket response;
    
    /******************* BELOW ARE COPIED *********************/
    
    response.version = request.version;
    
    if (request.code == COAP_EMPTY &&
        request.type == COAP_CON ) {
        response.type = COAP_RESET;
        response.code = COAP_EMPTY_MESSAGE;
        response.messageid = request.messageid;
        response.token = request.token;
        response.payload = NULL;
        response.payloadlen = 0;
        
        sendPacket(response, this->udp->remoteIP(), this->udp->remotePort());
    }

    else if(request.code == COAP_GET ||
            request.code == COAP_PUT ||
            request.code == COAP_POST ||
            request.code == COAP_DELETE) {
        
        if(request.type == COAP_CON) {
            response.type = COAP_ACK;
            response.tokenlen=request.tokenlen;
            response.messageid=request.messageid;
            response.token=request.token;
        }
        else if (request.type == COAP_NONCON) {
             response.type = COAP_NONCON;
            response.tokenlen = request.tokenlen;
            response.messageid = request.messageid;
            response.token = request.token;
        }
        
        if (request.code == COAP_GET) {
            
          
            if(url == String(".well-known/core")) {
                
                resourceDiscovery(response,Udp.remoteIP(),Udp.remotePort(),resource);
                
            }
            else if(! uri.find(url)) {
                
                response.payload = NULL;
                response.payloadlen = 0;
                response.code = COAP_NOT_FOUND;
                
                response.optionnum=0;
                
                char optionBuffer[2];
                optionBuffer[0] = ((uint16_t)COAP_TEXT_PLAIN  & 0xFF00) >> 8;
                optionBuffer[1] = ((uint16_t)COAP_TEXT_PLAIN  & 0x00FF) ;
                response.options[response.optionnum].buffer = (uint8_t *)optionBuffer;
                response.options[response.optionnum].length = 2;
                response.options[response.optionnum].number = COAP_CONTENT_FORMAT;
                response.optionnum ++;
                
                sendPacket(response, this->udp->remoteIP(),this->udp->remotePort());
                
            }
            else {
                
                uri.find(url)(request,this->udp->remoteIP(),Udp->remotePort(),0);
            }
            
        }else if(request.code == COAP_PUT){
            
            if(! uri.find(url)){
                
                response.payload=NULL;
                response.payloadlen=0;
                response.code=COAP_NOT_FOUND;
                
                
                response.optionnum=0;
                
                char optionBuffer[2];
                optionBuffer[0] = ((uint16_t)COAP_TEXT_PLAIN  & 0xFF00) >> 8;
                optionBuffer[1] = ((uint16_t)COAP_TEXT_PLAIN  & 0x00FF) ;
                response.options[response.optionnum].buffer = (uint8_t *)optionBuffer;
                response.options[response.optionnum].length = 2;
                response.options[response.optionnum].number = COAP_CONTENT_FORMAT;
                response.optionnum++;
                
                sendPacket(response,Udp.remoteIP(),Udp.remotePort());
                
            }else{
                uri.find(url)(request,Udp.remoteIP(),Udp.remotePort(),0);
            }
        }else if(request.code==COAP_POST){
            
            int i;
            for( i=0;i<rcount;i++){
                if(resource[i].rt==url){
                    
                    uri.find(url)(request,Udp.remoteIP(),Udp.remotePort(),0);
                    break;
                }
            }
            if(i==rcount){
                //add new resource
                
            }
            
        }else if(request.code==COAP_DELETE){
            
            
            if(!uri.find(url)){
                response.payload=NULL;
                response.payloadlen=0;
                response.code=COAP_NOT_FOUND;
                
                response.optionnum=0;
                
                char optionBuffer[2];
                optionBuffer[0] = ((uint16_t)COAP_TEXT_PLAIN  & 0xFF00) >> 8;
                optionBuffer[1] = ((uint16_t)COAP_TEXT_PLAIN  & 0x00FF) ;
                response.options[response.optionnum].buffer = (uint8_t *)optionBuffer;
                response.options[response.optionnum].length = 2;
                response.options[response.optionnum].number = COAP_CONTENT_FORMAT;
                response.optionnum++;
                
                sendPacket(response,Udp.remoteIP(),Udp.remotePort());
                
            }else{//delete
                
            }
            
        }
    }
    delete request.token;
    
}

//checking for the change for resource
unsigned currentMillis=millis();
if ((unsigned long)(currentMillis-previousMillis)>=interval)
{
    //observing a resouce
    
    uri.find(resource[0].rt)(request,(0,0,0,0),NULL,1);
    previousMillis=millis();
}




}
