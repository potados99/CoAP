//
//  CoapUri.cpp
//  ESP-CoAP-Test
//
//  Created by POTADOS on 18/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#include "CoapUri.h"

CoapUri::CoapUri() {
    for (int i = 0; i < MAX_CALLBACK; i++) {
        URIs[i] = "";
        callbacks[i] = NULL;
    }
}

void CoapUri::add(callback call, String url) {
    for (int i = 0; i < MAX_CALLBACK; i++)
        if (callbacks[i] != NULL && URIs[i].equals(url)) {
            callbacks[i] = call;
            return;
        }
    for (int i = 0; i < MAX_CALLBACK; i++) {
        if (callbacks[i] == NULL) {
            callbacks[i] = call;
            URIs[i] = url;
            return;
        }
    }
}

callback CoapUri::find(String url) {
    for (int i = 0; i < MAX_CALLBACK; i++) {
        if (callbacks[i] != NULL && URIs[i].equals(url)) {
            return callbacks[i];
        }
    }
    return NULL;
}
