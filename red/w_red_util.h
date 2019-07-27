
#include <string>
#include <map>


typedef std::map<u_int32_t, std::string> modelist;

std::string mode_as_string(u_int32_t mode);
modelist VideoPixelTypes();
int bits_per_sample(u_int32_t mode);

void pixel_types_report();

int res_scale_for_decode_mode(u_int32_t mode);

