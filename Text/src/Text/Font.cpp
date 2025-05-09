#include "Font.h"
namespace Aether::Text
{
    bool Font::UpdateDeviceData()
    {
        // glyph texture
        stagingBuffer.SetData(std::span<uint8_t>((uint8_t*)bufferGlyphs.data(),
                                                 sizeof(BufferGlyph) * bufferGlyphs.size()));
        glyphTexture.CopyBuffer(stagingBuffer);
        // curve texture
        stagingBuffer.SetData(std::span<uint8_t>((uint8_t*)bufferCurves.data(),
                                                 sizeof(BufferCurve) * bufferCurves.size()));
        curveTexture.CopyBuffer(stagingBuffer);
        return true;
    }
    bool Font::CreateDeviceData()
    {
        //create device buffer
		//
		// curve texture rgba_float32 512x512  
		// max curve count: 131072
		// *roughly* glyph count(20 curves per glyph): 6553.6
        // per line curve count: 256
		//
		// glyph texture rgba8888 128x128 
		// max glyph count: 16384
        // per line glyph count: 128
		//
		stagingBuffer=DeviceBuffer::CreateForStaging(512*512*4*4);
		if(!stagingBuffer)
		{
			assert(false && "create staging buffer failed");
			return false;
		}
		curveTexture=DeviceTexture::CreateForTexture(512, 512, PixelFormat::RGBA_FLOAT32).value();
		if(!curveTexture)
		{
			assert(false && "create curve texture failed");
			return false;
		}
		curveTexture.GetOrCreateDefaultImageView();
		glyphTexture=DeviceTexture::CreateForTexture(128, 128, PixelFormat::RGBA8888).value();
		if(!glyphTexture)
		{
			assert(false && "create glyph texture failed");
			return false;
		}
		glyphTexture.GetOrCreateDefaultImageView();
        return true;
    }
}