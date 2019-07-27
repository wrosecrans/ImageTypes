#include "w_red_util.h"

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include <OpenImageIO/imagebuf.h>
#include <R3DSDK.h>

using namespace std;
using namespace OIIO;
using namespace R3DSDK;

typedef map<u_int32_t, string> modelist;

string mode_as_string(u_int32_t mode) {
    char *mode_c = (char *) (&mode);
    string result;
    result += (mode_c[3]);
    result += (mode_c[2]);
    result += (mode_c[1]);
    result += (mode_c[0]);
    return result;
}


#define ENUM_STR(container, mode) container [mode] = "mode"

/*
 * A reasonable person will note that abusing a macro to avoid typing is probably the wrong solution.
 *
 * The R3D SDK lists enums for things like pixel types, but no convenient way to generate a string
 * version from it that can be used when generating error messages and confidence building printf()s
 * during development.
 *
 */

modelist VideoPixelTypes() {
    modelist types;

    // Effectively,
    //    types[SOME_VAL] = "SOME_VAL";

    ENUM_STR(types, PixelType_16Bit_RGB_Planar) ;
    ENUM_STR(types, PixelType_16Bit_RGB_Interleaved) ;
    ENUM_STR(types, PixelType_8Bit_BGRA_Interleaved) ;
    ENUM_STR(types, PixelType_10Bit_DPX_MethodB) ;
    ENUM_STR(types, PixelType_12Bit_BGR_Interleaved) ;
    ENUM_STR(types, PixelType_8Bit_BGR_Interleaved) ;
    ENUM_STR(types, PixelType_HalfFloat_RGB_Interleaved) ;
    ENUM_STR(types, PixelType_HalfFloat_RGB_ACES_Int) ;

    return types;
}


/*
 * Since most of the code here is already quite "smelly..."
 * A switch statement with intentional passthrough!
 *
 */

int bits_per_sample(u_int32_t mode) {
    switch(mode) {
    case PixelType_16Bit_RGB_Planar:
    case PixelType_16Bit_RGB_Interleaved:
        return 16;

    case PixelType_8Bit_BGRA_Interleaved:
    case PixelType_8Bit_BGR_Interleaved:
        return 8;

    case PixelType_10Bit_DPX_MethodB:
        return 10;
    case PixelType_12Bit_BGR_Interleaved:
        return 12;

    case PixelType_HalfFloat_RGB_Interleaved:
    case PixelType_HalfFloat_RGB_ACES_Int:
        return 16;
    }

    return 0;
}


size_t mem_needed(int w, int h, int n_channels=3, int bytes_per_samp=1)
{
    return w * h * n_channels * bytes_per_samp;
}


void pixel_types_report() {
    cout << "reporting on pixel types" << endl;
    for (auto n : VideoPixelTypes()) {
        cout << n.first << ": ("
             << mode_as_string(n.first) << ") "
             << n.second << "   " << bits_per_sample(n.first) << " bits per samp." << endl;
    }

    return;
}




// This AlignedMalloc may have been borrowed from the R3D SDK...
// TODO : reimplement cleanroom version from scratch.
unsigned char * AlignedMalloc(size_t & sizeNeeded)
{
    // alloc 15 bytes more to make sure we can align the buffer in case it isn't
    unsigned char * buffer = (unsigned char *)malloc(sizeNeeded + 15U);

    if (!buffer)
        return NULL;

    sizeNeeded = 0U;

    // cast to a 32-bit or 64-bit (depending on platform) integer so we can do the math
    uintptr_t ptr = (uintptr_t)buffer;

    // check if it's already aligned, if it is we're done
    if ((ptr % 16U) == 0U)
        return buffer;

    // calculate how many bytes we need
    sizeNeeded = 16U - (ptr % 16U);

    return buffer + sizeNeeded;
}


// In theory, mucking around with casting an int to a char array
// is fantastically unwise, and will explode badly on exotic platforms.
// OTOH, the R3D SDK doesn't really support any terribly exotic platforms.

// Anyhow, the third byte of the decode mode enum represents decoding at
// "Full," "Half," "Quarter" or a "Sixteenth" of the resolution.

// DECODE_FULL_RES_PREMIUM == Premium Res Full Decode == "PRFD"

int res_scale_for_decode_mode(u_int32_t mode) {
    switch (mode_as_string(mode)[2]) {
    case 'F':
        return 1;
    case 'H':
        return 2;
    case 'Q':
        return 4;
    case 'S':
        return 16;
    }
    return 0;
}

