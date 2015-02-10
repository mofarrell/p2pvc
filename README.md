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
If you are running OSX, try the `portaudio` branch, and install the additional dependency PortAudio.

# Usage

#### Audio only

    ./p2pvc [ip address]

#### Flags

`-v` flag enables video chat.

     ./p2pvc [ip address] -v

`-d` flag allows you to specify the dimension of the video in either `[width]x[height]` or `[width]:[height]` format. Note that both users should specify the same dimension.

     ./p2pvc [ip address] -v -d 200x50

`-A` and `-V` flags allow you to specify the port the audio and video run on respectively.

     ./p2pvc [ip address] -v -A 1337 -V 1338

`-b` flag displays incoming bandwidth in the top-right of the video display
 
     ./p2pvc [ip address] -v -b

# Known problems and resolutions

#### Black and white

This happens when p2pvc thinks the terminal doesn't have enough colors to display all 256.  Try `export TERM=xterm-256color` or equivalent to get it working.

#### No connection made

p2pvc does not get around NAT, so you may need to port forward.  It uses ports 55555 and 55556 for audio and video respectively.
