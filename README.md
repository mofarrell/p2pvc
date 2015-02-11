# p2pvc
A point to point color terminal video chat.

![demo]
(http://giant.gfycat.com/HideousSpiffyAdder.gif)

[and here's a still image](http://i.imgur.com/ziRPCWE.png)

# Installation

Arch users can install `p2pvc-git` from [the AUR](https://aur.archlinux.org/packages/p2pvc-git/)

### Dependencies

* OpenCV
* PortAudio
* ncurses

#### Ubuntu:

     sudo apt-get install libncurses-dev libopencv-dev portaudio19-dev

#### OS X (with Homebrew):

     brew install ncurses portaudio opencv

#### OS X (with MacPorts):

     sudo port install ncurses portaudio opencv

### Compilation
Make the binary.

    make

Video chat with yourself to test the camera. (Be sure to mute your mic or speakers or you'll get feedback!)

    ./p2pvc 127.0.0.1 -v

# Usage

#### Audio only

    ./p2pvc [ip address]

#### Flags

`-v` enables video chat.

     ./p2pvc [ip address] -v

`-d` sets the dimension of the video in either `[width]x[height]` or `[width]:[height]` format.

     ./p2pvc [ip address] -v -d 200x50

`-A` and `-V` allow you to specify the port the audio and video run on respectively.

     ./p2pvc [ip address] -v -A 1337 -V 1338

`-b` displays incoming bandwidth in the top-right of the video display.
 
     ./p2pvc [ip address] -v -b

`-e` to print stderr (which is by default routed to /dev/null).

     ./p2pvc [ip address] -e
     
`-B` renders in Braille Unicode characters.  Note that the dimensions must both be divisible by 4. 

     ./p2pvc [ip address] -v -B -d 200x152
     
`-r` sets the refresh rate.

     ./p2pvc [ip address] -v -r 10
     
![Demo.]
(http://fat.gfycat.com/WideRecklessChinesecrocodilelizard.gif)
# Known problems and resolutions

#### Black and white

This happens when p2pvc thinks the terminal doesn't have enough colors to display all 256.  Try `export TERM=xterm-256color` or equivalent to get it working.

#### No connection made

p2pvc does not get around NAT, so you may need to port forward.  It uses ports 55555 and 55556 for audio and video respectively.
