#include "gif-lib/shared/gif_lib.h"
#include <string>
#include <vector>
#include <sstream>

#include "UnityEngine/Color32.hpp"
#include "UnityEngine/TextureWrapMode.hpp"
#include "UnityEngine/FilterMode.hpp"
#include "AllFrameResult.hpp"

using namespace UnityEngine;
// raw PVRTC bytes for Unity
struct TextureColor {
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
    uint8_t Transparency;
};

struct Gif
{
    Gif(std::string& text) : datastream(&this->data), data(text){};
    Gif(std::span<uint8_t> const vec) : datastream(&this->data), data(reinterpret_cast<std::span<char> const &>(vec)){};
    Gif(std::span<char> const vec) : datastream(&this->data), data(vec){};
    Gif(Array<char>* array) : datastream(&this->data), data(array){};
    Gif(Array<uint8_t>* array) : Gif(reinterpret_cast<Array<char>*>(array)){};

    /// @brief on destruction we need to actually close the gif file otherwise we are leaking memory
    ~Gif()
    {
        int error = 0;
        DGifCloseFile(gif, &error);
    }
    
    /// @brief parse given data, pretty much always ran directly after construction
    /// @return error code
    int Parse()
    {
        int error = 0;
        /// open Gif file, first arg is anything you wanna pass (for us it's the this instance), second arg is a static function pointer that takes the args that Gif::read takes
        gif = DGifOpen(this, &Gif::read, &error);

        /// 0 means success! keep that in mind
        return error;
    }

    /// @brief "slurp" decompress all the data
    /// @return slurp error 1 for GIF_OK 0 for GIF_ERROR
    int Slurp()
    {
        return DGifSlurp(gif);
    }

    /// @brief static function that reads the data from the datastream
    /// @param pGifHandle the gif file
    /// @param dest the buffer to push to
    /// @param toRead the amount of bytes expected to be read into the buffer
    static int read(GifFileType* pGifHandle, GifByteType* dest, int toRead)
    {
        /// gifhandle->UserData is the first argument passed to DGifOpen where we passed the this pointer
        Gif& dataWrapper = *(Gif*)pGifHandle->UserData;
        return dataWrapper.datastream.readsome(reinterpret_cast<char*>(dest), toRead);
    }
    
    /// @brief gets the frame @ idx
    /// @return texture2d or nullptr on fail
    Texture2D* get_frame(int idx)
    {
        if (!gif || idx > get_length()) return nullptr;

        GifColorType* color;
        SavedImage* frame;
        ExtensionBlock* ext = 0;
        GifImageDesc* frameInfo;
        ColorMapObject* colorMap;
        int x, y, j, loc;

        frame = &(gif->SavedImages[idx]);

        frameInfo = &(frame->ImageDesc);

        if (frameInfo->ColorMap)
        {
            colorMap = frameInfo->ColorMap;
        }
        else
        {
            colorMap = gif->SColorMap;
        }

        for (j = 0; j < frame->ExtensionBlockCount; ++j)
        {
            if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE)
            {
                ext = &(frame->ExtensionBlocks[j]);
                break;
            }
        }
        
        // gif->SWidth is not neccesarily the same as FrameInfo->Width due to a frame possibly describing a smaller block of pixels than the entire gif size
        int width = get_width();
        int height = get_height();
        auto texture = Texture2D::New_ctor(width, height);
        // This is the same size as the entire size of the gif :)
        ::ArrayW<Color32> pixelData = texture->GetPixels32();
        
        // offset into the entire image, might need to also have it's y value flipped? need to test
        long pixelDataOffset = frameInfo->Top * width + frameInfo->Left;
        // it's easier to understand iteration from 0 -> value, than it is to understand value -> value
        for (y = 0; y < frameInfo->Height; ++y)
        {
            for (x = 0; x < frameInfo->Width; ++x)
            {
                loc = y * frameInfo->Width + x;
                if (frame->RasterBits[loc] == ext->Bytes[3] && ext->Bytes[0])
                {
                    continue;
                }

                color = &colorMap->Colors[frame->RasterBits[loc]];
                // for now we just use this method to determine where to draw on the image, we will probably come across a better way though
                long locWithinFrame = (frameInfo->Height - y - 1) * frameInfo->Width + x + pixelDataOffset;
                pixelData[locWithinFrame] = Color32(color->Red, color->Green, color->Blue, 0xff);
            }
        }

