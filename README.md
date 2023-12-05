You cannot do away with the static in Callback so you cannot use m_userData directly inside the Callback.
This means all Microphones will be using the same static Callback definition
which means all microphones will be writing to the same file unless you specify the file inside the custom userdata
similaryl, you will have to specify the encoder inside the custom userdata

can you template the custom userdata on the audiofile? 
if yes, then try to template the custom userdata on the encoder
maybe you dont need to tempate the custom userdata on the encoder and instead you could just use a base class Encoder which is templated like Encoder<MPEGEncoder> or Encoder<AACEncoder> and put the Encoder pointer inside the custom userdata struct


Something wrong with my template geniusness. 
Maybe I should switch back to using virtual functions?
If i create the mic on stack i call the destructor too soon.
if i create the mic on heap i never call the destructor
I need to figure out how to make the mic not destroy itself when it goes out of scope.
is it possible to use a variant (type safe union)? and then explicitly call destructor within the visit method?

We use a `std::variant` when creating a `Microphone`. The variant is a union of all possible template specializations of `Microphone<Encoder>`. 
We allocate `Microphone` on heap using `std::unique_ptr` because stack-allocated `Microphone` is destroyed by `std::variant` as soon as it goes out of scope of a code-block. Even if the `std::variant` was created outside the block. Not sure why that happens. Need to readup more on that here
https://stackoverflow.com/questions/75779392/managing-lifetime-of-stdvariant-instances-how-to-store-pointers-to-stdvari
The other benefit of using a `std::unique_ptr` for holding `Microphone`s is that `Microphone::Stop()` is automatically called in `Microphone::~Microphone()`. 

Client code combines the `Microphone` and `Encoder` together. 
Client code expects `Microphone` to have a `Start()`, `Stop()` method.
`Microphone` library expects the `Encoder` libarry to provide
    - `uint32_t m_samplerate` member variable.
    - `uint32_t m_channel` member variable.
    - `void DoEncode(const void*, const uint32_t)` method
We cannot make `Encoder` a member variable of `Microphone` because `Callback` is `static`. Hence we have to make `Encoder` a member variable of `UserData` instead

When I make a `Client` library with specializations (`IceCastClient`, `ShoutCastClient`) then I will have to template the `Encoder` on the `Client` expecting something like `Microphone<Encoder<Client>`
But maybe not. cause I think `IceCast` and `ShoutCast` might only differ in their headers. which does not merit a whole specialization. 


Start using `MSLogger` for logging.

do we need to copy the bytes inside our call to `Encoder::DoEncode(const void*, const uint32_t)`? Very likely cause it says `const void*` and not `void*` and that goes back to `AudioQueueBuffer::m_audioData`


## Playing a PCM file using ffplay
`ffplay -f s16be -ar 44100 -ac 2 hahaha.pcm`

## Specifying duration for Encoder 
Wish I could template `MPEGEncoder` and `AACEncoder` on `float duration` seconds so that they may alocate appropriate buffersizes for `m_pcmBuffer` and `m_encBuffer`

## Design so far
I have two separate `structs` `MPEGEncoder` and `AACEncoder`
They have some commonality interface, 
```
    const std::string               m_name;
    short                           m_pcmBuffer[2 * CALCULATE_CHANNEL_BUFFER_SIZE];
    unsigned char                   m_encBuffer[2 * CALCULATE_CHANNEL_BUFFER_SIZE];
    const uint32_t m_bitrate =      static_cast<uint32_t>(eb);
    const uint32_t m_samplerate =   static_cast<uint32_t>(es);
    const uint32_t m_channels =     static_cast<uint32_t>(ec);
    int                             m_bytesEncoded;
    constexpr int                   DoEncodeInterleaved(const void *pcmBuffer, const uint32_t pcmBufferSize)
```
The interface is checked for at compile time beacuse we make use of those member variables and functions. But is there a way to use `CRTP` to enforece a common interface `struct Encoder` for them? If I use `CRTP` I would likely have to use a `dynamic_cast<MPEGEncoder>(this)` at runtime inside of the call to `Encoder::DoEncodeInterleaved`. Honestly its not too bad right now. 

# Next steps
1. Finish the AAC Encoder and check by writing to file
2. Template the `Microphone` and `UserData` on `Client` so that and you can use `Client m_client` inside of the `Microphone::Callback` through the `UserData<Encoder,Client>` much like the `Encoder m_encoder`.
3. The AAC Encoder has some bug in it. it has tiny gaps in it. For the timebeing I'm just going to work on `Client` and then go back to working on AAC Encoder. I'm pretty sure this is beacuse I have to use a `miniPcmBuffer` and I'm probably missing out on the last one