//
//  bbb.zmq.pub.hpp
//  bbb.zmq.pub
//
//  Created by ISHII 2bit on 2017/12/16.
//

#ifndef bbb_zmq_pub_hpp
#define bbb_zmq_pub_hpp

namespace bbb {
    constexpr std::size_t major_version = 0;
    constexpr std::size_t minor_version = 0;
    constexpr std::size_t patch_version = 0;
    constexpr bool is_develop_ver = true;
    constexpr char distribute_url[] = "https://github.com/2bbb/bbb-max/";
};

enum class format_token : uint8_t {
    int8_type = 'c',
    uint8_type = 'C',
    int16_type = 's',
    uint16_type = 'S',
    int32_type = 'i',
    uint32_type = 'I',
    int64_type = 'l',
    uint64_type = 'L',
    float_type = 'f',
    double_type = 'd',
    text_type = 't',
    array_begin = '[',
    array_end = ']',
    skip_byte = '_',
    zero_padding = '0',
};

#endif /* bbb_zmq_pub_hpp */
