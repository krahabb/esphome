# ESPHome [![Discord Chat](https://img.shields.io/discord/429907082951524364.svg)](https://discord.gg/KhAMKrd) [![GitHub release](https://img.shields.io/github/release/esphome/esphome.svg)](https://GitHub.com/esphome/esphome/releases/)


This is a (personal) fork of the awesome [EspHome](https://github.com/esphome) project. It contains a bunch of additional nodes I've developed for my own use and I'm too lazy to push to upstream repo.

In details I've added/refactored this components:
- dallas: the original Dallas component in EspHome needs you to hard-code sensor IDs in configuration so you need to identify (by log or other means) and recompile your node when adding or changing physical sensors. My implementation allows to automatically 'discover' any new physical device attached to the relevant ESP interface line and register it in HA without knowing it in advance.
- scs_bridge: this component talks to SCS bus (by BTicino/Legrand) devices by encoding and decoding the messages exchanged on the bus. This way you can integrate 'legacy' home automation hardware in your HA-EspHome environment and bring some HAwesomeness to this old stuff. The component work is based on the reverse engineered protocol from [GuidoPic](http://guidopic.altervista.org/alter/eibscsgt.html) and needs some dedicated hardware in order to adapt the ESP GPIO to the bus physical layer. Actual implementation exposes basic 'hooks' in order to send/receive raw frames to/from devices and implementation for covers and switches.
- lc_tech: this component tries to implement a robust control implementation for LC-Technologies multi channel board relays (example [aliexpress item](https://it.aliexpress.com/item/1005001399344047.html?spm=a2g0o.productlist.0.0.43973adeWWBtfI&algo_pvid=d6dcedda-0960-446a-9260-951e0a46ceea&algo_exp_id=d6dcedda-0960-446a-9260-951e0a46ceea-12)). These boards have an ESP8266 acting as a wifi bridge between your LAN and a serial controller which actually drives the relays. There are various 'sketch' implementations online and they mostly work but I've decided to pack everything in a well-defined ESPHome component in order to tidy-up the configuration and programming. Also, since the boards themselves are a bit 'lame' (but they're so cheap..in all regards!), I've implemented a logic to repeatedly send the switch state though the internal board serial since their controller often skips the serial commands (and there's no way to know if it worked or not) so I've decided to keep periodically sending the last command. Now, I can wake up in the morning and have no flooding in my lawn (yes: real life!) since I'm using one of these boards to control sprinklers. At any rate: don't buy these, cheap but not really worth it (And they consume like a SUV).

See the [wiki](https://github.com/krahabb/esphome/wiki) for details and examples.

**Official Repositiory:** https://github.com/esphome

**Official Documentation:** https://esphome.io/
