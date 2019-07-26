#include <string>
#include <iostream>
#include <OpenImageIO/imagebuf.h>

// This example doesn't actually work without red.
#if USE_RED
    #include <R3DSDK.h>
    // But this if block demonstrates the sort of thing
    // you'd do if you were making optional red support.
    // The preprocessor define is set in the example CMake.
#endif

using namespace std;
using namespace OIIO;
using namespace R3DSDK;


size_t mem_needed(int w, int h, int n_channels=3, int bytes_per_samp=1)
{
    return w * h * n_channels * bytes_per_samp;
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
    char *mode_chars = (char *) (&mode);

    switch (mode_chars[2]) {
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





int main(int argc, char *argv[]) {

    cout << "Looking for R3D libs in " << REDRUNTIME << endl;
    auto status = R3DSDK::InitializeSdk(REDRUNTIME, 0);
    std::cout << "Red init status code " << status << " " << R3DSDK::GetSdkVersion() << std::endl;
    if (status) {
        std::cout << "Red init failed.  Exiting." << std::endl;
        R3DSDK::FinalizeSdk();
        return EXIT_FAILURE;
    }
    string file_path("/home/will/Videos/redonemx-4k-30fps/A004_C186_011278_001.R3D");
    if (argc > 1) {
        file_path = argv[1];
    }
    Clip clip(file_path.c_str());
    auto w = clip.Width(), h = clip.Height();
    cout << "Clip " << w << "x" << h << " : status: " << clip.Status() << endl;

    VideoDecodeJob job;
    job.BytesPerRow = w * 4 * 1;
    job.OutputBufferSize = mem_needed(w, h, 4);
    job.Mode = R3DSDK::DECODE_FULL_RES_PREMIUM;
    cout << "Decoding at  1/" << res_scale_for_decode_mode(job.Mode) << " resolution." << endl;

    auto mem = mem_needed(w, h, 4);
    unsigned char * buffer = AlignedMalloc(mem);
    job.OutputBuffer = buffer;
    job.PixelType = R3DSDK::PixelType_8Bit_BGRA_Interleaved;

    cout << "Going to decode frame 0" << endl;
    if (clip.DecodeVideoFrame(0U, job) != R3DSDK::DSDecodeOK) {
            printf("Decode failed?\n");
            R3DSDK::FinalizeSdk();
            return EXIT_FAILURE;
    }
    cout << "Decoding complete. " << endl;


    // Use OIIO to save the frame, rather than writing a raw buffer to disk.
    ImageSpec s(w, h, 4, TypeDesc::UINT8);

    auto result = ImageBuf(s, buffer);
    result.write("redsdk_output.jpg");

    return 0;
}
