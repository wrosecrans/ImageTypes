#include "w_red_util.h"

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <tuple>

#include <OpenImageIO/imagebuf.h>
#include <R3DSDK.h>

using namespace std;
using namespace OIIO;
using namespace R3DSDK;

typedef map<uint32_t, string> modelist;
typedef std::pair<int, int> resolution;

string mode_as_string(uint32_t mode) {
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




modelist DecodeModes() {
    modelist modes;
    ENUM_STR(modes, DECODE_FULL_RES_PREMIUM) ;

    ENUM_STR(modes, DECODE_HALF_RES_PREMIUM) ;
    ENUM_STR(modes, DECODE_HALF_RES_GOOD) ;

    ENUM_STR(modes, DECODE_QUARTER_RES_GOOD) ;
    ENUM_STR(modes, DECODE_EIGHT_RES_GOOD) ;
    ENUM_STR(modes, DECODE_SIXTEENTH_RES_GOOD) ;

    // Ignore DECODE_ROCKET_CUSTOM_RES
    return modes;
}

/*
 * Since most of the code here is already quite "smelly..."
 * A switch statement with intentional passthrough!
 *
 */

int bits_per_sample(uint32_t mode) {
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


size_t mem_needed(int w, int h, int n_channels, int bytes_per_samp)
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

// DECODE_FULL_RES_PREMIUM == "DFRP"

int res_scale_for_decode_mode(uint32_t mode) {
    switch (mode_as_string(mode)[1]) {
    case 'F':
        return 1;
    case 'H':
        return 2;
    case 'Q':
        return 4;
    case 'E':
        return 8;
    case 'S':
        return 16;
    }
    return 0;
}



resolution scaled_resolution(resolution full, uint32_t mode) {
    resolution r = full;
    auto scale = res_scale_for_decode_mode(mode);
    if (! scale) {
        return resolution(0,0);
    }
    r.first = full.first / scale;
    r.second = full.second / scale;
    return r;
}



// This can be done more efficiently, but it'll automagically work if
// more modes are added to DecodeModes(), and it's run rarely.

uint32_t adequate_decode_mode(resolution full, resolution target) {
    uint32_t last_mode = DECODE_FULL_RES_PREMIUM;

    if (target.first >= full.first || target.second >= full.second) {
        return DECODE_FULL_RES_PREMIUM;
    }

    auto rx = full.first / target.first;
    auto ry = full.second / target.second;
    auto ratio = rx;
    if (ry < rx) {
        ratio = ry;
    }

    if (ratio > 16) {
        return DECODE_SIXTEENTH_RES_GOOD;
    }

    cout << " Adequating ratio " << rx << "x" << ry << "/ " << ratio << " for "
         << full.first << "x" << full.second
         << "p into " << target.first << "x" << target.second
         << " f/r: " << full.first / rx << "x" << full.second / ry << endl;

    switch(ratio) {
    case 1:
        return DECODE_FULL_RES_PREMIUM;
    case 2:
        return DECODE_HALF_RES_GOOD;
    case 3:
    case 4:
        return DECODE_QUARTER_RES_GOOD;
    case 5:
    case 6:
    case 7:
    case 8:
        return DECODE_EIGHT_RES_GOOD;
    }

    return DECODE_EIGHT_RES_GOOD;



    for(auto mode : DecodeModes()) {
        auto scale = res_scale_for_decode_mode(mode.first);
        cout << "Checking " << mode_as_string(mode.first) << " scale:" << scale << " " << mode.second << endl;
        auto res = scaled_resolution(full, mode.first);
        cout << "    " << res.first << " " << target.first << endl;

        // If the resolution for this mode is below target, we've gone too far.
        if (res.first < target.first || res.second < target.second) {
        //     return last_mode;
        }
        last_mode = mode.first;
    }

    // If we made if through the whole list, the worst/fastest mode is adequate.
    return last_mode;
}








