<p align="center">
<img src="assets/CobraLogo.png" height="80%" width="80%" alt="libcobra" />
</p>

<p align="center">
<a href="https://discord.gg/VHVPWDmnKB"><img src="https://img.shields.io/discord/454623440729538571?color=%23d04c63&label=Discord&style=for-the-badge" alt="discord"></a>
</p>

<br>
<br>

<p align="center">
Fast and simple data live-transfer protocol between 2 participants in a connection
</p>

<br>

## Note
This version is about to be deprecated

See: https://github.com/sudox-team/cobra-rs

## Features

* Collecting data in packets instead of a byte stream
* Searching devices in a LAN that supports the libcobra protocol
* Maintaining a connection using a duplex ping technology
* Fully thread-safe

## Roadmap to v1.0
In active development, but does not yet have release date.

#### Encryption
Libcobra must be a safe transport that's why we need encryption support. 
Soon, libcobra will run TLS 1.3 but finally, we will have our encryption based on **libsodium** library.

#### UDP Channel
We want libcobra to be suitable everywhere, that's why we will implement the second channel based on UDP protocol. 
This will be useful in games or audio/video streaming.

#### Extension system
Libcobra allows you to send raw bytes, but usually, developers use different schemes to exchange data (for example API scheme over JSON or MsgPack).
We want to provide an easy way to create add-ons over libcobra.

#### Multi-language support
Now libcobra supports C as main language and Java (with Android), Dart (only Flutter) via bindings. 
We want these languages/frameworks to be supported: C++, Go, Swift, Node.js, Python.

#### Stability guarantees
After reaching version 1.0, the API will be backward compatible. 

## Contributing
We look forward to any of your contributions to the project. 
Feel free to ask any questions and open pull requests.

