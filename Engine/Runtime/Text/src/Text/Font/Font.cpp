#include "Font.h"
#include <Render/Utils.h>
namespace Aether::Text
{
    bool Font::UpdateDeviceData()
    {
        // glyph texture
        stagingBuffer.SetData(0,std::span<uint8_t>((uint8_t*)bufferGlyphs.data(),
                                                 sizeof(BufferGlyph) * bufferGlyphs.size()));
		glyphTexture.SyncTransitionLayout(rhi::TextureLayout::ShaderReadOnly, rhi::TextureLayout::TransferDst);
		Render::Utils::SyncUploadTexture2D(stagingBuffer, glyphTexture);
		glyphTexture.SyncTransitionLayout(rhi::TextureLayout::TransferDst, rhi::TextureLayout::ShaderReadOnly);
		
        // curve texture

        stagingBuffer.SetData(0,std::span<uint8_t>((uint8_t*)bufferCurves.data(),
                                                 sizeof(BufferCurve) * bufferCurves.size()));
		curveTexture.SyncTransitionLayout(rhi::TextureLayout::ShaderReadOnly, rhi::TextureLayout::TransferDst);
		Render::Utils::SyncUploadTexture2D(stagingBuffer, curveTexture);
		curveTexture.SyncTransitionLayout(rhi::TextureLayout::TransferDst, rhi::TextureLayout::ShaderReadOnly);
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
		// max glyph count: 8192
        // per line glyph count: 64
		//
		// @note 如果要修改纹理大小，要保证一行上的数据个数是整数个，不能出现一个数据跨越两行
		stagingBuffer=rhi::StagingBuffer::Create(512*512*4*4);
		if(!stagingBuffer)
		{
			assert(false && "create staging buffer failed");
			return false;
		}
		
		curveTexture=rhi::Texture2D::Create({
			.usages=PackFlags(rhi::TextureUsage::TransferDst, rhi::TextureUsage::Sample),
			.pixelFormat=PixelFormat::RGBA_FLOAT32,
			.width=512,
			.height=512,
			.layout=rhi::TextureLayout::Undefined
		});
		if(!curveTexture)
		{
			assert(false && "create curve texture failed");
			return false;
		}
		curveTexture.SyncTransitionLayout(rhi::TextureLayout::Undefined, rhi::TextureLayout::ShaderReadOnly);
		//curveTexture.GetOrCreateDefaultImageView();
		glyphTexture=rhi::Texture2D::Create(
			{
				.usages=PackFlags(rhi::TextureUsage::TransferDst, rhi::TextureUsage::Sample),
				.pixelFormat=PixelFormat::RGBA8888,
				.width=128,
				.height=128,
				.layout=rhi::TextureLayout::Undefined
			}
		);
		if(!glyphTexture)
		{
			assert(false && "create glyph texture failed");
			return false;
		}
		glyphTexture.SyncTransitionLayout(rhi::TextureLayout::Undefined, rhi::TextureLayout::ShaderReadOnly);
		//glyphTexture.GetOrCreateDefaultImageView();
        return true;
    }
}