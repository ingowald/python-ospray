// ======================================================================== //
// Copyright 2019 Ingo Wald                                                 //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include <Python.h>
#include "ospray/ospray.h"
#include <iostream>
#include <vector>

/* static PyObject *SpamError; */



// helper function to write the rendered image as PPM file
void writePPM(const char *fileName,
              const osp::vec2i &size,
              const uint32_t *pixel)
{
  FILE *file = fopen(fileName, "wb");
  if (!file) {
    fprintf(stderr, "fopen('%s', 'wb') failed: %d", fileName, errno);
    return;
  }
  fprintf(file, "P6\n%i %i\n255\n", size.x, size.y);
  unsigned char *out = (unsigned char *)alloca(3*size.x);
  for (int y = 0; y < size.y; y++) {
    const unsigned char *in = (const unsigned char *)&pixel[(size.y-1-y)*size.x];
    for (int x = 0; x < size.x; x++) {
      out[3*x + 0] = in[4*x + 0];
      out[3*x + 1] = in[4*x + 1];
      out[3*x + 2] = in[4*x + 2];
    }
    fwrite(out, 3*size.x, sizeof(char), file);
  }
  fprintf(file, "\n");
  fclose(file);
}




// ##################################################################
// helper functions
// ##################################################################

OSPFrameBufferFormat parseFrameBufferFormat(const std::string &format)
{
  if (format == "srgba") return OSP_FB_SRGBA;
  throw std::runtime_error("unkown frame buffer format '"+format+"'");
}

/*! given a list iterator elemnt (that is suppsoed to point to a
    number object), convert that to float, if possible */
std::string getString(PyObject *iter)
{
  if (PyString_Check(iter)) 
    return PyString_AsString(iter);
  throw std::runtime_error("argument is not a string or string-compatible type ...!?");
}

/*! given a list iterator elemnt (that is suppsoed to point to a
    number object), convert that to float, if possible */
float getFloat(PyObject *iter)
{
  if (PyFloat_Check(iter)) 
    return (float)PyFloat_AsDouble(iter);
  if (PyLong_Check(iter)) 
    return (float)PyLong_AsLong(iter);
  if (PyInt_Check(iter)) 
    return (float)PyInt_AsLong(iter);
  throw std::runtime_error("argument is not a float or float-compatible type ...!?");
}

/*! given a list iterator elemnt (that is suppsoed to point to a
    number object), convert that to float, if possible */
int getInt(PyObject *iter)
{
  if (PyFloat_Check(iter)) 
    return (int)PyFloat_AsDouble(iter);
  if (PyLong_Check(iter)) 
    return (int)PyLong_AsLong(iter);
  if (PyInt_Check(iter)) 
    return (int)PyInt_AsLong(iter);
  throw std::runtime_error("argument is not a int or int-compatible type ...!?");
}

/*! given a list iterator elemnt (that is suppsoed to point to a
    number object), convert that to float, if possible */
long getLong(PyObject *iter)
{
  if (PyLong_Check(iter)) 
    return (long)PyLong_AsLong(iter);
  if (PyInt_Check(iter)) 
    return (long)PyInt_AsLong(iter);
  throw std::runtime_error("argument is not a long int or long int-compatible type ...!?");
}

/*! converts a list object to a vector of floats. THe list obviously
    is expected to contains number objects (ints and longs get
    automatically converted to floats) */
std::vector<float> getFloats(PyObject *valuesList)
{
  PyObject *iter = PyObject_GetIter(valuesList);
  if (!iter) 
    throw std::runtime_error("parameter to ospSet3fv is not a list!?");

  std::vector<float> values;
  if (iter)
    while (PyObject *item = PyIter_Next(iter)) {
      values.push_back(getFloat(item));
    }
  return values;
}

/*! converts a list object to a vector of strings. THe list obviously
    is expected to contains number objects (ints and longs get
    automatically converted to strings) */
