// Wifi 
const char* ssid = "AndroidAP45 8F"; // Wifi SSID  Raptor GN
const char* password = "00448855"; // Wifi Password  r4pt0rTECH

//MQTT Broker
const char* mqtt_server = "192.168.55.131"; //or URL of your hosted environment e.g. demo.openremote.io
unsigned int mqtt_port = 1883; //SSL 8883 NoneSSL 1883
const char* username = "master:mqttuser2"; // Service User Realm:Serviceuser
const char* mqttpass = "djfeNgOclsueI01kwvtcGTY5WWV7gh0H"; // Service User Secret
const char* ClientID = "First_test_Client23"; //this can be anything you like but must be unique
//LastWill
const char* lastwill = "master/First_test_Client/writeattributevalue/AttributeName/AssetID";
const char* lastwillmsg = "0";


//subscribing Topic
const char *topic = "master/First_test_Client2/writeattributevalue/Subscribe_attribute/5vV2VDBMDTX0ETXl28Tq0z"; //see Subscribing Topics in Documentation https://github.com/openremote/openremote/wiki/User-Guide%3A-Manager-APIs#mqtt-api-mqtt-broker


//Local CA

const char* local_root_ca = \
                            "-----BEGIN CERTIFICATE-----\n" \
                            "MIIF/TCCA+WgAwIBAgIUZ/o8/s1N8dM2kn3jqrzMfoF+/SMwDQYJKoZIhvcNAQEL\n" \
                            "BQAwgYwxCzAJBgNVBAYTAk5MMRYwFAYDVQQIDA1Ob3J0aCBCcmFiYW50MRIwEAYD\n" \
                            "VQQHDAlFaW5kaG92ZW4xEzARBgNVBAoMCk9wZW5SZW1vdGUxFjAUBgNVBAMMDW9w\n" \
                            "ZW5yZW1vdGUuaW8xJDAiBgkqhkiG9w0BCQEWFXN1cHBvcnRAb3BlbnJlbW90ZS5p\n" \
                            "bzAgFw0yMTEwMjgxMTA5MjJaGA8yMDcxMDIwODExMDkyMlowgYwxCzAJBgNVBAYT\n" \
                            "Ak5MMRYwFAYDVQQIDA1Ob3J0aCBCcmFiYW50MRIwEAYDVQQHDAlFaW5kaG92ZW4x\n" \
                            "EzARBgNVBAoMCk9wZW5SZW1vdGUxFjAUBgNVBAMMDW9wZW5yZW1vdGUuaW8xJDAi\n" \
                            "BgkqhkiG9w0BCQEWFXN1cHBvcnRAb3BlbnJlbW90ZS5pbzCCAiIwDQYJKoZIhvcN\n" \
                            "AQEBBQADggIPADCCAgoCggIBALYNz5ZWMpFu+2K1kNep34Keh2clknA7Rm3Wke7p\n" \
                            "HoTpjddqHNvQSVjSOB7CZA1E5+a8O7mU6kvnE3wed/h+DO3cQWihbYTw9lNr9+2M\n" \
                            "LS8+jny+lDeuXOMXxcb3og4Cn88mfPihdsUkmI/i7Hk3oSjonyLPycMW/M3d1S6L\n" \
                            "6YkgrdkA3jbMtaR9epcCv/g1P9WBKzF05qIs1lVEzGXOyF2Km2rPAdUcUjFplBbE\n" \
                            "DyUgzbAYsdUTz0PCvAFi1y82/xreWeZQ8OlW1TnqMOR0elVDf/d8An+a3LV/uSaB\n" \
                            "ihmdS4yvxcqZNGU+ShbBO6P+8JjLQNDmPQgjEURCYT4NyHRBChJr31CCvV2QJ2Ik\n" \
                            "Hn5by9/KvvnZ/5y3KjYht9V3ooD0ptxIVKhhwuNFeDSC9Pnqr2QL+mawlWq+Wq/q\n" \
                            "/PMN034gTqLkYAsm4ltoQj/L0ebRdQ0t9L9FML4+fDtEOWAUz3/vRcN1J2y5dNjZ\n" \
                            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
                            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
                            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n" \
                            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx" \
                            "-----END CERTIFICATE-----";