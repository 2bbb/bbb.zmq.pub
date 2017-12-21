#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <thread>

#include <zmq.hpp>

#include "bbb.max.dev.hpp"
#include "bbb.zmq.pub.hpp"

zmq::context_t ctx;

template <typename type>
union converter {
    std::uint8_t u[sizeof(type)];
    type v;
};

template <typename type>
void conv_impl(std::vector<std::uint8_t> &buf, t_atom *v) {
    converter<type> conv;
    switch(v->a_type) {
        case A_LONG:  conv.v = atom_getlong(v); break;
        case A_FLOAT: conv.v = atom_getfloat(v); break;
        default:
            break;
    }
    buf.insert(buf.end(), std::begin(conv.u), std::end(conv.u));
}

std::map<format_token, std::function<void(std::vector<std::uint8_t> &, t_atom *)>> append_buffer{
    {
        format_token::int8_type,
        [](std::vector<std::uint8_t> &buf, t_atom *v) {
            union {
                std::uint8_t u;
                std::int8_t v;
            } conv{0};
            switch(v->a_type) {
                case A_LONG:  conv.v = atom_getlong(v); break;
                case A_FLOAT: conv.v = atom_getfloat(v); break;
                default:
                    break;
            }
            buf.push_back(conv.u);
        }
    }, {
        format_token::uint8_type,
        [](std::vector<std::uint8_t> &buf, t_atom *v) {
            std::uint8_t u{0};
            switch(v->a_type) {
                case A_LONG:  u = atom_getlong(v); break;
                case A_FLOAT: u = atom_getfloat(v); break;
                default:
                    break;
            }
            buf.push_back(u);
        }
    }, {
        format_token::int16_type,
        &conv_impl<std::int16_t>
    }, {
        format_token::uint16_type,
        &conv_impl<std::uint16_t>
    }, {
        format_token::int32_type,
        &conv_impl<std::int32_t>
    }, {
        format_token::uint32_type,
        &conv_impl<std::uint32_t>
    }, {
        format_token::int64_type,
        &conv_impl<std::int64_t>
    }, {
        format_token::uint64_type,
        &conv_impl<std::uint64_t>
    }, {
        format_token::float_type,
        &conv_impl<float>
    }, {
        format_token::double_type,
        &conv_impl<double>
    }, {
        format_token::text_type,
        [](std::vector<std::uint8_t> &buf, t_atom *v) {
            std::string str;
            switch(v->a_type) {
                case A_LONG:  str = std::to_string(atom_getlong(v)); break;
                case A_FLOAT: str = std::to_string(atom_getfloat(v)); break;
                case A_SYM:   str = atom_getsym(v)->s_name; break;
                default:
                    break;
            }
            buf.insert(buf.end(), std::begin(str), std::end(str));
            buf.push_back(0);
        }
    }
};

class MaxZmqPub : public bbb::max_obj<MaxZmqPub> {
    std::size_t buf_size{65536};
    std::string format_str{"[t]"};
    std::string connected_host{""};
    zmq::socket_t socket;
    bool now_connected{false};
    bool now_binded{false};
    bool is_client{false};
    
    static void print_info() {
        static bool isnt_printed = true;
        if(isnt_printed) {
            post("bbb.zmq.pub: v%d.%d.%d\n", bbb::major_version, bbb::minor_version, bbb::patch_version);
            if(bbb::is_develop_ver) {
                post("  [develop version]\n");
            }
            post("  author: ISHII 2bit [i@2bit.jp]\n");
            post("  url:    %s\n", bbb::distribute_url);
            isnt_printed = false;
        }
    }
    
    inline void dump(const std::string &str) {
        post("bbb.zmq.pub: %s\n", str.c_str());
    }
    
    inline void dump_error(const std::string &str) {
        error("bbb.zmq.pub: %s\n", str.c_str());
        outlet(1, "error_mess", str);
    }
    
    bool connect_impl(t_atom *host, bool as_client = true) {
        if(now_binded || now_connected) {
            dump_error("already connected or binded");
            return false;
        }
        connected_host = atom_getsym(host)->s_name;
        try {
            if(as_client) {
                socket.connect(connected_host.c_str());
                now_connected = true;
            } else {
                socket.bind(connected_host.c_str());
                now_binded = true;
            }
        } catch(const zmq::error_t &e) {
            dump_error(e.what());
            outlet(1, "connected", 0, "");
            return false;
        }
        outlet(1, "connected", 1, as_client ? "connect" : "bind");
        return true;
    }

    void disconnect_impl() {
        if(connected_host != "" && (now_connected || now_binded)) {
            if(now_binded) {
                char   endpoint[256];
                size_t endpoint_len = sizeof(endpoint);
                socket.getsockopt(ZMQ_LAST_ENDPOINT, endpoint, &endpoint_len);
                socket.unbind(endpoint);
                now_binded = false;
            } else {
                char   endpoint[256];
                size_t endpoint_len = sizeof(endpoint);
                socket.getsockopt(ZMQ_LAST_ENDPOINT, endpoint, &endpoint_len);
                socket.disconnect(endpoint);
                now_connected = false;
            }
            connected_host = "";
            outlet(1, "connected", 0, "");
        }
    }