std::vector<std::string> getStrings(PyObject *valuesList)
{
  PyObject *iter = PyObject_GetIter(valuesList);
  if (!iter) 
    throw std::runtime_error("parameter to ospSet3fv is not a list!?");

  std::vector<std::string> values;
  if (iter)
    while (PyObject *item = PyIter_Next(iter)) {
      values.push_back(getString(item));
    }
  return values;
}

/*! converts a list object to a vector of ints. THe list obviously
    is expected to contains number objects (ints and longs get
    automatically converted to ints) */
std::vector<int> getInts(PyObject *valuesList)
{
  PyObject *iter = PyObject_GetIter(valuesList);
  if (!iter) 
    throw std::runtime_error("parameter to ospSet3fv is not a list!?");

  std::vector<int> values;
  if (iter)
    while (PyObject *item = PyIter_Next(iter)) {
      values.push_back(getInt(item));
    }
  return values;
}

/*! converts a list object to a vector of longs. THe list obviously
    is expected to contains number objects (longs and longs get
    automatically converted to longs) */
std::vector<long> getLongs(PyObject *valuesList)
{
  PyObject *iter = PyObject_GetIter(valuesList);
  if (!iter) 
    throw std::runtime_error("parameter to ospSet3fv is not a list!?");

  std::vector<long> values;
  if (iter)
    while (PyObject *item = PyIter_Next(iter)) {
      values.push_back(getLong(item));
    }
  return values;
}





// ##################################################################
// actual API functions
// ##################################################################


// ==================================================================
// misc
// ==================================================================

// ------------------------------------------------------------------
// ospInit
// ------------------------------------------------------------------
extern "C" PyObject *ospray_init(PyObject *self, PyObject *args)
{
  ospInit(0,nullptr);
  Py_INCREF(Py_None);
  return Py_None;
}

// ------------------------------------------------------------------
// ospShutdown
// ------------------------------------------------------------------
extern "C" PyObject *ospray_shutdown(PyObject *self, PyObject *args)
{
  ospShutdown();
  Py_INCREF(Py_None);
  return Py_None;
}

// ------------------------------------------------------------------
// ospCommit
// ------------------------------------------------------------------
extern "C" PyObject *ospray_commit(PyObject *self, PyObject *args)
{
  // arguments:
  OSPObject object;
  if (!PyArg_ParseTuple(args, "l", &object)) 
    return NULL;
  ospCommit((OSPObject)object);
  Py_INCREF(Py_None);
  return Py_None;
}

// ------------------------------------------------------------------
// ospAddGeometry
// ------------------------------------------------------------------
extern "C" PyObject *ospray_addGeometry(PyObject *self, PyObject *args)
{
  // arguments:
  OSPObject model;
  OSPObject geom;
  if (!PyArg_ParseTuple(args, "ll", &model,&geom)) 
    return NULL;
  ospAddGeometry((OSPModel)model,(OSPGeometry)geom);
  Py_INCREF(Py_None);
  return Py_None;
}

// ------------------------------------------------------------------
// ospRelease
// ------------------------------------------------------------------
extern "C" PyObject *ospray_release(PyObject *self, PyObject *args)
{
  // arguments:
  OSPObject object;
  if (!PyArg_ParseTuple(args, "l", &object)) 
    return NULL;
  ospRelease((OSPObject)object);
  Py_INCREF(Py_None);
  return Py_None;
}

// ------------------------------------------------------------------
// ospFrameBufferClear
// ------------------------------------------------------------------
extern "C" PyObject *ospray_frameBufferClear(PyObject *self, PyObject *args)
{
  OSPFrameBuffer fb;
  PyObject   *channelsList;
  
  if (!PyArg_ParseTuple(args, "lO", &fb, &channelsList)) 
    return NULL;

  std::vector<std::string> channelStrings = getStrings(channelsList);
  uint64_t channels = 0;
  for (auto &channel: channelStrings) {
    if (channel == "color") channels |= OSP_FB_COLOR;
    if (channel == "depth") channels |= OSP_FB_DEPTH;
    if (channel == "accum") channels |= OSP_FB_ACCUM;
  }
  ospFrameBufferClear(fb,channels);
  Py_INCREF(Py_None);
  return Py_None;
}

