## Introduction

This is a port of [Monkey's Audio Codec][monkeys-audio] to Unix-like systems. It provides a command line utility and a library that can be used by other programs.

Monkey's Audio Codec is a lossless compressor for audio files. Files in this format can typically be recognized by the *.ape* extension. Other popular lossless compression formats are Xiph.Org's [FLAC][] and Apple's [ALAC][].

[monkeys-audio]: http://www.monkeysaudio.com/
[flac]: http://flac.sourceforge.net/
[alac]: http://alac.macosforge.org/

## Usage

To encode an audio file:

```sh
$ mac input.wav output.ape -c2000
```

To decode an audio file:

```sh
$ mac input.ape output.wav -d
```

See `mac(1)` for more information.

## Installing

The easiest way to install Monkey's Audio Codec is through [Homebrew][]. There is a formula for Monkey's Audio Codec in [my Homebrew tap][tap].

[homebrew]: http://mxcl.github.com/homebrew/
[tap]: https://github.com/fernandotcl/homebrew-fernandotcl

If you're compiling from source, you'll need:

* [CMake][]
* [Yasm][] (optional)

[cmake]: http://www.cmake.org/
[yasm]: http://yasm.tortall.net/

To compile and install:

```sh
cd /path/to/source
cmake .
make install
```

## Credits

Monkey's Audio Codec was created by [Matthew T. Ashland][matthew]. It was ported to Linux and further developed by Frank Klemm and [SuperMMX][]. This port is now being maintained by [Fernando Tarlá Cardoso Lemos][fernando].

[matthew]: mailto:email@monkeysaudio.com
[supermmx]: mailto:SuperMMX@gmail.com
[fernando]: mailto:fernandotcl@gmail.com

## License

Monkey's Audio Codec is available under an [unorthodox license][license-trouble]. See the LICENSE file for more information.

[license-trouble]: http://lists.debian.org/debian-legal/2007/09/msg00079.html
