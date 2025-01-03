## MacIcecast
`MacIcecast` is an [IceCast](https://icecast.org) client for Mac-OS that supports `MP3` and `AAC` streaming at standard bitrates and sample-rates.

## Dependancies
* [Boost::ASIO](https://www.boost.org/doc/libs/1_87_0/doc/html/boost_asio.html) for client-server communication
* [LibMP3LAME](https://lame.sourceforge.io) for MP3 encoding.
* [FDK-AAC](https://github.com/mstorsjo/fdk-aac) for AAC encoding.
* [AudioToolBox](https://developer.apple.com/documentation/audiotoolbox?language=objc) for audio-input.
* [Foundation](https://developer.apple.com/documentation/foundation?language=objc) and [Core Foundation](https://developer.apple.com/documentation/corefoundation?language=objc) for data-types. 
* [MSLogger](https://github.com/nav-mohan/mslogger) for thread-safe logging.


## Design 
Audio streaming software requires high performance and minimal latency to ensure smooth playback without glitches. In C++, virtual functions introduce a level of indirection through the virtual table (vtable), which can add overhead to function calls. While this overhead is typically small, inefficiences can accumulate and even microseconds delays are audibly discernable. To meet strict real-time constraints, `MacIcecast` was designed to avoid virtual functions, instead using static polymorphism.

[`std::variant`](https://en.cppreference.com/w/cpp/utility/variant) enables static polymorphism by allowing a type-safe union of multiple types, chosen at compile time. Instead of using virtual functions, you can define a `std::variant` with the types you want to handle and use `std::visit` to apply a visitor function to the active type. This eliminates runtime overhead from vtable lookups, as type resolution occurs at compile time. For example, `std::variant<TypeA, TypeB, TypeC>` lets you store either `TypeA` or `TypeB` or `TypeC` and perform operations on them without dynamic dispatch, improving performance in scenarios where runtime polymorphism is unnecessary. In the case of `MacIcecast` the `TypeA` and `TypeB` are encoders of specific bitrates, sample-rates, channels, and formats. The supported configurations are
* Formats: MP3, AAC
* Channels: Mono, Stereo
* Bitrates: 64 kbps, 128 kbps, 256 kbps
* Sample Rates: 44100 Hz

This results in 12 permutations. 12 template specialization is a modest price to pay for static polymorphism !

