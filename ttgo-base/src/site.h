#include "Secrets/secrets.h"

// Encapsulate Domoticz parameters for the two sites
struct Site
{
   const char* ssid;
   const char* password;
   const char* host;
   unsigned int port;
   int switchIdx;
   int tempIdx;
};

struct Site sites[2] = {
    { Secrets::ssid1, Secrets::password1, "192.168.2.50", 8080, 260, 261},
    { Secrets::ssid2, Secrets::password2, "192.168.1.101", 80, 18, 17},
};