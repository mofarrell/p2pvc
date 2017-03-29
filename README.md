# p2pvc
A point to point color terminal video chat.

![Demo](http://giant.gfycat.com/HideousSpiffyAdder.gif)

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

     brew tap homebrew/science
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

`-d` sets the dimension of the video in `[width]x[height]` format.

     ./p2pvc [ip address] -v -d 200x50

`-A` and `-V` allow you to specify the port the audio and video run on respectively.

     ./p2pvc [ip address] -v -A 1337 -V 1338

`-b` displays incoming bandwidth in the top-right of the video display.
 
     ./p2pvc [ip address] -v -b

`-e` to print stderr (which is by default routed to /dev/null).

     ./p2pvc [ip address] -e
     
`-B` renders in Braille Unicode characters.  Note that the dimensions must both be divisible by 4. 

     ./p2pvc [ip address] -v -B -d 200x152

`-I` sets the threshold for turning pixels on (when using the `-B` flag).  Ranges from 1 - 99, defaults 25.

    ./p2pvc [ip address] -v -B -I 50

`-E` sets and edge filter with `[lower]:[upper]` bounds.

    ./p2pvc [ip address] -v -B -E 100:300

`-c` sets the color of the video.  Used in the form `[r]:[g]:[b]`.  Each color ranges from 0 - 100.

     ./p2pvc [ip address] -v -c 0:100:0

`-s` sets the saturation of the colors in the video.  0.0 is greyscale, 2.0 is default.

     ./p2pvc [ip address] -v -s 3.0
     
`-a` sets custom ASCII character maps.  Repeat characters to weight their frequency.

     ./p2pvc [ip address] -v -a " ......#####"
     
`-r` sets the refresh rate.

     ./p2pvc [ip address] -v -r 10
     
![Demo](http://fat.gfycat.com/WideRecklessChinesecrocodilelizard.gif)
# Known problems and resolutions

#### Black and white

This happens when p2pvc thinks the terminal doesn't have enough colors to display all 256.  Try `export TERM=xterm-256color` or equivalent to get it working.

#### No connection made

p2pvc does not get around NAT, so you may need to port forward.  It uses ports 55555 and 55556 for audio and video respectively.