        texture->SetAllPixels32(pixelData, 0);
        texture->Apply();
        return texture;
    }
    
    // Stolen from Nya: https://github.com/FrozenAlex/Nya-utils :lovege:
    /// @brief gets the frame @ idx
    /// @return texture2d or nullptr on fail
    AllFramesResult get_all_frames() {
        // Not sure if it helps but everything works
        const char * GifVersion = DGifGetGifVersion(gif);
        int length = get_length();
        if (!gif || length == 0) return {
            nullptr, nullptr
        };

        int width = get_width();
        int height = get_height();
        
        ::ArrayW<UnityEngine::Texture2D*> frames = ArrayW<UnityEngine::Texture2D*>(length);
        ArrayW<float> timings = ArrayW<float> (length);

        // FrameBuffer
        TextureColor *pixelData = new TextureColor[width * height];

        // Persist data from the previous frame
        GifColorType* color;
        SavedImage* frame;
        ExtensionBlock* ext = nullptr;
        GifImageDesc* frameInfo;
        ColorMapObject* colorMap;
        // Graphic control ext block
        GraphicsControlBlock GCB;
        int GCBResult = GIF_ERROR;

        for (int idx = 0; idx < length; idx++) {
            int x, y, j, loc;

            frame = &(gif->SavedImages[idx]);
            frameInfo = &(frame->ImageDesc);
            if (frameInfo->ColorMap) {
                colorMap = frameInfo->ColorMap;
            } else {
                colorMap = gif->SColorMap;
            }
            for (j = 0; j < frame->ExtensionBlockCount; ++j) {
                if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
                    ext = &(frame->ExtensionBlocks[j]);
                    break;
                }
            }
        

            // Get additional info about the frame
            if (ext != nullptr) {
                GCBResult = DGifExtensionToGCB(ext->ByteCount, (const GifByteType*)ext->Bytes, &GCB);
            }
            // gif->SWidth is not neccesarily the same as FrameInfo->Width due to a frame possibly describing a smaller block of pixels than the entire gif size
        
            UnityEngine::Texture2D* texture = UnityEngine::Texture2D::New_ctor(width, height, UnityEngine::TextureFormat::RGBA32, false);
            // This is the same size as the entire size of the gif :)
            // offset into the entire image, might need to also have it's y value flipped? need to test
            long flippedFrameTop = height - frameInfo->Top - frameInfo->Height;
            long pixelDataOffset = flippedFrameTop * width + frameInfo->Left;
            // it's easier to understand iteration from 0 -> value, than it is to understand value -> value
            for (y = 0; y < frameInfo->Height; ++y) {
                for (x = 0; x < frameInfo->Width; ++x) {
                    // Weirdness here is to flip Y coordinate
                    loc = ( frameInfo->Height-y-1 ) * frameInfo->Width + x;
                    // Checks if the pixel is transparent
                    if (GCB.TransparentColor >=0 && frame->RasterBits[loc] == ext->Bytes[3] && ext->Bytes[0]) {
                        continue;
                    }

                    color = &colorMap->Colors[frame->RasterBits[loc]];
                    // for now we just use this method to determine where to draw on the image, we will probably come across a better way though
                    long locWithinFrame = x + pixelDataOffset;
                    pixelData[locWithinFrame] = {
                        color->Red, color->Green, color->Blue, 0xff
                    };
                }

                // Goes to a new row (saves compute power)
                pixelDataOffset = pixelDataOffset + width;
            }

            // Copy raw pixel data to texture
            texture->LoadRawTextureData(pixelData,  width * height * 4);
            // texture->set_filterMode(UnityEngine::FilterMode::Trilinear);
            // Compress texture
            texture->Compress(false);
            // Upload to GPU
            texture->Apply();

            frames[idx] = texture;
            timings[idx] = static_cast<float>(GCB.DelayTime);        
        };
        // Clear FrameBuffer to not leak things
        delete[] pixelData;
        return {
            frames, timings
        };
    }
    

    int get_width() { return gif ? gif->SWidth : 0; };
    int get_height() { return gif ? gif->SHeight : 0; };
    int get_length() { return gif ? gif->ImageCount : 0; };

public:
    GifFileType* gif = nullptr;

private:
    // wrapper type to wrap any datatype for use within a stream
    template <typename CharT, typename TraitsT = std::char_traits<CharT>>
    class vectorwrapbuf : public std::basic_streambuf<CharT, TraitsT> {
    public:
        vectorwrapbuf(std::string& text) {
            this->std::basic_streambuf<CharT, TraitsT>::setg(text.data(), text.data(), text.data() + text.size());
        }

        vectorwrapbuf(std::span<CharT> vec) {
            this->std::basic_streambuf<CharT, TraitsT>::setg(vec.data(), vec.data(), vec.data() + vec.size());
        }

        vectorwrapbuf(Array<CharT>* arr) {
            this->std::basic_streambuf<CharT, TraitsT>::setg(arr->values, arr->values, arr->values + arr->Length());
        }
    };

    std::istream datastream;
    vectorwrapbuf<char> data;
};