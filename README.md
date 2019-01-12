# libpcoap

<a href="http://coap.technology/" target=_blank>CoAP</a> implementation for Arduino and ESP8266.

The initial codes was forked from <a href="http://github.com/hirotakaster/CoAP-simple-library" target=_blank>hirotakaster/CoAP-simple-library</a>.

## Source Code
This library has two headers for use: 

- **CoapServer.h**
- **CoapClient.h**.

Being both server and client is neither tested nor recommanded. 

## Implementation

This library implements only few parts of CoAP.

`GET` and `PUT` methods are supported.

*Observe* and *resource discovery* is not suppprted yet.

## How To Use

Clone <a href="http://github.com/potados99/libpcoap" target=_blank>this repository</a> to library path of arduino IDE.

    cd $HOME/Documents/Arduino/libraries
    git clone http://github.com/potados99/libpcoap
   
## Example

Here is CoAP server example on ESP8266.

~~~C
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <CoapServer.h>

WiFiUDP udp;
CoapServer server(udp);

// WiFi information.
const char *SSID = "****";
const char *PASSWORD = "****";

// Callback defines actions when data is recieved. 
char *myCallback(CoapPacket &packet, IPAddress ip, int port);

void setup() {
  WiFi.begin(SSID, PASSWORD);
  
  // Add resource with callback and resource name.
  server.addResource(myCallback, "device");
  server.start();
}

void loop() {
  server.loop();
}

char *myCallback(CoapPacket &packet, IPAddress ip, int port) {
  const char * msg = (const char *)packet.payload ? (const char *)packet.payload : "NULL";
  
  // This string MUST be allocated dynamically. It will be freed after in loop.
  char *reply = (char *)malloc(32); 
  memset(reply, 0, 32);

  sprintf(reply, "Recieved payload: [%s]", msg);
  
  // After return, reply is sent automatically.
  return reply;
}
~~~
