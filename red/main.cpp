
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <OpenImageIO/imagebuf.h>
#include <R3DSDK.h>

#include "w_red_util.h"
using namespace std;
using namespace OIIO;
using namespace R3DSDK;




int main(int argc, char *argv[]) {

    pixel_types_report();
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
