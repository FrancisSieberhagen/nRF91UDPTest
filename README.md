# nRF91UDPTest

## Test BSD library - NB-IoT UDP client send JSON data to server (server/udp_server)

### nRF Connect SDK!
    https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/index.html

    Installing the nRF Connect SDK through nRF Connect for Desktop
    https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_assistant.html

### Nordicsemi nRF9160 NB-IoT 
    https://www.nordicsemi.com/Software-and-tools/Development-Kits/nRF9160-DK

### CLion - A cross-platform IDE for C
    https://devzone.nordicsemi.com/f/nordic-q-a/49730/howto-nrf9160-development-with-clion

### Application Description
    JSON Data packet {"ActionName":"BSD Test","LED1":false,"LED2":true}

    Client: Send JSON packet to server
    Server: Send JSON packet to client

    Client: Validate ActionName - "ActionName":"BSD Test"
    Client: Toggle nRF9160-DK leds - "LED1":false,"LED2":true

### Change Test Server ip & port in prj.conf  
    CONFIG_SERVER_HOST="139.162.163.251"
    CONFIG_SERVER_PORT=42511

### Build hex 
    $ export ZEPHYR_BASE=/????
    $ west build -b nrf9160_pca10090ns

### Program nRF9160-DK using nrfjprog
    $ nrfjprog --program build/zephyr/merged.hex -f nrf91 --chiperase --reset --verify

### Build server
    $ gcc udp_server.c -o udp_server


### nRF Connect
![alt text](https://raw.githubusercontent.com/FrancisSieberhagen/nRF91UDPTest/master/images/nRFConnect.jpg)


### Server
![alt text](https://raw.githubusercontent.com/FrancisSieberhagen/nRF91UDPTest/master/images/server.jpg)



