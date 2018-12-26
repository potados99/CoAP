//
//  CoapUri.hpp
//  ESP-CoAP-Test
//
//  Created by POTADOS on 18/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#ifndef CoapUri_h
#define CoapUri_h

#include "CoapPacket.h"

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
