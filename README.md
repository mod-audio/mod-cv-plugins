# MOD-CV-Plugins

This repository contains the source code for the `mod-cv-plugins`

##### MOD-CV-Plugins:
* mod-cv-attenuverter
* mod-cv-clock
* mod-cv-control
* mod-cv-meter
* mod-cv-switch1
* mod-cv-switch2
* mod-cv-switch3
* mod-cv-switch4
* mod-cv-to-audio
* mod-midi-to-cv-mono
* mod-midi-to-cv-poly
* mod-cv-abs
* mod-cv-gate
* mod-cv-random
* mod-cv-range
* mod-cv-round
* mod-cv-slew
* mod-logic-operators
* mod-audio-to-cv(beta)
* mod-cv-change(beta)

### Building and installation

The logic operators plugin uses [DPF](https://github.com/DISTRHO/DPF).
This is included in a submodule, therefore this needs to be enabled before this plugin can be build. To enabled this, run:
```
$ git submodule init
$ git submodule update
```

Then all plugins can be build by running:
```
$ make
```

To install all plugins run:
```
$ make install
```

To build an individual plugin, do:
```
$ cd source/<plugin-directory>
$ make
```
