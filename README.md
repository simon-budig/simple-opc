Simple OpenPixelControl client
==============================



This simple client is to be used to feed an OpenPixelControl server
[^1]. Example renderings are provided for an example 8x8x8 cube. The
whole thing started for an installation on the 31st Chaos Communication
Congress (see [^2]).

During the congress we built a first set of animations that are hereby
shared with you. 


== Running

To see the light you have to run an OPC server on your local host, for
example the OpenGL-variant using 

$ gl_server cube-layout.json

and then run any renderer, for example 

$ renderer-all

Obviously it is possible to connect to any OPC server on the internet,
you only have to change the hardcoded destination address.



== The Animations

**TODO**

- astern
- ball
- …



[^1]: http://openpixelcontrol.org/

[^2]: http://gallery.ctdo.de/main.php?g2_itemId=4892