// ------------------------------------------------------------------
// ospFrameBufferSave
// ------------------------------------------------------------------
extern "C" PyObject *ospray_frameBufferSave(PyObject *self, PyObject *args)
{
  char *fileName;
  OSPFrameBuffer fb;
  osp::vec2i size;
  char *format;
  
  if (!PyArg_ParseTuple(args, "sl(ii)s",
                        &fileName, &fb, &size.x, &size.y, &format)) 
    return NULL;

  uint32_t *pixels = (uint32_t *)ospMapFrameBuffer(fb,OSP_FB_COLOR);
  writePPM(fileName,size,pixels);
  ospUnmapFrameBuffer(pixels,fb);
  Py_INCREF(Py_None);
  return Py_None;
}

// ------------------------------------------------------------------
// ospRenderFrame
// ------------------------------------------------------------------
extern "C" PyObject *ospray_renderFrame(PyObject *self, PyObject *args)
{
  OSPFrameBuffer fb;
  OSPRenderer renderer;
  PyObject   *channelsList;
  
  if (!PyArg_ParseTuple(args, "llO", &fb, &renderer, &channelsList)) 
    return NULL;

  std::vector<std::string> channelStrings = getStrings(channelsList);
  uint64_t channels = 0;
  for (auto &channel: channelStrings) {
    if (channel == "color") channels |= OSP_FB_COLOR;
    if (channel == "depth") channels |= OSP_FB_DEPTH;
    if (channel == "accum") channels |= OSP_FB_ACCUM;
  }
  ospRenderFrame(fb,renderer,channels);
  Py_INCREF(Py_None);
  return Py_None;
}







// ==================================================================
// object creation
// ==================================================================

// ------------------------------------------------------------------
// ospNewCamera
// ------------------------------------------------------------------
extern "C" PyObject *ospray_newCamera(PyObject *self, PyObject *args)
{
  const char *typeString = nullptr;
  if (!PyArg_ParseTuple(args, "s", &typeString)) 
    return NULL; 
  OSPCamera camera = ospNewCamera(typeString);
  return Py_BuildValue("l", (uint64_t)camera);
}

// ------------------------------------------------------------------
// ospNewRenderer
// ------------------------------------------------------------------
extern "C" PyObject *ospray_newRenderer(PyObject *self, PyObject *args)
{
  const char *typeString = nullptr;
  if (!PyArg_ParseTuple(args, "s", &typeString)) 
    return NULL; 
  OSPRenderer renderer = ospNewRenderer(typeString);
  return Py_BuildValue("l", (uint64_t)renderer);
}

// ------------------------------------------------------------------
// ospNewLight
// ------------------------------------------------------------------
extern "C" PyObject *ospray_newLight(PyObject *self, PyObject *args)
{
  const char *typeString = nullptr;
  if (!PyArg_ParseTuple(args, "s", &typeString)) 
    return NULL; 
  OSPLight light = ospNewLight3(typeString);
  return Py_BuildValue("l", (uint64_t)light);
}


// ------------------------------------------------------------------
// ospNewFrameBuffer
// ------------------------------------------------------------------
extern "C" PyObject *ospray_newFrameBuffer(PyObject *self, PyObject *args)
{
  osp::vec2i size;
  const char *formatString;
  PyObject   *channelsList;
  
  if (!PyArg_ParseTuple(args, "(ii)sO",
                        &size.x, &size.y, &formatString, &channelsList)) 
    return NULL;

  OSPFrameBufferFormat format = parseFrameBufferFormat(formatString);

  std::vector<std::string> channelStrings = getStrings(channelsList);
  uint64_t channels = 0;
  for (auto &channel: channelStrings) {
    if (channel == "color") channels |= OSP_FB_COLOR;
    if (channel == "depth") channels |= OSP_FB_DEPTH;
    if (channel == "accum") channels |= OSP_FB_ACCUM;
  }
  OSPFrameBuffer fb = ospNewFrameBuffer(size,format,channels);
  return Py_BuildValue("l", (uint64_t)fb);
}



