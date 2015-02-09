# p2pvc
A point to point color terminal video chat.
![demo]
(http://i.imgur.com/ziRPCWE.png)
[and here's a video](http://gfycat.com/HideousSpiffyAdder)

# Compilation
Make the binary.

    make

Video chat with yourself to test the camera.

    ./p2pvc 127.0.0.1 -v

### Dependencies

* OpenCV
* PulseAudio
* ncurses

If you are running Ubuntu: `sudo apt-get install libncurses-dev libopencv-dev libpulse-dev`
