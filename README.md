PySPRay / python-ospray - Python-bindings for the OSPRay API
============================================================

This repo captures some python-bindings for the ospray API. Its goal
was not so much to have such a plugin in the first place, as it was
about learning python, and about experimenting with how python
bindings actually work (and once doing so, why not do something
potentially useful!?).

It is work in progress (primarily to experiment with writing python
interfaces, and to develop/test my own sandbox GPU implementation of
that API), but also works with the public CPU version of ospray.

This being work in progress a lot of functions are still missing
(right now only a python-variant of the very first ospray tutorial is
working), but it should be rather straightforward to add missing API
functions by copy-n-pasting from the existing ones (if you do, don't
forget to send a pull request so I can merge back!).

A simply example of using this API from python is given in
"examples/ospTutorial.py" - it's pretty much a one-to-one python
version of the original ospTutorial.c tutorial sample that comes with
ospray, with the main difference that I currently just render an image
to disk rather than returning it to python as a "list" of pixels. If
anybody does have a usecase for that functionality of returning actual
pixels to python, please let me know!

And of course - any feedback is welcome!

Building and Installing
-----------------------

This library assumes that you have a working install of ospray on your
system; preferably installed in /usr/local/ (if you use a different directory, you have to edit the paths in `src/setup.py`.

Assuming you have that, you can install with python 2.7's plugin setup
mechanism, as simple as

``` bash
   cd src
   python setup.py build
   python setup.py install
```


A Simple Example of *Using* these bindings
------------------------------------------

Note this example is a literal - but abbreviated - copy of the samples/ospTutorial.py example:

``` python
#!/usr/bin/python

import ospray
from ospray import *

def main() :
    imgSize = [1024,768]

    ## camera
    cam_pos  = [0,   0, 0]
    cam_up   = [0,   1, 0]
    cam_view = [0.1, 0, 1]

    ## triangle mesh data
    vertex = [
        -1.0, -1.0, 3.0, 0,
        -1.0,  1.0, 3.0, 0,
        1.0, -1.0, 3.0, 0,
        0.1,  0.1, 0.3, 0
        ]
    color = [ ... ]
    index = [ ... ]

    ## initialize OSPRay
    ospray.ospInit()
    
    ## create and setup camera
    camera = ospNewCamera("perspective")
    ospSetf(camera, "aspect", imgSize[0]/imgSize[1])
    ospSet3fv(camera, "pos", cam_pos);
    ospSet3fv(camera, "dir", cam_view);
    ospSet3fv(camera, "up",  cam_up);
    ## commit each object to indicate modifications are done
    ospCommit(camera) 

    ## create and setup model and mesh
    mesh = ospNewGeometry("triangles")
    data = ospNewData(4, 'float3a', vertex)
    ospCommit(data)
    ospSetData(mesh, "vertex", data)
    ospRelease(data) ##we are done using this handle
    
    data = ospNewData(4, 'float4', color)
	...
    data = ospNewData(2, 'int3', index)
	...
    ospCommit(mesh)
    
    world = ospNewModel()
    ospAddGeometry(world, mesh)
    ospCommit(world)

    ## choose Scientific Visualization renderer
    renderer = ospNewRenderer("scivis")

    ## create and setup light for Ambient Occlusion
    light = ospNewLight("ambient")
    ospCommit(light)
    lights = ospNewData(1, 'OSP_LIGHT', [ light ])
    ospCommit(lights)

    ## complete setup of renderer
    ospSet1i(renderer, "aoSamples", 1)
    ospSet1f(renderer, "bgColor", 1.0) ## white, transparent
    ospSetObject(renderer, "model",  world)
    ospSetObject(renderer, "camera", camera)
    ospSetObject(renderer, "lights", lights)
    ospCommit(renderer)
    
    ## create and setup framebuffer
    framebuffer = ospNewFrameBuffer(imgSize, "srgba", [ "color", "accum" ])
    ospFrameBufferClear(framebuffer, [ "color", "accum" ]);

    ## render one frame
    ospRenderFrame(framebuffer, renderer, ["color","accum"])
    ospFrameBufferSave("firstFrame.ppm", framebuffer, imgSize, "srgba")
	...
    ## final cleanups
    ospRelease(renderer)
	...
    ospShutdown()
    
main()
```
Once such a python file exists, you should be able to just run it with `python ospTutorial.py`.

