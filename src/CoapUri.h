//
//  CoapUri.h
//  CoAP-simple-library
//
//  Created by POTADOS on 21/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#ifndef CoapUri_h
#define CoapUri_h

#include "CoapPacket.h"

/**
 * Type definition for coap callback.
 */
typedef char * (*callback)(CoapPacket &, IPAddress, int);

class CoapUri {
private:
    String      URIs[MAX_CALLBACK];
    callback    callbacks[MAX_CALLBACK];

public:
    CoapUri();

    void        add(callback call, String url);
    callback    find(String url);
};

#endif /* CoapUri_h */
