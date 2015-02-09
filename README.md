# p2pvc
A point to point color terminal video chat.

![demo]
(http://giant.gfycat.com/HideousSpiffyAdder.gif)

[and here's a still image](http://i.imgur.com/ziRPCWE.png)

# Installation

Arch users can install `p2pvc-git` from [the AUR](https://aur.archlinux.org/packages/p2pvc-git/)

### Build from source
Make the binary.

    make

Video chat with yourself to test the camera. (Be sure to mute your mic or speakers or you'll get feedback!)

    ./p2pvc 127.0.0.1 -v

### Dependencies

* OpenCV
* PulseAudio
* ncurses

If you are running Ubuntu: `sudo apt-get install libncurses-dev libopencv-dev libpulse-dev`

### Known problems and resolutions

#### Black and white

This happens when p2pvc thinks the terminal doesn't have enough colors to display all 256.  Try `export TERM=xterm-color256` or equivalent to get it working.

#### No connection made

p2pvc does not get around NAT, so you may need to port forward.  It uses ports 55555 and 55556 for audio and video respectively.
