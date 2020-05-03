# Emodem
Simple serial WiFi "modem" for the ESP-32. This was created in order to provide a simple WiFi interface via the user port of a Commodore 64 so that I could connect to BBSes and other such activities.

If you would like to recreate this:
* Connect user port pins B+C to the TX pin of the ESP-32
* Connect user port pin M to the RX pin of the ESP-32
* Connect user port pin 1 to a GND pin of the ESP-32

Then open the user port serial interface in a terminal program on your Commodore 64. Make sure your terminal program is set to the correct baud rate.

## Commands
Commands are sent to the modem  are of the form `EM$<command><argument>`. Note that there is no space between the `EM$`, the command, or the argument.

All successful commands will print `OK` upon completion.

Commands are as follows:
* `EM$test` - displays `OK`, used to verify the serial interface is working
* `EM$ssid<id>` - set the WiFi SSID
* `EM$pass<pass>` - set the WiFi password
* `EM$connect` - connect to the specified SSID with the specified password
* `EM$info` - display info about connection configuration and status
* `EM$address` - display local IP and public IP if connected to WiFi
* `EM$host<host>` - set the TCP host to establish a telnet connection to
* `EM$port<port>` - set the TCP port to establish a telnet connection to
* `EM$tcpconnect` - establish a telnet connection to the specified host and port
* `EM$save` - save the current configuration to the ESP-32's flash memory
* `EM$load` - load a previously saved configuration from flash memory

For convenience, saved configuration is automatically loaded from flash memory at startup.
