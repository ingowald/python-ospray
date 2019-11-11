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
    color =  [
        0.9, 0.5, 0.5, 1.0,
        0.8, 0.8, 0.8, 1.0,
        0.8, 0.8, 0.8, 1.0,
        0.5, 0.9, 0.5, 1.0
        ]
    index = [
        0, 1, 2,
        1, 2, 3
        ]

    ## initialize OSPRay; OSPRay parses (and removes) its commandline
    ## parameters, e.g. "--osp:debug"
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
    #TODO: data = ospNewData(4, OSP_FLOAT3A, vertex, 0)
    data = ospNewData(4, 'float3a', vertex)
    ## OSP_FLOAT3 format is also supported for vertex positions
    ospCommit(data)
    ospSetData(mesh, "vertex", data)
    ospRelease(data) ##we are done using this handle
    
    data = ospNewData(4, 'float4', color)
    ospCommit(data)
    ospSetData(mesh, "vertex.color", data)
    ospRelease(data) ## we are done using this handle

    data = ospNewData(2, 'int3', index)
    ## OSP_INT4 format is also supported for triangle indices
    ospCommit(data)
    ospSetData(mesh, "index", data)
    ospRelease(data) ## we are done using this handle

    ospCommit(mesh)
    
    world = ospNewModel()
    ospAddGeometry(world, mesh)
    ospRelease(mesh) ## we are done using this handle
    ospCommit(world)

    ## create renderer
    renderer = ospNewRenderer("scivis")
    ## choose Scientific Visualization renderer

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
    print("rendering first frame")
    ospRenderFrame(framebuffer, renderer, ["color","accum"])
    
    ## access framebuffer and write its content as PPM file
    ##const uint32_t * fb = (uint32_t*)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR)
    print("saving frame buffer to 'firstFrame.png'")
    ospFrameBufferSave("firstFrame.png", framebuffer, imgSize, "srgba")
    ##ospUnmapFrameBuffer(fb, framebuffer)

    ## render 10 more frames, which are accumulated to result in a better converged image
    for frame in range(0,10) :
        print("accumulating frame #"+str(frame))
        ospRenderFrame(framebuffer, renderer, ["color","accum"])

    ##fb = (uint32_t*)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR)
    ##writePPM("accumulatedFrame.ppm", &imgSize, fb)
    ##ospUnmapFrameBuffer(fb, framebuffer)
    print("saving accumulated frame buffer to 'accumulatedFrame.png'")
    ospFrameBufferSave("accumulatedFrame.png", framebuffer, imgSize, "srgba")

    ## final cleanups
    ospRelease(renderer)
    ospRelease(camera)
    ospRelease(lights)
    ospRelease(light)
    ospRelease(framebuffer)
    ospRelease(world)

    ospShutdown()


    
main()
