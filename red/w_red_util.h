
#include <string>
#include <map>
#include <tuple>

typedef std::map<uint32_t, std::string> modelist;
typedef std::pair<int, int> resolution;

size_t mem_needed(int w, int h, int n_channels=3, int bytes_per_samp=1);
std::string mode_as_string(uint32_t mode);

modelist VideoPixelTypes();
modelist DecodeModes();

int bits_per_sample(uint32_t mode);

void pixel_types_report();

int res_scale_for_decode_mode(uint32_t mode);
resolution scaled_resolution(resolution full, uint32_t mode);

uint32_t adequate_decode_mode(resolution full, resolution target);

unsigned char * AlignedMalloc(size_t & sizeNeeded);

