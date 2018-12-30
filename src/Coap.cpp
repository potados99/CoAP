//
//  Coap.cpp
//  CoAP-simple-library
//
//  Created by POTADOS on 21/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#include "Coap.h"

/****************************************************************
 * public
 ***************************************************************/

Coap::Coap(UDP& udp) {
    this->udp = &udp;
}

void Coap::start(int port) {
    this->udp->begin(port);
    this->started = true;
}

bool Coap::loop() {
    if (! started) { return false; }

    uint8_t     buffer[BUF_MAX_SIZE];   /* buffer to store udp packet */
    CoapPacket  packet;                 /* coap packet */
    int32_t     packetSize;             /* udp packet size */

    if ((packetSize = this->udp->parsePacket())) {          /* when something is available */
        if (packetSize > BUF_MAX_SIZE) { return false; }    /* E: too big */

        memset(buffer, 0, sizeof(buffer));                  /* clear buffer */
        this->udp->read(buffer, packetSize);                /* read */

        // PARSE!
        bool success = packet.importFromBuffer(buffer, packetSize);
        if (! success) { return false; }                    /* E: packet invalid */

        // packet recieved behavior
        this->packetRecievedBehavior(packet);

        return true;                                        /* all done well */
    }

    return true;                                            /* nothing to parse */
}


/****************************************************************
 * private
 ***************************************************************/

uint16_t Coap::sendPacket(CoapPacket &packet, IPAddress ip, int port) {
    uint8_t buffer[BUF_MAX_SIZE];
    memset(buffer, 0, sizeof(buffer));

    uint16_t packetSize = packet.exportToBuffer(buffer, BUF_MAX_SIZE);

    this->udp->beginPacket(ip, port);
    this->udp->write(buffer, packetSize);
    this->udp->endPacket();

    return packet.messageid;
}
