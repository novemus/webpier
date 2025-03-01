# README

This repository is dedicated to the [WebPier](https://github.com/novemus/webpier) application. This program is still in development and not all the planned features has been implemented yet. But it is already in a functional state and can be used, among other things, for testing purposes.

The [WebPier](https://github.com/novemus/webpier) app is designed to exchange TCP services with remote hosts, including those located behind the NAT. Peers do not need to have public IP addresses and use third-party relay servers. The 'WebPier' creates a direct UDP tunnel between the hosts and maps the remote TCP service to the local interface, or forwadrs the local TCP service to the remote side. This program is essentially a graphical shell for the [plexus](https://github.com/novemus/plexus) and [wormhole](https://github.com/novemus/wormhole) utilities and is designed to provide a user-friendly mean for managing the export/import of TCP services.

## Using

The easiest way to get started is to download and install the prebuild [package](https://github.com/novemus/webpier/releases) for Debian or the MSI [installer](https://github.com/novemus/webpier/releases) for Windows platforms.

On the first start, you will be prompted to define the local *pier* identity. It consists from two parts. The first one is an owner identifier, which should be your email address if you want to use the email as the rendezvous service. It can be any word in case of using DHT network as the rendezvous, but using email name is more unified approach. In addition, this is a good, human-readable way to distinguish participants and avoid naming collisions. The second part of the identifier is the name of the local *pier* which must be unique for the owner. After providing the identifier, a pair of cryptographic keys will be generated. They will be used to protect rendezvous messages. Then you have to check and, if necessary, change some of parameters by the settings dialog from the "File -> Settings..." menu. Accessible STUN server with DHT bootstarp server or your email account must be specified. After that you have to exchange offers with participants. To create your *offer* and accept the same from someone, use the dialogs from the "File -> Create an offer..." and "File -> Upload an offer...". The *offer* will consist the *pier* public key and suggested services. Also the service forwarding can be configured manually in the relevant tables:

* **Name** - a name to represent the service forwarding
* **Pier** - remote piers to export to or a remote pier to import from the service
* **Address** - IPv4 address of the exporting service or local IPv4 address to import the remote service in form XXX.XXX.XXX.XXX:PORT
* **Gateway** - local IPv4 address for the transport UDP tunnel in form XXX.XXX.XXX.XXX:PORT
* **Autostart** - should the service forwarding be run with the application startup or manually
* **Obscure** - should the transport UDP tunnel be obfuscated, must be equal for both sides
* **Rendezvous** - selector of the preffered randezvous, must match the remote side

You can manage the status of services from the context menu or from the desktop tray.

## Build

To build the project from the source you will need to provide the following dependencies: [plexus](https://github.com/novemus/plexus), [wormhole](https://github.com/novemus/wormhole), [wxWidgets](https://github.com/wxWidgets/wxWidgets), [openssl](https://github.com/openssl/openssl), [boost](https://github.com/boostorg/boost) libraries and their interface dependencies. For example, `plexus` library also needs the [tubus](https://github.com/novemus/tubus) and [opendht](https://github.com/savoirfairelinux/opendht) library with their own interface dependensies. If that doesn't scare you then try to build the project.

The Linux CMake presets assume that the dependencies are accessible via CMAKE_PREFIX_PATH and PKG_CONFIG_PATH variables or located in `install/x64-static` or `install/x64` (depending on the build preset) directories next to the project source folder:

```console
$ git clone https://github.com/novemus/webpier.git
$ cd webpier
$ cmake --preset=linux-static-release
$ cpack --preset=debian-package
```

The Windows CMake presets rely on the `vcpkg` packet manager and its environment to resolve the dependencies. You can install missing ones to the `install/x64-static` or `install/x64` (depending on the build preset) directory next to the project source folder:

```console
$ git clone https://github.com/novemus/webpier.git
$ set VCPKG_ROOT=path/to/vcpkg/root
$ cd webpier
$ cmake --preset=windows-static-release
$ cpack --preset=windows-msi
```

## Bugs and improvements

Feel free to [report](https://github.com/novemus/webpier/issues) bugs and [suggest](https://github.com/webpier/plexus/issues) improvements. 

## License

Webpier is licensed under the Apache License 2.0, which means that you are free to get and use it for commercial and non-commercial purposes as long as you fulfill its conditions. See the LICENSE.txt file for more details.

## Copyright

Copyright © 2025 Novemus Band. All Rights Reserved.
