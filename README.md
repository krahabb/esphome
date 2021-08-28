# ESPHome [![Discord Chat](https://img.shields.io/discord/429907082951524364.svg)](https://discord.gg/KhAMKrd) [![GitHub release](https://img.shields.io/github/release/esphome/esphome.svg)](https://GitHub.com/esphome/esphome/releases/)


This is a (personal) fork of the awesome [EspHome](https://github.com/esphome) project. It contains a bunch of additional nodes I've developed for my own use and I'm too lazy to push to upstream repo.

In details I've added/refactored this components:
- dallas: the original Dallas component in EspHome needs you to hard-code sensor IDs in configuration so you need to identify (by log or other means) and recompile your node when adding or changing physical sensors. My implementation allows to automatically 'discover' any new physical device attached to the relevant ESP interface line and register it in HA without knowing it in advance.
- scs_bridge: this component talks to SCS bus (by BTicino/Legrand) devices by encoding and decoding the messages exchanged on the bus. This way you can integrate 'legacy' home automation hardware in your HA-EspHome environment and bring some HAwesomeness to this old stuff. The component work is based on the reverse engineered protocol from [GuidoPic](http://guidopic.altervista.org/alter/eibscsgt.html) and needs some dedicated hardware in order to adapt the ESP GPIO to the bus physical layer. Actual implementation exposes basic 'hooks' in order to send/receive raw frames to/from devices and implementation for covers and switches.

See the [wiki](https://github.com/krahabb/esphome/wiki) for details and examples.

**Official Repositiory:** https://github.com/esphome

**Official Documentation:** https://esphome.io/