    void parse_format(t_atom *pat_atom) {
        format_str = atom_getsym(pat_atom)->s_name;
    }
    
public:
    MaxZmqPub()
        : socket(ctx, ZMQ_PUB)
    {
        setupIO(1, 2); // inlets / outlets
        print_info();
    }
    
	MaxZmqPub(t_symbol *sym, long ac, t_atom *av)
        : MaxZmqPub()
    {
        copyArgs(sym, ac, av);
    }
    
    ~MaxZmqPub() {
        disconnect_impl();
    }
    
    void loadbang(void *) {
        dump("will initialize...");
        outlet(1, "error_mess", "");
        
        std::size_t ac = args.size();
        t_atom *av = args.data();
        if(2 < ac) {
            if(atom_gettype(av + 2) == A_SYM) {
                is_client = strncmp(atom_getsym(av + 2)->s_name, "connect", 32) == 0;
            }
        }
        if(1 < ac) {
            if(atom_gettype(av + 1) == A_SYM) {
                parse_format(av + 1);
            }
        }
        if(0 < ac) {
            if(atom_gettype(av) == A_SYM) {
                connect_impl(av, is_client);
            }
        }
    }
    
	// methods:
    void send(long inlet, t_symbol *s, long argc, t_atom *argv) {
        if(!now_connected && !now_binded) {
            dump_error("not binded & connected");
            return;
        }
        std::size_t format_cursor{0};
        std::vector<std::uint8_t> buf;
        buf.reserve(buf_size);
        
        for(std::size_t i = 0, format_length = format_str.length(); i < argc && format_cursor < format_length; format_cursor++) {
            format_token token = (format_token)format_str[format_cursor];
            switch(token) {
                case format_token::int8_type:
                case format_token::uint8_type:
                case format_token::int16_type:
                case format_token::uint16_type:
                case format_token::int32_type:
                case format_token::uint32_type:
                case format_token::int64_type:
                case format_token::uint64_type:
                case format_token::float_type:
                case format_token::double_type:
                case format_token::text_type: {
                    append_buffer[token](buf, argv + i);
                    i++;
                    break;
                }
                case format_token::array_begin:
                    break;
                case format_token::array_end: {
                    while(format_cursor && (format_token)format_str[format_cursor--] != format_token::array_begin);
                    if(format_cursor == 0 && (format_token)format_str[0] != format_token::array_begin) {
                        dump_error("format_str is wrong");
                        return;
                    }
                    break;
                }
                case format_token::skip_byte:
                    i++;
                    break;
                case format_token::zero_padding:
                    buf.push_back(0);
                    break;
                default:
                    break;
            }
        }
        zmq::message_t mess;
        std::size_t size = socket.send(buf.data(), buf.size());
        outlet(0, size);
    }
    
    void connect(long inlet, t_symbol *s, long ac, t_atom *av) {
        if(0 < ac && atom_gettype(av) == A_SYM) connect_impl(av, true);
    }
    void bind(long inlet, t_symbol *s, long ac, t_atom *av) {
        if(0 < ac && atom_gettype(av) == A_SYM) connect_impl(av, false);
    }
    void disconnect(long inlet, t_symbol *s, long ac, t_atom *av) {
        disconnect_impl();
    }
    void unbind(long inlet, t_symbol *s, long ac, t_atom *av) {
        disconnect_impl();
    }
    void format(long inlet, t_symbol *s, long ac, t_atom *av) {
        if(0 < ac && atom_gettype(av) == A_SYM) parse_format(av);
    }

    void assist(void *b, long io, long index, char *s) {
        switch (io) {
            case 1:
                switch(index) {
                    case 0:
                        strncpy_zero(s, "input data", 32);
                        break;
                    default:
                        break;
                }
                break;
            case 2:
                switch(index) {
                    case 0:
                        strncpy_zero(s, "sent byte", 32);
                        break;
                    case 1:
                        strncpy_zero(s, "status", 32);
                        break;
                    default:
                        break;
                }
                break;
        }
    }
};

C74_EXPORT int main(void) {
	// create a class with the given name:
	MaxZmqPub::makeMaxClass("bbb.zmq.pub");
    MaxZmqPub::registerStandardFunctions();
    MaxZmqPub::registerGimme<&MaxZmqPub::bind>("bind");
    MaxZmqPub::registerGimme<&MaxZmqPub::connect>("connect");
    MaxZmqPub::registerGimme<&MaxZmqPub::unbind>("unbind");
    MaxZmqPub::registerGimme<&MaxZmqPub::disconnect>("disconnect");
    MaxZmqPub::registerGimme<&MaxZmqPub::format>("format");
    MaxZmqPub::registerGimme<&MaxZmqPub::send>("send");
}
