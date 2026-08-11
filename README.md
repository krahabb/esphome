# ESPHome [![Discord Chat](https://img.shields.io/discord/429907082951524364.svg)](https://discord.gg/KhAMKrd) [![GitHub release](https://img.shields.io/github/release/esphome/esphome.svg)](https://GitHub.com/esphome/esphome/releases/) [![CodSpeed](https://img.shields.io/endpoint?url=https://codspeed.io/badge.json)](https://codspeed.io/esphome/esphome)

<a href="https://esphome.io/">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://media.esphome.io/logo/logo-text-on-dark.svg">
    <img src="https://media.esphome.io/logo/logo-text-on-light.svg" alt="ESPHome Logo">
  </picture>
</a>

This is a (personal) fork of the awesome [EspHome](https://github.com/esphome) project. It contains a bunch of additional nodes I've developed for my own use and I'm too lazy to push to upstream repo.

Over time, this repository is gaining more and more components and in order to give them better visibility I've decided to clone individual components in their own dedicated repos so you could have a clearer experience when using those. Right now I've 'extracted' these:
- [esphome-victron-vedirect](https://github.com/krahabb/esphome-victron-vedirect): interface to Victron VEDirect communication
- [esphome-victron-ble-ir](https://github.com/krahabb/esphome-victron-ble-ir): interface to Victron instant readout (over BLE) protocol

Those repositories are just published clones of the individual components while development keeps being hosted in this repository.

As for this repository these custom components are available:
- [m3_dallas](https://github.com/krahabb/esphome/blob/krahabb3/esphome/components/m3_dallas): the original Dallas component in EspHome needs you to hard-code sensor IDs in configuration so you need to identify (by log or other means) and recompile your node when adding or changing physical sensors. My implementation allows to automatically 'discover' any new physical device attached to the relevant ESP interface line and register it in HA without knowing it in advance.
- [scs_bridge](https://github.com/krahabb/esphome/blob/krahabb3/esphome/components/scs_bridge): this component talks to SCS bus (by BTicino/Legrand) devices by encoding and decoding the messages exchanged on the bus. This way you can integrate 'legacy' home automation hardware in your HA-EspHome environment and bring some HAwesomeness to this old stuff. The component work is based on the reverse engineered protocol from [GuidoPic](http://guidopic.altervista.org/alter/eibscsgt.html) and needs some dedicated hardware in order to adapt the ESP GPIO to the bus physical layer. Actual implementation exposes basic 'hooks' in order to send/receive raw frames to/from devices and implementation for covers and switches. Sample configuration in the [wiki](https://github.com/krahabb/esphome/wiki/scs_bridge).
- [m3_lc_tech](https://github.com/krahabb/esphome/blob/krahabb3/esphome/components/m3_lc_tech): this component tries to implement a robust control implementation for LC-Technologies multi channel board relays (example [aliexpress item](https://it.aliexpress.com/item/1005001399344047.html?spm=a2g0o.productlist.0.0.43973adeWWBtfI&algo_pvid=d6dcedda-0960-446a-9260-951e0a46ceea&algo_exp_id=d6dcedda-0960-446a-9260-951e0a46ceea-12)). These boards have an ESP8266 acting as a wifi bridge between your LAN and a serial controller which actually drives the relays. There are various 'sketch' implementations online and they mostly work but I've decided to pack everything in a well-defined ESPHome component in order to tidy-up the configuration and programming. Also, since the boards themselves are a bit 'lame' (but they're so cheap..in all regards!), I've implemented a logic to repeatedly send the switch state though the internal board serial since their controller often skips the serial commands (and there's no way to know if it worked or not) so I've decided to keep periodically sending the last command. Now, I can wake up in the morning and have no flooding in my lawn (yes: real life!) since I'm using one of these boards to control sprinklers. At any rate: don't buy these, cheap but not really worth it (And they consume like a SUV).
- [m3_antbms](https://github.com/krahabb/esphome/blob/krahabb3/esphome/components/m3_antbms): gateway component to interface your ANTBMS-like BMS with HA. It connects directly to the BMS serial (the small 4 pin connector carrying GND - +5V - TXD - RXD) thus not using the BT radio for the matter. Component is actually working even if I'm planning some improvements here and there but documentation is way behind..I'll try come up with something.

See the [wiki](https://github.com/krahabb/esphome/wiki) for details and examples.

**Official Repositiory:** https://github.com/esphome

**Official Documentation:** https://esphome.io/