// ------------------------------------------------------------------
// ospNewModel
// ------------------------------------------------------------------
extern "C" PyObject *ospray_newModel(PyObject *self, PyObject *args)
{
  OSPModel model = ospNewModel();
  return Py_BuildValue("l", (uint64_t)model);
}

// ------------------------------------------------------------------
// ospNewGeometry
// ------------------------------------------------------------------
extern "C" PyObject *ospray_newGeometry(PyObject *self, PyObject *args)
{
  const char *typeString = nullptr;
  if (!PyArg_ParseTuple(args, "s", &typeString)) 
    return NULL; 
  OSPGeometry geometry = ospNewGeometry(typeString);
  return Py_BuildValue("l", (uint64_t)geometry);
}


// ------------------------------------------------------------------
// ospNewData
// ------------------------------------------------------------------
extern "C" PyObject *ospray_newData(PyObject *self, PyObject *args)
{
  // args:
  int numItems;
  const char *formatString;
  PyObject   *valuesList;
  
  if (!PyArg_ParseTuple(args, "isO", &numItems, &formatString, &valuesList)) 
    return NULL;

  const std::string format = formatString;
  if (format == "float3a" || format == "OSP_FLOAT3A") {
    std::vector<float> values = getFloats(valuesList);
    OSPData data = ospNewData(numItems,OSP_FLOAT3A,values.data(),0);
    return Py_BuildValue("l", (uint64_t)data);
  }
  if (format == "float4" || format == "OSP_FLOAT4") {
    std::vector<float> values = getFloats(valuesList);
    OSPData data = ospNewData(numItems,OSP_FLOAT4,values.data(),0);
    return Py_BuildValue("l", (uint64_t)data);
  }
  if (format == "int3" || format == "OSP_INT3") {
    std::vector<int> values = getInts(valuesList);
    OSPData data = ospNewData(numItems,OSP_INT3,values.data(),0);
    return Py_BuildValue("l", (uint64_t)data);
  }
  if (format == "light" || format == "OSP_LIGHT") {
    std::vector<long> values = getLongs(valuesList);
    OSPData data = ospNewData(numItems,OSP_LIGHT,values.data(),0);
    return Py_BuildValue("l", (uint64_t)data);
  }

  throw std::runtime_error("unknown or not implemeneted format type '"
                                  +format+"' is ospNewData");
  
}







// ==================================================================
// 'set' methods
// ==================================================================

// ------------------------------------------------------------------
// ospSetObject(OSPObject)
// ------------------------------------------------------------------
extern "C" PyObject *ospray_setObject(PyObject *self, PyObject *args)
{
  // arguments:
  OSPObject object;
  const char *varName;
  OSPObject value;
  
  if (!PyArg_ParseTuple(args, "lsl", &object,&varName,&value)) 
    return NULL;

  ospSetObject(object,varName,value);
  Py_INCREF(Py_None);
  return Py_None;
}

// ------------------------------------------------------------------
// ospSetData(OSPData)
// ------------------------------------------------------------------
extern "C" PyObject *ospray_setData(PyObject *self, PyObject *args)
{
  // arguments:
  OSPObject object;
  const char *varName;
  OSPData value;
  
  if (!PyArg_ParseTuple(args, "lsl", &object,&varName,&value)) 
    return NULL;

  ospSetData(object,varName,value);
  Py_INCREF(Py_None);
  return Py_None;
}

// ------------------------------------------------------------------
// ospSet1i(int)
// ------------------------------------------------------------------
extern "C" PyObject *ospray_set1i(PyObject *self, PyObject *args)
{
  // arguments:
  OSPObject object;
  const char *varName;
  int value;
  
  if (!PyArg_ParseTuple(args, "lsi", &object,&varName,&value)) 
    return NULL;

  ospSet1i(object,varName,value);
  Py_INCREF(Py_None);
  return Py_None;
}

