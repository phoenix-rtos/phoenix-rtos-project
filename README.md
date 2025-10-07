# Use-Case-3 DEMO running on Phoenix-RTOS

This repository is a part of the COGNIT Energy Use Case Demo, which is described in more details in [use-case-3](https://github.com/SovereignEdgeEU-COGNIT/use-case-3).

This project is a fork of [phoenix-rtos-project](https://github.com/phoenix-rtos/phoenix-rtos-project). Documentation of the Phoenix-RTOS system, together with the building instructions can be found there.

This repository is the demo application that is meant to be run on a Smart Energy Meter (SEM). The application communicates and controls energetically important household devices. The meter controls the mentioned devices according to the result of an AI decision model, which is offloaded to the COGNIT Edge Cluster with the help of the COGNIT device client [device-runtime-c](https://github.com/SovereignEdgeEU-COGNIT/device-runtime-c).

## How does this work?

The application periodically performs a list of routines:

### AI model decision making

* The data containing the state of the household devices is acquired from the simulation process (`use-case-3` scripts) through Modbus protocol,
* The decision function is offloaded to COGNIT using the Device Client,
* The result of the function is obtained,
* The household devices are configured according to the result of the decision function. This is done again through Modbus.

### AI model training

* The steps analogical to the ones related to the decision making,
* AI model is stored outside of the SEM device, by the COGNIT framework.

### Sending metrological data to besmart.energy

* Energy consumption and production is registered in the external `besmart.energy` system,
* The AI model obtains metrological data from the besmart.energy system, after they are appropriately processed therein.

### Updating user preferences

* The user preferences are obtained from the simulation process.
* This includes offloading frequency and possible forced (immediate) offloading requests.

## Building and running on `ia32-generic-qemu` target

### Building

Firstly, consult the documentation of Phoenix-RTOS in order to set up the environment for building and running the project on `ia32-generic-qemu` target. This can be found on [docs.phoenix-rtos.com](https://docs.phoenix-rtos.com/). You are also going to need the [network configuration](https://docs.phoenix-rtos.com/latest/ports/azure_sdk.html).

Finally you will be running the following command, that builds the whole project:

```bash
TARGET=ia32-generic-qemu CONSOLE=serial ./docker-build.sh all
```

After that the emulation can be started:

```bash
./scripts/ia32-generic-qemu-net.sh
```

Now, inside the phoenix console (in qemu), the appropriate shell configuration can be established:

```bash
(psh)% linuxrc
```

### Running

The application is started by calling:

```txt
root@?:~ #  cognit_app -c cognit/config.json -a 192.168.122.1 -p 9999
```

The command line options of the app are:

* `-c` – path to the configuration file,
* `-a` – address of the Modbus servers, set up by the simulation process,
* `-p` – port on which these Modbus servers listen.
* `-s` – path of the serial device connected to the Modbus servers (used when running on an actual target device)

### Configuring

The example of the configuration file can be found in `_user/cognit_app/config/config.json`.
