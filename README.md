# bbb.zmq.pub

[zeromq](http://zeromq.org) publisher for [_Cycling'74 Max_](https://cycling74.com/products/max).

## Notice

if you want to develop this object, you had better see [bbb.max.dev](2bbb/bbb.max.dev).

this repository is subdivided from bbb.max.dev.

and after clone/download bbb.max.dev, you see this **For developer** on this README.

## Object Argument

### **bbb.zmq.pub** [ [ `host_str` [ `format_char...` ] ] `bind_state`]

- `host_str` : destination
- - default: `""` (doesn’t connect to subscriber)
- `format_char...` : see _format_ on Messages
- - default: `[t]`
- `use_bind` : if this parameter is "connect"  then use "connect", otherwise use "bind"
  - default: `""` (use bind)

**ex.**

- **bbb.zmq.pub**
- **bbb.zmq.pub** `tcp://*:9999`
- **bbb.zmq.pub** ``ipc:///tmp/foo [c]``

## Messages

### bind `host_str`

bind `host_str` and wait connection from publisher.

### **connect** `host_str`

connect to publisher is binded to host_str.

### unbind

unbind (implementation is same to disconnect. you can use disconnect too)

### **disconnect**

disconnect from publisher.

### **format** `format_char…`

- `c` : char (int8_t)
- `C` : unsigned char (uint8_t)
- `s` : short (int16_t)
- `S` : unsigned short (uint16_t)
- `i` : int (int32_t)
- `I` : unsigned int (uint32_t)
- `l` : long (int64_t)
- `L` : unsigned long (uint64_t)
- `f` : float
- `d` : double
- `t` : null terminated string
- `0`: padding 1byte with 0
- `_` : discard 1byte
- `[X]` : array of type `X`
  - `X` allows complex type doesn't include array.
    - ex. `cc[ccif]` is valid

**ex.**

- format `iCCC`

- - int32_t, uint8_t, uint8_t, uint8_t

- format `ff[iit]`

- - float, float, int32_t, int32_t, string, int32_t, int32_t, string, …, int32_t, int32_t, string

## Licenses

MIT

### Licenses of dependencies

- [2bbb/bbb.max.dev](https://github.com/2bbb/bbb.max.dev) (MIT)
  - [Cycling74/max-sdk](https://github.com/Cycling74/max-sdk) (MIT)
  - [grrrwaaa/maxcpp](https://github.com/grrrwaaa/maxcpp) (MIT)
- [zeromq/libzmq](https://github.com/zeromq/libzmq) (MPL-2.0)
- [zeromq/cppzmq](https://github.com/zeromq/cppzmq) (MIT)

# For developer

## Dependencies

- [2bbb/bbb.max.dev](https://github.com/2bbb/bbb.max.dev)
  - [Cycling74/max-sdk](https://github.com/Cycling74/max-sdk)
    - `v7.3.3` tag
  - [2bbb/maxcpp](https://github.com/grrrwaaa/maxcpp) (fork of [grrrwaaa/maxcpp](https://github.com/grrrwaaa/maxcpp))
    - `bbb.max.dev` branch
- [zeromq/cppzmq](https://github.com/zeromq/cppzmq)
  - [zeromq/libzmq](https://github.com/zeromq/libzmq) (recommend to use [**homebrew**](https://brew.sh/))

## Dev env.

- Touchbar MacBook Pro (15-inch, 2016)
- macOS Sierra v10.12.6
- Xcode v9.1 (9B55)
- Max v7.3.4 (Oct)

### Setup dev env.

```bash
brew install zeromq
```

## Author

* ISHII 2bit
* i[at]2bit.jp

## At last

If you get happy with using this object, and you're rich, please donation for support continuous development.

Bitcoin: `17AbtW73aydfYH3epP8T3UDmmDCcXSGcaf`

