
#include <string>
#include <map>


typedef std::map<uint32_t, std::string> modelist;
size_t mem_needed(int w, int h, int n_channels=3, int bytes_per_samp=1);
std::string mode_as_string(uint32_t mode);
modelist VideoPixelTypes();
int bits_per_sample(uint32_t mode);

void pixel_types_report();

int res_scale_for_decode_mode(uint32_t mode);
unsigned char * AlignedMalloc(size_t & sizeNeeded);

