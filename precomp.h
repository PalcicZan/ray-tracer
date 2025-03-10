// add your includes to this file instead of to individual .cpp files
// to enjoy the benefits of precompiled headers:
// - fast compilation
// - solve issues with the order of header files once (here)
// do not include headers in header files (ever).

#define SCRWIDTH		800
#define SCRHEIGHT		512
// #define FULLSCREEN
// #define ADVANCEDGL	// faster if your system supports it


#include <inttypes.h>

extern "C" 
{ 
#include "glew.h" 
}
#include "gl.h"
#include "io.h"
#include <fstream>
#include <stdio.h>
#include "fcntl.h"
#include "SDL.h"
#include "wglext.h"
#include "freeimage.h"
#include "math.h"
#include "stdlib.h"
#include "emmintrin.h"
#include "immintrin.h"
#include "windows.h"
#include "template.h"
#include "surface.h"

// user settings
#include "settings.h"

#include "threads.h"
#include <assert.h>
#include <math.h>

using namespace std;
using namespace Tmpl8;


// ray tracer
#include "ray.h"
#include "scene.h"
#include "camera.h"
#include "bvh.h"
#include "renderer.h"
#include "game.h"