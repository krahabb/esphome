# esphome-easun-6048
EspHome component to interface Easun (and the likes: PowMr, MakeSkyBlue) MPPT PV chargers through display port.

![Easun 6048 Controller](docs/easun6048-mppt-controller.webp)

Even though these controllers often implement an RS485 communication port (may be hidden inside the casing), this hardware/software project is not meant to interface that port but, instead, to 'hack' the internal communication line used by the display controller to query and config the main charger controller.
This is done by connecting your ESP to the same bus used for that communication and make it act like a 'sniffer'
since status and config data are continuosly polled by the display controller and so are always transmitted on the bus. If you also want to 'configure' the main board, i.e. send configuration parameters, some special care need to be taken in order to share the TX line with the display controller (more on this) so that the main board could think it is just talking with the 'legal' display controller. A detailed description of the protocol and hardware connection can be found [here](https://github.com/frankB415/easun6048_display_sniffer) (Thanks to @frankB415).

## Hardware
As stated, in order to connect the ESP to the charge controller you have to open the case and 'intercept' the 4-wires cable connecting the main board to the display. Here you'll have:


|MAIN BOARD||ESP||DISPLAY|
|-|-|-|-|-|
|GND[^1]| --------|GND|--------------------------- |GND|
|TX|  --------|RX|--------------------------- |RX|
|RX|  --------|TX|-----(10Kohm resistor)----- |TX|
|3.3| --------|3.3|--------------------------- |3.3|

[^1] I'm currently unable to provide a correct pin ordering/numbering for the 4-wire bus so you'd have to check it yourself. I'll post better infos when I'll be able to re-open the controller case.

Signals are 3.3V so that you can just wire-in your ESP to the bus without any further logic. Be careful when connecting the TX line since you might have both the display board and your ESP UART driving the line with conflicting signals. A resistor at least (as indicated) is needed in order to prevent short-circuiting the two outputs together. Also, in order to allow the display board TX line to correctly drive the bus, you'd have to configure the ESP TX output as `OPEN DRAIN`.

> **Important:**
>
> The ESP8266 hardware UART TX doesn't allow OPEN DRAIN configuration but this component handles the case by automatically 'disabling' the TX pin unless needed for transmission. This way, the display controller can drive the bus without conflicts in normal circumstances (the aforementioned resistor is still needed though). This is achieved by setting the `serial_mode` configuration option to `slave` (see [example configuration](docs/m3_easun6048_example.yaml))
>
> For ESP32, I didn't manage to test it, nor I did provide any 'pin hacking' so the component might work or not. Some experiments/fixes are maybe needed (pls post a PR/Issue if you manage to find a fix)

## How to use
After connecting the hardware as illustrated (in my setup, the 3.3V provided by the bus is enough to power the ESP8266), copy/edit the provided [sample configuration](docs/m3_easun6048_example.yaml). More info about the available options are available inside the sample. The sample configures the full set of entities available, both for reading the state (sensors) and for configuring the controller (same params as available on the display controller).
Some `text_sensors` are available to 'dump' the raw frames as seen on the bus.
As stated I've only tested this component with ESP8266 hardware UART. Nevertheless, ESP8266 software serial should work (with its own caveats) but some care should be taken as usual with special boot pins on your ESP.

At the moment, I'm unable to decode what appear as status data (controller charging phase and other internal state/alarm info). These are now exposed as 'raw' sensors (named 'Offset ...') and you might want to invest some time in trying decode these. I'll try to update the component as soon as I get more historycal data from the controller. Some hints might be extracted from the MODBUS protocol [document](docs/MPPT%20MODBUS%20Protocol%20-%20English%20Version.pdf)

Technically speaking you might want to completely drop the display controller and just use the ESP to interface the main board. This might be possible with this component (setting the `serial_mode` configuration option to `master` so that the component itself issues the polling commands and never 'releases' the TX line). I didn't try this scenario since [@frankB415 states here](https://github.com/frankB415/easun6048_display_sniffer/tree/main?tab=readme-ov-file#paket-injection-to-change-the-settings) that a kind of handshaking/token exchange is happening on the bus between the display and the main board so that, without the display, the communication bus doesn't work.

In my tests, I was able to use the `master` `serial_mode` without any issue but the charger board was already 'bootstrapped' by the display board and I didn't try to power up the whole thing with just the ESP connected.


## Notes
This is a 'replica' of the same component as developed on https://github.com/krahabb/esphome. This repository is just an extraction in order to raise its public status.

## Credits
This project is based on various sources of reverse engineered informations about the connection and protocol decoding of the communication link between main controller <-> display board.
- background: https://diysolarforum.com/threads/easun-and-others-6048-charger-smart-interface.74826/
- working Arduino project: https://github.com/frankB415/easun6048_display_sniffer
