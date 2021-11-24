# WiFi_TemperatureMonitor_Project
ADC and serial communication project for temperature monitoring.

The serial data format used is the default one: 115200N81 (115200 baud rate, no parity, 8 data bits, and 1 stop bit).
The clock used is SMCLK in its default configuration, which, after PUC (power up clear), is configured to use the DCOCLKDIV.
The FLL stabalized frequency of SMCLK is 1.048576 MHZ.