// ------------------------------------------------------------------
// ospSet1f(float)
// ------------------------------------------------------------------
extern "C" PyObject *ospray_set1f(PyObject *self, PyObject *args)
{
  // arguments:
  OSPObject object;
  const char *varName;
  float value;
  
  if (!PyArg_ParseTuple(args, "lsf", &object,&varName,&value)) 
    return NULL;

  ospSet1f(object,varName,value);
  Py_INCREF(Py_None);
  return Py_None;
}

// ------------------------------------------------------------------
// ospSet3fv(list of three floats)
// ------------------------------------------------------------------
extern "C" PyObject *ospray_set3fv(PyObject *self, PyObject *args)
{
  // arguments:
  OSPObject object;
  const char *varName;
  PyObject *valuesList;
  
  if (!PyArg_ParseTuple(args, "lsO", &object,&varName,&valuesList)) 
    return NULL;

  std::vector<float> value = getFloats(valuesList);
  assert(value.size() == 3);
  ospSet3fv(object,varName,value.data());
  Py_INCREF(Py_None);
  return Py_None;
}















// ##################################################################
// final method table and hook-up code
// ##################################################################

static PyMethodDef SpamMethods[] = {
  //misc
  {"ospInit",       ospray_init,       METH_VARARGS, "initialize ospray library."},
  {"ospShutdown",       ospray_shutdown,       METH_VARARGS, "shutdownialize ospray library."},
  {"ospCommit",     ospray_commit,     METH_VARARGS, "ospCommit()."},
  {"ospAddGeometry",ospray_addGeometry,METH_VARARGS, "ospAddGeometry."},
  {"ospRelease",    ospray_release,    METH_VARARGS, "release object handle"},
  {"ospFrameBufferClear",ospray_frameBufferClear,   METH_VARARGS, "clear specified channels of a frame buffer."},
  {"ospFrameBufferSave",ospray_frameBufferSave,   METH_VARARGS, "save frame buffer in a file."},
  {"ospRenderFrame",ospray_renderFrame,   METH_VARARGS, "clear specified channels of a frame buffer."},
  //object creation
  {"ospNewCamera",  ospray_newCamera,  METH_VARARGS, "create a new camera object."},
  {"ospNewRenderer",ospray_newRenderer,METH_VARARGS, "create a new renderer object."},
  {"ospNewLight",   ospray_newLight,   METH_VARARGS, "create a new light object."},
  {"ospNewModel",   ospray_newModel,   METH_VARARGS, "create a new model object."},
  {"ospNewFrameBuffer",ospray_newFrameBuffer,   METH_VARARGS, "create a new frame buffer object."},
  {"ospNewData",    ospray_newData,    METH_VARARGS, "create a new data object."},
  {"ospNewGeometry",ospray_newGeometry,METH_VARARGS, "create a new geometry object."},
  //set functions
  {"ospSetData",    ospray_setData,  METH_VARARGS, "set data-object parameter."},
  {"ospSetObject",  ospray_setObject,METH_VARARGS, "set object-object parameter."},
  {"ospSetf",       ospray_set1f,    METH_VARARGS, "set 1f-typed parameter."},
  {"ospSet1i",      ospray_set1i,    METH_VARARGS, "set 1i-typed parameter."},
  {"ospSet1f",      ospray_set1f,    METH_VARARGS, "set 1f-typed parameter."},
  {"ospSet3fv",     ospray_set3fv,   METH_VARARGS, "set param to list of three floats."},
  //...
  {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initospray(void)
{
  printf("#PySPRay: Initializing pyton-ospray module...\n");
  
  (void) Py_InitModule("ospray", SpamMethods);
}

// iw - the tutorial suggests doing this function, but i'm not even
// sure it ever gets called!?
int main(int argc, char *argv[])
{
  /* Pass argv[0] to the Python interpreter */
  Py_SetProgramName(argv[0]);
  
  /* Initialize the Python interpreter.  Required. */
  Py_Initialize();
  
  /* Add a static module */
  initospray();
}
