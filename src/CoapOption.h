//
//  CoapOption.h
//  CoAP-simple-library
//
//  Created by POTADOS on 21/11/2018.
//  Copyright © 2018 POTADOS. All rights reserved.
//

#ifndef CoapOption_h
#define CoapOption_h

#include "includes.h"

class CoapOption {
public:
    /**
     * Number of this option.
     */
    uint8_t number;
    
    /**
     * Length of this option.
     */
    uint8_t length;
    
    /**
     * Content of this option.
     */
    uint8_t *buffer;
};

#endif /* CoapOption_h */
