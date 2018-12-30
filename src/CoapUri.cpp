//
//  CoapUri.cpp
//  CoAP-simple-library
//
//  Created by POTADOS on 21/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#include "CoapUri.h"

CoapUri::CoapUri() {
    for (int i = 0; i < MAX_CALLBACK; i++) {
        this->URIs[i] = "";
        this->callbacks[i] = NULL;
    }
}

void CoapUri::add(callback call, String url) {
    for (int i = 0; i < MAX_CALLBACK; i++)
        if (this->callbacks[i] != NULL && this->URIs[i].equals(url)) {
            this->callbacks[i] = call;
            return;
        }
    for (int i = 0; i < MAX_CALLBACK; i++) {
        if (this->callbacks[i] == NULL) {
            this->callbacks[i] = call;
            this->URIs[i] = url;
            return;
        }
    }
}

callback CoapUri::find(String url) {
    for (int i = 0; i < MAX_CALLBACK; i++) {
        if (this->callbacks[i] != NULL && this->URIs[i].equals(url)) {
            return this->callbacks[i];
        }
    }
    return NULL;
}
