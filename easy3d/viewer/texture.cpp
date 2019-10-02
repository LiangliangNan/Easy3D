/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <easy3d/viewer/texture.h>
#include "Texture_p.h"
#include "Texturehelper_p.h"
#include "qopenglfunctions.h"
#include <QtGui/qcolor.h>
#include <QtGui/qopenglcontext.h>
#include <QtCore/qdebug.h>
#include <private/qobject_p.h>
#include <private/qopenglcontext_p.h>



namespace easy3d {


//this is to work around GL_TEXTURE_WRAP_R_OES which also has 0x8072 as value
#if !defined(GL_TEXTURE_WRAP_R)
    #define GL_TEXTURE_WRAP_R 0x8072
#endif

TexturePrivate::TexturePrivate(Texture::Target textureTarget,
                                             Texture *qq)
    : q_ptr(qq),
      context(0),
      target(textureTarget),
      textureId(0),
      format(Texture::NoFormat),
      formatClass(Texture::NoFormatClass),
      requestedMipLevels(1),
      mipLevels(-1),
      layers(1),
      faces(1),
      samples(0),
      fixedSamplePositions(true),
      baseLevel(0),
      maxLevel(1000),
      depthStencilMode(Texture::DepthMode),
      comparisonFunction(Texture::CompareLessEqual),
      comparisonMode(Texture::CompareNone),
      minFilter(Texture::Nearest),
      magFilter(Texture::Nearest),
      maxAnisotropy(1.0f),
      minLevelOfDetail(-1000.0f),
      maxLevelOfDetail(1000.0f),
      levelOfDetailBias(0.0f),
      textureView(false),
      autoGenerateMipMaps(true),
      storageAllocated(false),
      texFuncs(0),
      functions(0)
{
    dimensions[0] = dimensions[1] = dimensions[2] = 1;

    switch (target) {
    case Texture::Target1D:
        bindingTarget = Texture::BindingTarget1D;
        break;
    case Texture::Target1DArray:
        bindingTarget = Texture::BindingTarget1DArray;
        break;
    case Texture::Target2D:
        bindingTarget = Texture::BindingTarget2D;
        break;
    case Texture::Target2DArray:
        bindingTarget = Texture::BindingTarget2DArray;
        break;
    case Texture::Target3D:
        bindingTarget = Texture::BindingTarget3D;
        break;
    case Texture::TargetCubeMap:
        bindingTarget = Texture::BindingTargetCubeMap;
        faces = 6;
        break;
    case Texture::TargetCubeMapArray:
        bindingTarget = Texture::BindingTargetCubeMapArray;
        faces = 6;
        break;
    case Texture::Target2DMultisample:
        bindingTarget = Texture::BindingTarget2DMultisample;
        break;
    case Texture::Target2DMultisampleArray:
        bindingTarget = Texture::BindingTarget2DMultisampleArray;
        break;
    case Texture::TargetRectangle:
        bindingTarget = Texture::BindingTargetRectangle;
        break;
    case Texture::TargetBuffer:
        bindingTarget = Texture::BindingTargetBuffer;
        break;
    }

    swizzleMask[0] = Texture::RedValue;
    swizzleMask[1] = Texture::GreenValue;
    swizzleMask[2] = Texture::BlueValue;
    swizzleMask[3] = Texture::AlphaValue;

    wrapModes[0] = wrapModes[1] = wrapModes[2] = target == Texture::TargetRectangle
        ? Texture::ClampToEdge : Texture::Repeat;
}

TexturePrivate::~TexturePrivate()
{
    destroy();
}

void TexturePrivate::initializeOpenGLFunctions()
{
    // If we already have a functions object, there is nothing to do
    if (texFuncs)
        return;

    // See if the context already has a suitable resource we can use.
    // If not create a functions object and add it to the context in case
    // others wish to use it too
    texFuncs = context->textureFunctions();
    if (!texFuncs) {
        texFuncs = new TextureHelper(context);
        context->setTextureFunctions(texFuncs);
    }
}

bool TexturePrivate::create()
{
    if (textureId != 0)
        return true;

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        qWarning("Requires a valid current OpenGL context.\n"
                 "Texture has not been created");
        return false;
    }
    context = ctx;
    functions = ctx->functions();

    // Resolve any functions we will need based upon context version and create the texture
    initializeOpenGLFunctions();

    // What features do we have?
    Texture::Feature feature = Texture::ImmutableStorage;
    while (feature != Texture::MaxFeatureFlag) {
        if (Texture::hasFeature(feature))
            features |= feature;
        feature = static_cast<Texture::Feature>(feature << 1);
    }

    functions->glGenTextures(1, &textureId);
    return textureId != 0;
}

void TexturePrivate::destroy()
{
    if (!textureId) {
        // not created or already destroyed
        return;
    }
    QOpenGLContext *currentContext = QOpenGLContext::currentContext();
    if (!currentContext) {
        qWarning("TexturePrivate::destroy() called without a current context.\n"
                 "Texture has not been destroyed");
        return;
    }
    if (!QOpenGLContext::areSharing(currentContext, context)) {

        qWarning("TexturePrivate::destroy() called but texture context %p"
                 " is not shared with current context %p.\n"
                 "Texture has not been destroyed",
                 static_cast<const void *>(context),
                 static_cast<const void *>(currentContext));
        return;
    }

    functions->glDeleteTextures(1, &textureId);

    context = 0;
    functions = 0;
    textureId = 0;
    format = Texture::NoFormat;
    formatClass = Texture::NoFormatClass;
    requestedMipLevels = 1;
    mipLevels = -1;
    layers = 1;
    faces = 1;
    samples = 0;
    fixedSamplePositions = true,
    baseLevel = 0;
    maxLevel = 1000;
    depthStencilMode = Texture::DepthMode;
    minFilter = Texture::Nearest;
    magFilter = Texture::Nearest;
    maxAnisotropy = 1.0f;
    minLevelOfDetail = -1000.0f;
    maxLevelOfDetail = 1000.0f;
    levelOfDetailBias = 0.0f;
    textureView = false;
    autoGenerateMipMaps = true;
    storageAllocated = false;
    texFuncs = 0;

    swizzleMask[0] = Texture::RedValue;
    swizzleMask[1] = Texture::GreenValue;
    swizzleMask[2] = Texture::BlueValue;
    swizzleMask[3] = Texture::AlphaValue;

    wrapModes[0] = wrapModes[1] = wrapModes[2] = target == Texture::TargetRectangle
        ? Texture::ClampToEdge : Texture::Repeat;
}

void TexturePrivate::bind()
{
    functions->glBindTexture(target, textureId);
}

void TexturePrivate::bind(uint unit, Texture::TextureUnitReset reset)
{
    GLint oldTextureUnit = 0;
    if (reset == Texture::ResetTextureUnit)
        functions->glGetIntegerv(GL_ACTIVE_TEXTURE, &oldTextureUnit);

    texFuncs->glActiveTexture(GL_TEXTURE0 + unit);
    functions->glBindTexture(target, textureId);

    if (reset == Texture::ResetTextureUnit)
        texFuncs->glActiveTexture(GL_TEXTURE0 + oldTextureUnit);
}

void TexturePrivate::release()
{
    functions->glBindTexture(target, 0);
}

void TexturePrivate::release(uint unit, Texture::TextureUnitReset reset)
{
    GLint oldTextureUnit = 0;
    if (reset == Texture::ResetTextureUnit)
        functions->glGetIntegerv(GL_ACTIVE_TEXTURE, &oldTextureUnit);

    texFuncs->glActiveTexture(GL_TEXTURE0 + unit);
    functions->glBindTexture(target, 0);

    if (reset == Texture::ResetTextureUnit)
        texFuncs->glActiveTexture(GL_TEXTURE0 + oldTextureUnit);
}

bool TexturePrivate::isBound() const
{
    GLint boundTextureId = 0;
    functions->glGetIntegerv(bindingTarget, &boundTextureId);
    return (static_cast<unsigned int>(boundTextureId) == textureId);
}

bool TexturePrivate::isBound(uint unit) const
{
    GLint oldTextureUnit = 0;
    functions->glGetIntegerv(GL_ACTIVE_TEXTURE, &oldTextureUnit);

    GLint boundTextureId = 0;
    texFuncs->glActiveTexture(GL_TEXTURE0 + unit);
    functions->glGetIntegerv(bindingTarget, &boundTextureId);
    bool result = (static_cast<unsigned int>(boundTextureId) == textureId);

    texFuncs->glActiveTexture(GL_TEXTURE0 + oldTextureUnit);
    return result;
}

int TexturePrivate::evaluateMipLevels() const
{
    switch (target) {
    case Texture::Target1D:
    case Texture::Target1DArray:
    case Texture::Target2D:
    case Texture::Target2DArray:
    case Texture::Target3D:
    case Texture::TargetCubeMap:
    case Texture::TargetCubeMapArray:
        return qMin(maximumMipLevelCount(), qMax(1, requestedMipLevels));

    case Texture::TargetRectangle:
    case Texture::Target2DMultisample:
    case Texture::Target2DMultisampleArray:
    case Texture::TargetBuffer:
    default:
        return 1;
    }
}

static bool isSizedTextureFormat(Texture::TextureFormat internalFormat)
{
    switch (internalFormat) {
    case Texture::NoFormat:
        return false;

    case Texture::R8_UNorm:
    case Texture::RG8_UNorm:
    case Texture::RGB8_UNorm:
    case Texture::RGBA8_UNorm:
    case Texture::R16_UNorm:
    case Texture::RG16_UNorm:
    case Texture::RGB16_UNorm:
    case Texture::RGBA16_UNorm:
    case Texture::R8_SNorm:
    case Texture::RG8_SNorm:
    case Texture::RGB8_SNorm:
    case Texture::RGBA8_SNorm:
    case Texture::R16_SNorm:
    case Texture::RG16_SNorm:
    case Texture::RGB16_SNorm:
    case Texture::RGBA16_SNorm:
    case Texture::R8U:
    case Texture::RG8U:
    case Texture::RGB8U:
    case Texture::RGBA8U:
    case Texture::R16U:
    case Texture::RG16U:
    case Texture::RGB16U:
    case Texture::RGBA16U:
    case Texture::R32U:
    case Texture::RG32U:
    case Texture::RGB32U:
    case Texture::RGBA32U:
    case Texture::R8I:
    case Texture::RG8I:
    case Texture::RGB8I:
    case Texture::RGBA8I:
    case Texture::R16I:
    case Texture::RG16I:
    case Texture::RGB16I:
    case Texture::RGBA16I:
    case Texture::R32I:
    case Texture::RG32I:
    case Texture::RGB32I:
    case Texture::RGBA32I:
    case Texture::R16F:
    case Texture::RG16F:
    case Texture::RGB16F:
    case Texture::RGBA16F:
    case Texture::R32F:
    case Texture::RG32F:
    case Texture::RGB32F:
    case Texture::RGBA32F:
    case Texture::RGB9E5:
    case Texture::RG11B10F:
    case Texture::RG3B2:
    case Texture::R5G6B5:
    case Texture::RGB5A1:
    case Texture::RGBA4:
    case Texture::RGB10A2:

    case Texture::D16:
    case Texture::D24:
    case Texture::D32:
    case Texture::D32F:

    case Texture::D24S8:
    case Texture::D32FS8X24:

    case Texture::S8:

    case Texture::RGB_DXT1:
    case Texture::RGBA_DXT1:
    case Texture::RGBA_DXT3:
    case Texture::RGBA_DXT5:
    case Texture::R_ATI1N_UNorm:
    case Texture::R_ATI1N_SNorm:
    case Texture::RG_ATI2N_UNorm:
    case Texture::RG_ATI2N_SNorm:
    case Texture::RGB_BP_UNSIGNED_FLOAT:
    case Texture::RGB_BP_SIGNED_FLOAT:
    case Texture::RGB_BP_UNorm:
    case Texture::SRGB8:
    case Texture::SRGB8_Alpha8:
    case Texture::SRGB_DXT1:
    case Texture::SRGB_Alpha_DXT1:
    case Texture::SRGB_Alpha_DXT3:
    case Texture::SRGB_Alpha_DXT5:
    case Texture::SRGB_BP_UNorm:
    case Texture::R11_EAC_UNorm:
    case Texture::R11_EAC_SNorm:
    case Texture::RG11_EAC_UNorm:
    case Texture::RG11_EAC_SNorm:
    case Texture::RGB8_ETC2:
    case Texture::SRGB8_ETC2:
    case Texture::RGB8_PunchThrough_Alpha1_ETC2:
    case Texture::SRGB8_PunchThrough_Alpha1_ETC2:
    case Texture::RGBA8_ETC2_EAC:
    case Texture::SRGB8_Alpha8_ETC2_EAC:
    case Texture::RGBA_ASTC_4x4:
    case Texture::RGBA_ASTC_5x4:
    case Texture::RGBA_ASTC_5x5:
    case Texture::RGBA_ASTC_6x5:
    case Texture::RGBA_ASTC_6x6:
    case Texture::RGBA_ASTC_8x5:
    case Texture::RGBA_ASTC_8x6:
    case Texture::RGBA_ASTC_8x8:
    case Texture::RGBA_ASTC_10x5:
    case Texture::RGBA_ASTC_10x6:
    case Texture::RGBA_ASTC_10x8:
    case Texture::RGBA_ASTC_10x10:
    case Texture::RGBA_ASTC_12x10:
    case Texture::RGBA_ASTC_12x12:
    case Texture::SRGB8_Alpha8_ASTC_4x4:
    case Texture::SRGB8_Alpha8_ASTC_5x4:
    case Texture::SRGB8_Alpha8_ASTC_5x5:
    case Texture::SRGB8_Alpha8_ASTC_6x5:
    case Texture::SRGB8_Alpha8_ASTC_6x6:
    case Texture::SRGB8_Alpha8_ASTC_8x5:
    case Texture::SRGB8_Alpha8_ASTC_8x6:
    case Texture::SRGB8_Alpha8_ASTC_8x8:
    case Texture::SRGB8_Alpha8_ASTC_10x5:
    case Texture::SRGB8_Alpha8_ASTC_10x6:
    case Texture::SRGB8_Alpha8_ASTC_10x8:
    case Texture::SRGB8_Alpha8_ASTC_10x10:
    case Texture::SRGB8_Alpha8_ASTC_12x10:
    case Texture::SRGB8_Alpha8_ASTC_12x12:
        return true;

    case Texture::RGB8_ETC1:
        return false;

    case Texture::DepthFormat:
    case Texture::AlphaFormat:

    case Texture::RGBFormat:
    case Texture::RGBAFormat:

    case Texture::LuminanceFormat:

    case Texture::LuminanceAlphaFormat:
        return false;
    }

    Q_UNREACHABLE();
    return false;
}

static bool isTextureTargetMultisample(Texture::Target target)
{
    switch (target) {
    case Texture::Target1D:
    case Texture::Target1DArray:
    case Texture::Target2D:
    case Texture::Target2DArray:
    case Texture::Target3D:
    case Texture::TargetCubeMap:
    case Texture::TargetCubeMapArray:
        return false;

    case Texture::Target2DMultisample:
    case Texture::Target2DMultisampleArray:
        return true;

    case Texture::TargetRectangle:
    case Texture::TargetBuffer:
        return false;
    }

    Q_UNREACHABLE();
    return false;
}

bool TexturePrivate::isUsingImmutableStorage() const
{
    // Use immutable storage whenever possible, falling back to mutable
    // Note that if multisample textures are not supported at all, we'll still fail into
    // the mutable storage allocation
    return isSizedTextureFormat(format)
            && (isTextureTargetMultisample(target)
                ? features.testFlag(Texture::ImmutableMultisampleStorage)
                : features.testFlag(Texture::ImmutableStorage));
}

void TexturePrivate::allocateStorage(Texture::PixelFormat pixelFormat, Texture::PixelType pixelType)
{
    // Resolve the actual number of mipmap levels we can use
    mipLevels = evaluateMipLevels();

    if (isUsingImmutableStorage())
        allocateImmutableStorage();
    else
        allocateMutableStorage(pixelFormat, pixelType);
}

static Texture::PixelFormat pixelFormatCompatibleWithInternalFormat(Texture::TextureFormat internalFormat)
{
    switch (internalFormat) {
    case Texture::NoFormat:
        return Texture::NoSourceFormat;

    case Texture::R8_UNorm:
        return Texture::Red;

    case Texture::RG8_UNorm:
        return Texture::RG;

    case Texture::RGB8_UNorm:
        return Texture::RGB;

    case Texture::RGBA8_UNorm:
        return Texture::RGBA;

    case Texture::R16_UNorm:
        return Texture::Red;

    case Texture::RG16_UNorm:
        return Texture::RG;

    case Texture::RGB16_UNorm:
        return Texture::RGB;

    case Texture::RGBA16_UNorm:
        return Texture::RGBA;

    case Texture::R8_SNorm:
        return Texture::Red;

    case Texture::RG8_SNorm:
        return Texture::RG;

    case Texture::RGB8_SNorm:
        return Texture::RGB;

    case Texture::RGBA8_SNorm:
        return Texture::RGBA;

    case Texture::R16_SNorm:
        return Texture::Red;

    case Texture::RG16_SNorm:
        return Texture::RG;

    case Texture::RGB16_SNorm:
        return Texture::RGB;

    case Texture::RGBA16_SNorm:
        return Texture::RGBA;

    case Texture::R8U:
        return Texture::Red_Integer;

    case Texture::RG8U:
        return Texture::RG_Integer;

    case Texture::RGB8U:
        return Texture::RGB_Integer;

    case Texture::RGBA8U:
        return Texture::RGBA_Integer;

    case Texture::R16U:
        return Texture::Red_Integer;

    case Texture::RG16U:
        return Texture::RG_Integer;

    case Texture::RGB16U:
        return Texture::RGB_Integer;

    case Texture::RGBA16U:
        return Texture::RGBA_Integer;

    case Texture::R32U:
        return Texture::Red_Integer;

    case Texture::RG32U:
        return Texture::RG_Integer;

    case Texture::RGB32U:
        return Texture::RGB_Integer;

    case Texture::RGBA32U:
        return Texture::RGBA_Integer;

    case Texture::R8I:
        return Texture::Red_Integer;

    case Texture::RG8I:
        return Texture::RG_Integer;

    case Texture::RGB8I:
        return Texture::RGB_Integer;

    case Texture::RGBA8I:
        return Texture::RGBA_Integer;

    case Texture::R16I:
        return Texture::Red_Integer;

    case Texture::RG16I:
        return Texture::RG_Integer;

    case Texture::RGB16I:
        return Texture::RGB_Integer;

    case Texture::RGBA16I:
        return Texture::RGBA_Integer;

    case Texture::R32I:
        return Texture::Red_Integer;

    case Texture::RG32I:
        return Texture::RG_Integer;

    case Texture::RGB32I:
        return Texture::RGB_Integer;

    case Texture::RGBA32I:
        return Texture::RGBA_Integer;

    case Texture::R16F:
        return Texture::Red;

    case Texture::RG16F:
        return Texture::RG;

    case Texture::RGB16F:
        return Texture::RGB;

    case Texture::RGBA16F:
        return Texture::RGBA;

    case Texture::R32F:
        return Texture::Red;

    case Texture::RG32F:
        return Texture::RG;

    case Texture::RGB32F:
        return Texture::RGB;

    case Texture::RGBA32F:
        return Texture::RGBA;

    case Texture::RGB9E5:
        return Texture::RGB;

    case Texture::RG11B10F:
        return Texture::RGB;

    case Texture::RG3B2:
        return Texture::RGB;

    case Texture::R5G6B5:
        return Texture::RGB;

    case Texture::RGB5A1:
        return Texture::RGBA;

    case Texture::RGBA4:
        return Texture::RGBA;

    case Texture::RGB10A2:
        return Texture::RGBA;

    case Texture::D16:
    case Texture::D24:
    case Texture::D32:
    case Texture::D32F:
        return Texture::Depth;

    case Texture::D24S8:
    case Texture::D32FS8X24:
        return Texture::DepthStencil;

    case Texture::S8:
        return Texture::Stencil;

    case Texture::RGB_DXT1:
    case Texture::RGBA_DXT1:
    case Texture::RGBA_DXT3:
    case Texture::RGBA_DXT5:
    case Texture::R_ATI1N_UNorm:
    case Texture::R_ATI1N_SNorm:
    case Texture::RG_ATI2N_UNorm:
    case Texture::RG_ATI2N_SNorm:
    case Texture::RGB_BP_UNSIGNED_FLOAT:
    case Texture::RGB_BP_SIGNED_FLOAT:
    case Texture::RGB_BP_UNorm:
    case Texture::SRGB8:
    case Texture::SRGB8_Alpha8:
    case Texture::SRGB_DXT1:
    case Texture::SRGB_Alpha_DXT1:
    case Texture::SRGB_Alpha_DXT3:
    case Texture::SRGB_Alpha_DXT5:
    case Texture::SRGB_BP_UNorm:
    case Texture::RGB8_ETC1:
        return Texture::RGBA;

    case Texture::R11_EAC_UNorm:
    case Texture::R11_EAC_SNorm:
        return Texture::Red;

    case Texture::RG11_EAC_UNorm:
    case Texture::RG11_EAC_SNorm:
        return Texture::RG;

    case Texture::RGB8_ETC2:
    case Texture::SRGB8_ETC2:
        return Texture::RGB;

    case Texture::RGB8_PunchThrough_Alpha1_ETC2:
    case Texture::SRGB8_PunchThrough_Alpha1_ETC2:
        return Texture::RGBA;

    case Texture::RGBA8_ETC2_EAC:
    case Texture::SRGB8_Alpha8_ETC2_EAC:
        return Texture::RGBA;

    case Texture::RGBA_ASTC_4x4:
    case Texture::RGBA_ASTC_5x4:
    case Texture::RGBA_ASTC_5x5:
    case Texture::RGBA_ASTC_6x5:
    case Texture::RGBA_ASTC_6x6:
    case Texture::RGBA_ASTC_8x5:
    case Texture::RGBA_ASTC_8x6:
    case Texture::RGBA_ASTC_8x8:
    case Texture::RGBA_ASTC_10x5:
    case Texture::RGBA_ASTC_10x6:
    case Texture::RGBA_ASTC_10x8:
    case Texture::RGBA_ASTC_10x10:
    case Texture::RGBA_ASTC_12x10:
    case Texture::RGBA_ASTC_12x12:
    case Texture::SRGB8_Alpha8_ASTC_4x4:
    case Texture::SRGB8_Alpha8_ASTC_5x4:
    case Texture::SRGB8_Alpha8_ASTC_5x5:
    case Texture::SRGB8_Alpha8_ASTC_6x5:
    case Texture::SRGB8_Alpha8_ASTC_6x6:
    case Texture::SRGB8_Alpha8_ASTC_8x5:
    case Texture::SRGB8_Alpha8_ASTC_8x6:
    case Texture::SRGB8_Alpha8_ASTC_8x8:
    case Texture::SRGB8_Alpha8_ASTC_10x5:
    case Texture::SRGB8_Alpha8_ASTC_10x6:
    case Texture::SRGB8_Alpha8_ASTC_10x8:
    case Texture::SRGB8_Alpha8_ASTC_10x10:
    case Texture::SRGB8_Alpha8_ASTC_12x10:
    case Texture::SRGB8_Alpha8_ASTC_12x12:
        return Texture::RGBA;

    case Texture::DepthFormat:
        return Texture::Depth;

    case Texture::AlphaFormat:
        return Texture::Alpha;

    case Texture::RGBFormat:
        return Texture::RGB;

    case Texture::RGBAFormat:
        return Texture::RGBA;

    case Texture::LuminanceFormat:
        return Texture::Luminance;

    case Texture::LuminanceAlphaFormat:
        return Texture::LuminanceAlpha;
    }

    Q_UNREACHABLE();
    return Texture::NoSourceFormat;
}

static Texture::PixelType pixelTypeCompatibleWithInternalFormat(Texture::TextureFormat internalFormat)
{
    switch (internalFormat) {
    case Texture::NoFormat:
        return Texture::NoPixelType;

    case Texture::R8_UNorm:
    case Texture::RG8_UNorm:
    case Texture::RGB8_UNorm:
    case Texture::RGBA8_UNorm:
    case Texture::R16_UNorm:
    case Texture::RG16_UNorm:
    case Texture::RGB16_UNorm:
    case Texture::RGBA16_UNorm:
        return Texture::UInt8;

    case Texture::R8_SNorm:
    case Texture::RG8_SNorm:
    case Texture::RGB8_SNorm:
    case Texture::RGBA8_SNorm:
    case Texture::R16_SNorm:
    case Texture::RG16_SNorm:
    case Texture::RGB16_SNorm:
    case Texture::RGBA16_SNorm:
        return Texture::Int8;

    case Texture::R8U:
    case Texture::RG8U:
    case Texture::RGB8U:
    case Texture::RGBA8U:
    case Texture::R16U:
    case Texture::RG16U:
    case Texture::RGB16U:
    case Texture::RGBA16U:
    case Texture::R32U:
    case Texture::RG32U:
    case Texture::RGB32U:
    case Texture::RGBA32U:
        return Texture::UInt8;

    case Texture::R8I:
    case Texture::RG8I:
    case Texture::RGB8I:
    case Texture::RGBA8I:
    case Texture::R16I:
    case Texture::RG16I:
    case Texture::RGB16I:
    case Texture::RGBA16I:
    case Texture::R32I:
    case Texture::RG32I:
    case Texture::RGB32I:
    case Texture::RGBA32I:
        return Texture::Int8;

    case Texture::R16F:
    case Texture::RG16F:
    case Texture::RGB16F:
    case Texture::RGBA16F:
        return Texture::Float16;

    case Texture::R32F:
    case Texture::RG32F:
    case Texture::RGB32F:
    case Texture::RGBA32F:
        return Texture::Float32;

    case Texture::RGB9E5:
        return Texture::UInt16_RGB5A1_Rev;

    case Texture::RG11B10F:
        return Texture::UInt32_RG11B10F;

    case Texture::RG3B2:
        return Texture::UInt8_RG3B2;

    case Texture::R5G6B5:
        return Texture::UInt16_R5G6B5;

    case Texture::RGB5A1:
        return Texture::UInt16_RGB5A1;

    case Texture::RGBA4:
        return Texture::UInt16_RGBA4;

    case Texture::RGB10A2:
        return Texture::UInt32_RGB10A2;

    case Texture::D16:
        return Texture::UInt16;

    case Texture::D24:
    case Texture::D32:
        return Texture::UInt32;

    case Texture::D32F:
        return Texture::Float32;

    case Texture::D24S8:
        return Texture::UInt32_D24S8;

    case Texture::D32FS8X24:
        return Texture::Float32_D32_UInt32_S8_X24;

    case Texture::S8:
        return Texture::UInt8;

    case Texture::RGB_DXT1:
    case Texture::RGBA_DXT1:
    case Texture::RGBA_DXT3:
    case Texture::RGBA_DXT5:
    case Texture::R_ATI1N_UNorm:
    case Texture::R_ATI1N_SNorm:
    case Texture::RG_ATI2N_UNorm:
    case Texture::RG_ATI2N_SNorm:
    case Texture::RGB_BP_UNSIGNED_FLOAT:
    case Texture::RGB_BP_SIGNED_FLOAT:
    case Texture::RGB_BP_UNorm:
    case Texture::SRGB8:
    case Texture::SRGB8_Alpha8:
    case Texture::SRGB_DXT1:
    case Texture::SRGB_Alpha_DXT1:
    case Texture::SRGB_Alpha_DXT3:
    case Texture::SRGB_Alpha_DXT5:
    case Texture::SRGB_BP_UNorm:
    case Texture::R11_EAC_UNorm:
    case Texture::R11_EAC_SNorm:
    case Texture::RG11_EAC_UNorm:
    case Texture::RG11_EAC_SNorm:
    case Texture::RGB8_ETC2:
    case Texture::SRGB8_ETC2:
    case Texture::RGB8_PunchThrough_Alpha1_ETC2:
    case Texture::SRGB8_PunchThrough_Alpha1_ETC2:
    case Texture::RGBA8_ETC2_EAC:
    case Texture::SRGB8_Alpha8_ETC2_EAC:
    case Texture::RGB8_ETC1:
    case Texture::RGBA_ASTC_4x4:
    case Texture::RGBA_ASTC_5x4:
    case Texture::RGBA_ASTC_5x5:
    case Texture::RGBA_ASTC_6x5:
    case Texture::RGBA_ASTC_6x6:
    case Texture::RGBA_ASTC_8x5:
    case Texture::RGBA_ASTC_8x6:
    case Texture::RGBA_ASTC_8x8:
    case Texture::RGBA_ASTC_10x5:
    case Texture::RGBA_ASTC_10x6:
    case Texture::RGBA_ASTC_10x8:
    case Texture::RGBA_ASTC_10x10:
    case Texture::RGBA_ASTC_12x10:
    case Texture::RGBA_ASTC_12x12:
    case Texture::SRGB8_Alpha8_ASTC_4x4:
    case Texture::SRGB8_Alpha8_ASTC_5x4:
    case Texture::SRGB8_Alpha8_ASTC_5x5:
    case Texture::SRGB8_Alpha8_ASTC_6x5:
    case Texture::SRGB8_Alpha8_ASTC_6x6:
    case Texture::SRGB8_Alpha8_ASTC_8x5:
    case Texture::SRGB8_Alpha8_ASTC_8x6:
    case Texture::SRGB8_Alpha8_ASTC_8x8:
    case Texture::SRGB8_Alpha8_ASTC_10x5:
    case Texture::SRGB8_Alpha8_ASTC_10x6:
    case Texture::SRGB8_Alpha8_ASTC_10x8:
    case Texture::SRGB8_Alpha8_ASTC_10x10:
    case Texture::SRGB8_Alpha8_ASTC_12x10:
    case Texture::SRGB8_Alpha8_ASTC_12x12:
        return Texture::UInt8;

    case Texture::DepthFormat:
        return Texture::UInt32;

    case Texture::AlphaFormat:
    case Texture::RGBFormat:
    case Texture::RGBAFormat:
    case Texture::LuminanceFormat:
    case Texture::LuminanceAlphaFormat:
        return Texture::UInt8;
    }

    Q_UNREACHABLE();
    return Texture::NoPixelType;
}

static bool isCompressedFormat(Texture::TextureFormat internalFormat)
{
    switch (internalFormat) {
    case Texture::NoFormat:

    case Texture::R8_UNorm:
    case Texture::RG8_UNorm:
    case Texture::RGB8_UNorm:
    case Texture::RGBA8_UNorm:
    case Texture::R16_UNorm:
    case Texture::RG16_UNorm:
    case Texture::RGB16_UNorm:
    case Texture::RGBA16_UNorm:
    case Texture::R8_SNorm:
    case Texture::RG8_SNorm:
    case Texture::RGB8_SNorm:
    case Texture::RGBA8_SNorm:
    case Texture::R16_SNorm:
    case Texture::RG16_SNorm:
    case Texture::RGB16_SNorm:
    case Texture::RGBA16_SNorm:
    case Texture::R8U:
    case Texture::RG8U:
    case Texture::RGB8U:
    case Texture::RGBA8U:
    case Texture::R16U:
    case Texture::RG16U:
    case Texture::RGB16U:
    case Texture::RGBA16U:
    case Texture::R32U:
    case Texture::RG32U:
    case Texture::RGB32U:
    case Texture::RGBA32U:
    case Texture::R8I:
    case Texture::RG8I:
    case Texture::RGB8I:
    case Texture::RGBA8I:
    case Texture::R16I:
    case Texture::RG16I:
    case Texture::RGB16I:
    case Texture::RGBA16I:
    case Texture::R32I:
    case Texture::RG32I:
    case Texture::RGB32I:
    case Texture::RGBA32I:
    case Texture::R16F:
    case Texture::RG16F:
    case Texture::RGB16F:
    case Texture::RGBA16F:
    case Texture::R32F:
    case Texture::RG32F:
    case Texture::RGB32F:
    case Texture::RGBA32F:
    case Texture::RGB9E5:
    case Texture::RG11B10F:
    case Texture::RG3B2:
    case Texture::R5G6B5:
    case Texture::RGB5A1:
    case Texture::RGBA4:
    case Texture::RGB10A2:

    case Texture::D16:
    case Texture::D24:
    case Texture::D32:
    case Texture::D32F:

    case Texture::D24S8:
    case Texture::D32FS8X24:

    case Texture::S8:
        return false;

    case Texture::RGB_DXT1:
    case Texture::RGBA_DXT1:
    case Texture::RGBA_DXT3:
    case Texture::RGBA_DXT5:
    case Texture::R_ATI1N_UNorm:
    case Texture::R_ATI1N_SNorm:
    case Texture::RG_ATI2N_UNorm:
    case Texture::RG_ATI2N_SNorm:
    case Texture::RGB_BP_UNSIGNED_FLOAT:
    case Texture::RGB_BP_SIGNED_FLOAT:
    case Texture::RGB_BP_UNorm:
    case Texture::SRGB8:
    case Texture::SRGB8_Alpha8:
    case Texture::SRGB_DXT1:
    case Texture::SRGB_Alpha_DXT1:
    case Texture::SRGB_Alpha_DXT3:
    case Texture::SRGB_Alpha_DXT5:
    case Texture::SRGB_BP_UNorm:
    case Texture::R11_EAC_UNorm:
    case Texture::R11_EAC_SNorm:
    case Texture::RG11_EAC_UNorm:
    case Texture::RG11_EAC_SNorm:
    case Texture::RGB8_ETC2:
    case Texture::SRGB8_ETC2:
    case Texture::RGB8_PunchThrough_Alpha1_ETC2:
    case Texture::SRGB8_PunchThrough_Alpha1_ETC2:
    case Texture::RGBA8_ETC2_EAC:
    case Texture::SRGB8_Alpha8_ETC2_EAC:
    case Texture::RGB8_ETC1:
    case Texture::RGBA_ASTC_4x4:
    case Texture::RGBA_ASTC_5x4:
    case Texture::RGBA_ASTC_5x5:
    case Texture::RGBA_ASTC_6x5:
    case Texture::RGBA_ASTC_6x6:
    case Texture::RGBA_ASTC_8x5:
    case Texture::RGBA_ASTC_8x6:
    case Texture::RGBA_ASTC_8x8:
    case Texture::RGBA_ASTC_10x5:
    case Texture::RGBA_ASTC_10x6:
    case Texture::RGBA_ASTC_10x8:
    case Texture::RGBA_ASTC_10x10:
    case Texture::RGBA_ASTC_12x10:
    case Texture::RGBA_ASTC_12x12:
    case Texture::SRGB8_Alpha8_ASTC_4x4:
    case Texture::SRGB8_Alpha8_ASTC_5x4:
    case Texture::SRGB8_Alpha8_ASTC_5x5:
    case Texture::SRGB8_Alpha8_ASTC_6x5:
    case Texture::SRGB8_Alpha8_ASTC_6x6:
    case Texture::SRGB8_Alpha8_ASTC_8x5:
    case Texture::SRGB8_Alpha8_ASTC_8x6:
    case Texture::SRGB8_Alpha8_ASTC_8x8:
    case Texture::SRGB8_Alpha8_ASTC_10x5:
    case Texture::SRGB8_Alpha8_ASTC_10x6:
    case Texture::SRGB8_Alpha8_ASTC_10x8:
    case Texture::SRGB8_Alpha8_ASTC_10x10:
    case Texture::SRGB8_Alpha8_ASTC_12x10:
    case Texture::SRGB8_Alpha8_ASTC_12x12:
        return true;

    case Texture::DepthFormat:
    case Texture::AlphaFormat:
    case Texture::RGBFormat:
    case Texture::RGBAFormat:
    case Texture::LuminanceFormat:
    case Texture::LuminanceAlphaFormat:
        return false;
    }

    Q_UNREACHABLE();
    return false;
}

void TexturePrivate::allocateMutableStorage(Texture::PixelFormat pixelFormat, Texture::PixelType pixelType)
{
    // There is no way to allocate mutable storage for compressed textures in in
    // versions older than OpenGL 3.1 and OpenGL ES 3.0, because the older specs
    // do not mandate accepting null data pointers for glCompressedTexImage*D,
    // unlike glTexImage*D (which in turn does not accept compressed formats).
    if (isCompressedFormat(format)) {
        storageAllocated = true;
        return;
    }

    switch (target) {
    case Texture::TargetBuffer:
        // Buffer textures get their storage from an external OpenGL buffer
        qWarning("Buffer textures do not allocate storage");
        return;

    case Texture::Target1D:
        if (features.testFlag(Texture::Texture1D)) {
            for (int level = 0; level < mipLevels; ++level)
                texFuncs->glTextureImage1D(textureId, target, bindingTarget, level, format,
                                           mipLevelSize(level, dimensions[0]),
                                           0,
                                           pixelFormat, pixelType, 0);
        } else {
            qWarning("1D textures are not supported");
            return;
        }
        break;

    case Texture::Target1DArray:
        if (features.testFlag(Texture::Texture1D)
                && features.testFlag(Texture::TextureArrays)) {
            for (int level = 0; level < mipLevels; ++level)
                texFuncs->glTextureImage2D(textureId, target, bindingTarget, level, format,
                                           mipLevelSize(level, dimensions[0]),
                                           layers,
                                           0,
                                           pixelFormat, pixelType, 0);
        } else {
            qWarning("1D array textures are not supported");
            return;
        }
        break;

    case Texture::Target2D:
    case Texture::TargetRectangle:
        for (int level = 0; level < mipLevels; ++level)
            texFuncs->glTextureImage2D(textureId, target, bindingTarget, level, format,
                                       mipLevelSize(level, dimensions[0]),
                                       mipLevelSize(level, dimensions[1]),
                                       0,
                                       pixelFormat, pixelType, 0);
        break;

    case Texture::TargetCubeMap: {
        // Cubemaps are the odd one out. We have to allocate storage for each
        // face and miplevel using the special cubemap face targets rather than
        // GL_TARGET_CUBEMAP.
        const Texture::CubeMapFace faceTargets[] = {
            Texture::CubeMapPositiveX, Texture::CubeMapNegativeX,
            Texture::CubeMapPositiveY, Texture::CubeMapNegativeY,
            Texture::CubeMapPositiveZ, Texture::CubeMapNegativeZ
        };

        for (int faceTarget = 0; faceTarget < 6; ++faceTarget) {
            for (int level = 0; level < mipLevels; ++level) {
                texFuncs->glTextureImage2D(textureId, faceTargets[faceTarget], bindingTarget,
                                           level, format,
                                           mipLevelSize(level, dimensions[0]),
                                           mipLevelSize(level, dimensions[1]),
                                           0,
                                           pixelFormat, pixelType, 0);
            }
        }
        break;
    }

    case Texture::Target2DArray:
        if (features.testFlag(Texture::TextureArrays)) {
            for (int level = 0; level < mipLevels; ++level)
                texFuncs->glTextureImage3D(textureId, target, bindingTarget, level, format,
                                           mipLevelSize(level, dimensions[0]),
                                           mipLevelSize(level, dimensions[1]),
                                           layers,
                                           0,
                                           pixelFormat, pixelType, 0);
        } else {
            qWarning("Array textures are not supported");
            return;
        }
        break;

    case Texture::TargetCubeMapArray:
        // Cubemap arrays must specify number of layer-faces (6 * layers) as depth parameter
        if (features.testFlag(Texture::TextureCubeMapArrays)) {
            for (int level = 0; level < mipLevels; ++level)
                texFuncs->glTextureImage3D(textureId, target, bindingTarget, level, format,
                                           mipLevelSize(level, dimensions[0]),
                                           mipLevelSize(level, dimensions[1]),
                                           6 * layers,
                                           0,
                                           pixelFormat, pixelType, 0);
        } else {
            qWarning("Cubemap Array textures are not supported");
            return;
        }
        break;

    case Texture::Target3D:
        if (features.testFlag(Texture::Texture3D)) {
            for (int level = 0; level < mipLevels; ++level)
                texFuncs->glTextureImage3D(textureId, target, bindingTarget, level, format,
                                           mipLevelSize(level, dimensions[0]),
                                           mipLevelSize(level, dimensions[1]),
                                           mipLevelSize(level, dimensions[2]),
                                           0,
                                           pixelFormat, pixelType, 0);
        } else {
            qWarning("3D textures are not supported");
            return;
        }
        break;

    case Texture::Target2DMultisample:
        if (features.testFlag(Texture::TextureMultisample)) {
            texFuncs->glTextureImage2DMultisample(textureId, target, bindingTarget, samples, format,
                                                  dimensions[0], dimensions[1],
                                                  fixedSamplePositions);
        } else {
            qWarning("Multisample textures are not supported");
            return;
        }
        break;

    case Texture::Target2DMultisampleArray:
        if (features.testFlag(Texture::TextureMultisample)
                && features.testFlag(Texture::TextureArrays)) {
            texFuncs->glTextureImage3DMultisample(textureId, target, bindingTarget, samples, format,
                                                  dimensions[0], dimensions[1], layers,
                                                  fixedSamplePositions);
        } else {
            qWarning("Multisample array textures are not supported");
            return;
        }
        break;
    }

    storageAllocated = true;
}

void TexturePrivate::allocateImmutableStorage()
{
    switch (target) {
    case Texture::TargetBuffer:
        // Buffer textures get their storage from an external OpenGL buffer
        qWarning("Buffer textures do not allocate storage");
        return;

    case Texture::Target1D:
        if (features.testFlag(Texture::Texture1D)) {
            texFuncs->glTextureStorage1D(textureId, target, bindingTarget, mipLevels, format,
                                         dimensions[0]);
        } else {
            qWarning("1D textures are not supported");
            return;
        }
        break;

    case Texture::Target1DArray:
        if (features.testFlag(Texture::Texture1D)
                && features.testFlag(Texture::TextureArrays)) {
            texFuncs->glTextureStorage2D(textureId, target, bindingTarget, mipLevels, format,
                                         dimensions[0], layers);
        } else {
            qWarning("1D array textures are not supported");
            return;
        }
        break;

    case Texture::Target2D:
    case Texture::TargetCubeMap:
    case Texture::TargetRectangle:
        texFuncs->glTextureStorage2D(textureId, target, bindingTarget, mipLevels, format,
                                     dimensions[0], dimensions[1]);
        break;

    case Texture::Target2DArray:
        if (features.testFlag(Texture::TextureArrays)) {
            texFuncs->glTextureStorage3D(textureId, target, bindingTarget, mipLevels, format,
                                         dimensions[0], dimensions[1], layers);
        } else {
            qWarning("Array textures are not supported");
            return;
        }
        break;

    case Texture::TargetCubeMapArray:
        // Cubemap arrays must specify number of layer-faces (6 * layers) as depth parameter
        if (features.testFlag(Texture::TextureCubeMapArrays)) {
            texFuncs->glTextureStorage3D(textureId, target, bindingTarget, mipLevels, format,
                                         dimensions[0], dimensions[1], 6 * layers);
        } else {
            qWarning("Cubemap Array textures are not supported");
            return;
        }
        break;

    case Texture::Target3D:
        if (features.testFlag(Texture::Texture3D)) {
            texFuncs->glTextureStorage3D(textureId, target, bindingTarget, mipLevels, format,
                                         dimensions[0], dimensions[1], dimensions[2]);
        } else {
            qWarning("3D textures are not supported");
            return;
        }
        break;

    case Texture::Target2DMultisample:
        if (features.testFlag(Texture::ImmutableMultisampleStorage)) {
            texFuncs->glTextureStorage2DMultisample(textureId, target, bindingTarget, samples, format,
                                                    dimensions[0], dimensions[1],
                                                    fixedSamplePositions);
        } else {
            qWarning("Multisample textures are not supported");
            return;
        }
        break;

    case Texture::Target2DMultisampleArray:
        if (features.testFlag(Texture::ImmutableMultisampleStorage)
                && features.testFlag(Texture::TextureArrays)) {
            texFuncs->glTextureStorage3DMultisample(textureId, target, bindingTarget, samples, format,
                                                    dimensions[0], dimensions[1], layers,
                                                    fixedSamplePositions);
        } else {
            qWarning("Multisample array textures are not supported");
            return;
        }
        break;
    }

    storageAllocated = true;
}

void TexturePrivate::setData(int mipLevel, int layer, int layerCount, Texture::CubeMapFace cubeFace,
                                    Texture::PixelFormat sourceFormat, Texture::PixelType sourceType,
                                    const void *data, const QOpenGLPixelTransferOptions * const options)
{
    switch (target) {
    case Texture::Target1D:
        Q_UNUSED(layer);
        Q_UNUSED(cubeFace);
        Q_UNUSED(layerCount);
        texFuncs->glTextureSubImage1D(textureId, target, bindingTarget, mipLevel,
                                      0, mipLevelSize( mipLevel, dimensions[0] ),
                                      sourceFormat, sourceType, data, options);
        break;

    case Texture::Target1DArray:
        Q_UNUSED(cubeFace);
        texFuncs->glTextureSubImage2D(textureId, target, bindingTarget, mipLevel,
                                      0, layer,
                                      mipLevelSize(mipLevel, dimensions[0]),
                                      layerCount,
                                      sourceFormat, sourceType, data, options);
        break;

    case Texture::Target2D:
        Q_UNUSED(layer);
        Q_UNUSED(cubeFace);
        Q_UNUSED(layerCount);
        texFuncs->glTextureSubImage2D(textureId, target, bindingTarget, mipLevel,
                                      0, 0,
                                      mipLevelSize(mipLevel, dimensions[0]),
                                      mipLevelSize(mipLevel, dimensions[1]),
                                      sourceFormat, sourceType, data, options);
        break;

    case Texture::Target2DArray:
        Q_UNUSED(cubeFace);
        texFuncs->glTextureSubImage3D(textureId, target, bindingTarget, mipLevel,
                                      0, 0, layer,
                                      mipLevelSize(mipLevel, dimensions[0]),
                                      mipLevelSize(mipLevel, dimensions[1]),
                                      layerCount,
                                      sourceFormat, sourceType, data, options);
        break;

    case Texture::Target3D:
        Q_UNUSED(cubeFace);
        Q_UNUSED(layerCount);
        texFuncs->glTextureSubImage3D(textureId, target, bindingTarget, mipLevel,
                                      0, 0, layer,
                                      mipLevelSize(mipLevel, dimensions[0]),
                                      mipLevelSize(mipLevel, dimensions[1]),
                                      mipLevelSize(mipLevel, dimensions[2]),
                                      sourceFormat, sourceType, data, options);
        break;

    case Texture::TargetCubeMap:
        Q_UNUSED(layer);
        Q_UNUSED(layerCount);
        texFuncs->glTextureSubImage2D(textureId, cubeFace, bindingTarget, mipLevel,
                                      0, 0,
                                      mipLevelSize(mipLevel, dimensions[0]),
                                      mipLevelSize(mipLevel, dimensions[1]),
                                      sourceFormat, sourceType, data, options);
        break;

    case Texture::TargetCubeMapArray: {
        int faceIndex = cubeFace - Texture::CubeMapPositiveX;
        int layerFace = 6 * layer + faceIndex;
        texFuncs->glTextureSubImage3D(textureId, target, bindingTarget, mipLevel,
                                      0, 0, layerFace,
                                      mipLevelSize(mipLevel, dimensions[0]),
                                      mipLevelSize(mipLevel, dimensions[1]),
                                      layerCount,
                                      sourceFormat, sourceType, data, options);
        break;
    }

    case Texture::TargetRectangle:
        Q_UNUSED(mipLevel);
        Q_UNUSED(layer);
        Q_UNUSED(cubeFace);
        Q_UNUSED(layerCount);
        texFuncs->glTextureSubImage2D(textureId, target, bindingTarget, 0,
                                      0, 0,
                                      dimensions[0],
                                      dimensions[1],
                                      sourceFormat, sourceType, data, options);
        break;

    case Texture::Target2DMultisample:
    case Texture::Target2DMultisampleArray:
    case Texture::TargetBuffer:
        // We don't upload pixel data for these targets
        qWarning("Texture::setData(): Texture target does not support pixel data upload");
        break;
    }

    // If requested perform automatic mip map generation
    if (mipLevel == 0 && autoGenerateMipMaps && mipLevels > 1) {
        Q_Q(Texture);
        q->generateMipMaps();
    }
}

void TexturePrivate::setCompressedData(int mipLevel, int layer, int layerCount,
                                              Texture::CubeMapFace cubeFace,
                                              int dataSize, const void *data,
                                              const QOpenGLPixelTransferOptions * const options)
{
    if (!isCompressedFormat(format)) {
        qWarning("Cannot set compressed data for non-compressed format 0x%x", format);
        return;
    }

    const bool needsFullSpec = !isUsingImmutableStorage(); // was allocateStorage() a no-op?

    switch (target) {
    case Texture::Target1D:
        Q_UNUSED(layer);
        Q_UNUSED(cubeFace);
        Q_UNUSED(layerCount);
        if (needsFullSpec) {
            texFuncs->glCompressedTextureImage1D(textureId, target, bindingTarget, mipLevel,
                                                 format,
                                                 mipLevelSize(mipLevel, dimensions[0]),
                                                 0, dataSize, data, options);
        } else {
            texFuncs->glCompressedTextureSubImage1D(textureId, target, bindingTarget, mipLevel,
                                                    0, mipLevelSize( mipLevel, dimensions[0] ),
                                                    format, dataSize, data, options);
        }
        break;

    case Texture::Target1DArray:
        Q_UNUSED(cubeFace);
        if (!needsFullSpec) {
            texFuncs->glCompressedTextureSubImage2D(textureId, target, bindingTarget, mipLevel,
                                                    0, layer,
                                                    mipLevelSize(mipLevel, dimensions[0]),
                                                    layerCount,
                                                    format, dataSize, data, options);
        }
        break;

    case Texture::Target2D:
        Q_UNUSED(layer);
        Q_UNUSED(cubeFace);
        Q_UNUSED(layerCount);
        if (needsFullSpec) {
            texFuncs->glCompressedTextureImage2D(textureId, target, bindingTarget, mipLevel,
                                                 format,
                                                 mipLevelSize(mipLevel, dimensions[0]),
                                                 mipLevelSize(mipLevel, dimensions[1]),
                                                 0, dataSize, data, options);
        } else {
            texFuncs->glCompressedTextureSubImage2D(textureId, target, bindingTarget, mipLevel,
                                                    0, 0,
                                                    mipLevelSize(mipLevel, dimensions[0]),
                                                    mipLevelSize(mipLevel, dimensions[1]),
                                                    format, dataSize, data, options);
        }
        break;

    case Texture::Target2DArray:
        Q_UNUSED(cubeFace);
        if (!needsFullSpec) {
            texFuncs->glCompressedTextureSubImage3D(textureId, target, bindingTarget, mipLevel,
                                                    0, 0, layer,
                                                    mipLevelSize(mipLevel, dimensions[0]),
                                                    mipLevelSize(mipLevel, dimensions[1]),
                                                    layerCount,
                                                    format, dataSize, data, options);
        }
        break;

    case Texture::Target3D:
        Q_UNUSED(cubeFace);
        Q_UNUSED(layerCount);
        if (needsFullSpec) {
            texFuncs->glCompressedTextureImage3D(textureId, target, bindingTarget, mipLevel,
                                                 format,
                                                 mipLevelSize(mipLevel, dimensions[0]),
                                                 mipLevelSize(mipLevel, dimensions[1]),
                                                 mipLevelSize(mipLevel, dimensions[2]),
                                                 0, dataSize, data, options);
        } else {
            texFuncs->glCompressedTextureSubImage3D(textureId, target, bindingTarget, mipLevel,
                                                    0, 0, layer,
                                                    mipLevelSize(mipLevel, dimensions[0]),
                                                    mipLevelSize(mipLevel, dimensions[1]),
                                                    mipLevelSize(mipLevel, dimensions[2]),
                                                    format, dataSize, data, options);
        }
        break;

    case Texture::TargetCubeMap:
        Q_UNUSED(layer);
        Q_UNUSED(layerCount);
        if (needsFullSpec) {
            texFuncs->glCompressedTextureImage2D(textureId, cubeFace, bindingTarget, mipLevel,
                                                 format,
                                                 mipLevelSize(mipLevel, dimensions[0]),
                                                 mipLevelSize(mipLevel, dimensions[1]),
                                                 0, dataSize, data, options);
        } else {
            texFuncs->glCompressedTextureSubImage2D(textureId, cubeFace, bindingTarget, mipLevel,
                                                    0, 0,
                                                    mipLevelSize(mipLevel, dimensions[0]),
                                                    mipLevelSize(mipLevel, dimensions[1]),
                                                    format, dataSize, data, options);
        }
        break;

    case Texture::TargetCubeMapArray: {
        int faceIndex = cubeFace - Texture::CubeMapPositiveX;
        int layerFace = 6 * layer + faceIndex;
        if (!needsFullSpec) {
            texFuncs->glCompressedTextureSubImage3D(textureId, target, bindingTarget, mipLevel,
                                                    0, 0, layerFace,
                                                    mipLevelSize(mipLevel, dimensions[0]),
                                                    mipLevelSize(mipLevel, dimensions[1]),
                                                    layerCount,
                                                    format, dataSize, data, options);
        }
        break;
    }

    case Texture::TargetRectangle:
    case Texture::Target2DMultisample:
    case Texture::Target2DMultisampleArray:
    case Texture::TargetBuffer:
        // We don't upload pixel data for these targets
        qWarning("Texture::setCompressedData(): Texture target does not support pixel data upload");
        break;
    }

    // If requested perform automatic mip map generation
    if (mipLevel == 0 && autoGenerateMipMaps && mipLevels > 1) {
        Q_Q(Texture);
        q->generateMipMaps();
    }
}

void TexturePrivate::setWrapMode(Texture::WrapMode mode)
{
    switch (target) {
    case Texture::Target1D:
    case Texture::Target1DArray:
    case Texture::TargetBuffer:
        wrapModes[0] = mode;
        texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_S, mode);
        break;

    case Texture::Target2D:
    case Texture::Target2DArray:
    case Texture::TargetCubeMap:
    case Texture::TargetCubeMapArray:
    case Texture::Target2DMultisample:
    case Texture::Target2DMultisampleArray:
    case Texture::TargetRectangle:
        wrapModes[0] = wrapModes[1] = mode;
        texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_S, mode);
        texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_T, mode);
        break;

    case Texture::Target3D:
        wrapModes[0] = wrapModes[1] = wrapModes[2] = mode;
        texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_S, mode);
        texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_T, mode);
        texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_R, mode);
        break;
    }
}

void TexturePrivate::setWrapMode(Texture::CoordinateDirection direction, Texture::WrapMode mode)
{
    switch (target) {
    case Texture::Target1D:
    case Texture::Target1DArray:
    case Texture::TargetBuffer:
        switch (direction) {
        case Texture::DirectionS:
            wrapModes[0] = mode;
            texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_S, mode);
            break;

        case Texture::DirectionT:
        case Texture::DirectionR:
            qWarning("Texture::setWrapMode() direction not valid for this texture target");
            break;
        }
        break;

    case Texture::Target2D:
    case Texture::Target2DArray:
    case Texture::TargetCubeMap:
    case Texture::TargetCubeMapArray:
    case Texture::Target2DMultisample:
    case Texture::Target2DMultisampleArray:
    case Texture::TargetRectangle:
        switch (direction) {
        case Texture::DirectionS:
            wrapModes[0] = mode;
            texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_S, mode);
            break;

        case Texture::DirectionT:
            wrapModes[1] = mode;
            texFuncs->glTextureParameteri(textureId, target, bindingTarget, GL_TEXTURE_WRAP_T, mode);
            break;

        case Texture::DirectionR:
            qWarning("Texture::setWrapMode() direction not valid for this texture target");
            break;
        }
        break;

    case Texture::Target3D:
        switch (direction) {
        case Texture::DirectionS:
            wrapModes[0] = mode;
            texFuncs->glTextureParameteri(textureId, target, bindingTarget, direction, mode);
            break;

        case Texture::DirectionT:
            wrapModes[1] = mode;
            texFuncs->glTextureParameteri(textureId, target, bindingTarget, direction, mode);
            break;

        case Texture::DirectionR:
            wrapModes[2] = mode;
            texFuncs->glTextureParameteri(textureId, target, bindingTarget, direction, mode);
            break;
        }
        break;
    }
}

Texture::WrapMode TexturePrivate::wrapMode(Texture::CoordinateDirection direction) const
{
    switch (target) {
    case Texture::Target1D:
    case Texture::Target1DArray:
    case Texture::TargetBuffer:
        switch (direction) {
        case Texture::DirectionS:
            return wrapModes[0];

        case Texture::DirectionT:
        case Texture::DirectionR:
            qWarning("Texture::wrapMode() direction not valid for this texture target");
            return Texture::Repeat;
        }
        break;

    case Texture::Target2D:
    case Texture::Target2DArray:
    case Texture::TargetCubeMap:
    case Texture::TargetCubeMapArray:
    case Texture::Target2DMultisample:
    case Texture::Target2DMultisampleArray:
    case Texture::TargetRectangle:
        switch (direction) {
        case Texture::DirectionS:
            return wrapModes[0];

        case Texture::DirectionT:
            return wrapModes[1];

        case Texture::DirectionR:
            qWarning("Texture::wrapMode() direction not valid for this texture target");
            return Texture::Repeat;
        }
        break;

    case Texture::Target3D:
        switch (direction) {
        case Texture::DirectionS:
            return wrapModes[0];

        case Texture::DirectionT:
            return wrapModes[1];

        case Texture::DirectionR:
            return wrapModes[2];
        }
        break;
    }
    // Should never get here
    Q_ASSERT(false);
    return Texture::Repeat;
}

Texture *TexturePrivate::createTextureView(Texture::Target viewTarget,
                                                         Texture::TextureFormat viewFormat,
                                                         int minimumMipmapLevel, int maximumMipmapLevel,
                                                         int minimumLayer, int maximumLayer) const
{
    // Do sanity checks - see http://www.opengl.org/wiki/GLAPI/glTextureView

    // Check the targets are compatible
    bool viewTargetCompatible = false;
    switch (target) {
    case Texture::Target1D:
    case Texture::Target1DArray:
        viewTargetCompatible = (viewTarget == Texture::Target1D
                             || viewTarget == Texture::Target1DArray);
        break;


    case Texture::Target2D:
    case Texture::Target2DArray:
        viewTargetCompatible = (viewTarget == Texture::Target2D
                             || viewTarget == Texture::Target2DArray);
        break;

    case Texture::Target3D:
        viewTargetCompatible = (viewTarget == Texture::Target3D);
        break;

    case Texture::TargetCubeMap:
    case Texture::TargetCubeMapArray:
        viewTargetCompatible = (viewTarget == Texture::TargetCubeMap
                             || viewTarget == Texture::Target2D
                             || viewTarget == Texture::Target2DArray
                             || viewTarget == Texture::TargetCubeMapArray);
        break;

    case Texture::Target2DMultisample:
    case Texture::Target2DMultisampleArray:
        viewTargetCompatible = (viewTarget == Texture::Target2DMultisample
                             || viewTarget == Texture::Target2DMultisampleArray);
        break;

    case Texture::TargetRectangle:
        viewTargetCompatible = (viewTarget == Texture::TargetRectangle);
        break;

    case Texture::TargetBuffer:
        // Cannot be used with texture views
        break;
    }

    if (!viewTargetCompatible) {
        qWarning("Texture::createTextureView(): Incompatible source and view targets");
        return 0;
    }

    // Check the formats are compatible
    bool viewFormatCompatible = false;
    switch (formatClass) {
    case Texture::NoFormatClass:
        break;

    case Texture::FormatClass_128Bit:
        viewFormatCompatible = (viewFormat == Texture::RGBA32F
                             || viewFormat == Texture::RGBA32U
                             || viewFormat == Texture::RGBA32I);
        break;

    case Texture::FormatClass_96Bit:
        viewFormatCompatible = (viewFormat == Texture::RGB32F
                             || viewFormat == Texture::RGB32U
                             || viewFormat == Texture::RGB32I);
        break;

    case Texture::FormatClass_64Bit:
        viewFormatCompatible = (viewFormat == Texture::RGBA16F
                             || viewFormat == Texture::RG32F
                             || viewFormat == Texture::RGBA16U
                             || viewFormat == Texture::RG32U
                             || viewFormat == Texture::RGBA16I
                             || viewFormat == Texture::RG32I
                             || viewFormat == Texture::RGBA16_UNorm
                             || viewFormat == Texture::RGBA16_SNorm);
        break;

    case Texture::FormatClass_48Bit:
        viewFormatCompatible = (viewFormat == Texture::RGB16_UNorm
                             || viewFormat == Texture::RGB16_SNorm
                             || viewFormat == Texture::RGB16F
                             || viewFormat == Texture::RGB16U
                             || viewFormat == Texture::RGB16I);
        break;

    case Texture::FormatClass_32Bit:
        viewFormatCompatible = (viewFormat == Texture::RG16F
                             || viewFormat == Texture::RG11B10F
                             || viewFormat == Texture::R32F
                             || viewFormat == Texture::RGB10A2
                             || viewFormat == Texture::RGBA8U
                             || viewFormat == Texture::RG16U
                             || viewFormat == Texture::R32U
                             || viewFormat == Texture::RGBA8I
                             || viewFormat == Texture::RG16I
                             || viewFormat == Texture::R32I
                             || viewFormat == Texture::RGBA8_UNorm
                             || viewFormat == Texture::RG16_UNorm
                             || viewFormat == Texture::RGBA8_SNorm
                             || viewFormat == Texture::RG16_SNorm
                             || viewFormat == Texture::SRGB8_Alpha8
                             || viewFormat == Texture::RGB9E5);
        break;

    case Texture::FormatClass_24Bit:
        viewFormatCompatible = (viewFormat == Texture::RGB8_UNorm
                             || viewFormat == Texture::RGB8_SNorm
                             || viewFormat == Texture::SRGB8
                             || viewFormat == Texture::RGB8U
                             || viewFormat == Texture::RGB8I);
        break;

    case Texture::FormatClass_16Bit:
        viewFormatCompatible = (viewFormat == Texture::R16F
                             || viewFormat == Texture::RG8U
                             || viewFormat == Texture::R16U
                             || viewFormat == Texture::RG8I
                             || viewFormat == Texture::R16I
                             || viewFormat == Texture::RG8_UNorm
                             || viewFormat == Texture::R16_UNorm
                             || viewFormat == Texture::RG8_SNorm
                             || viewFormat == Texture::R16_SNorm);
        break;

    case Texture::FormatClass_8Bit:
        viewFormatCompatible = (viewFormat == Texture::R8U
                             || viewFormat == Texture::R8I
                             || viewFormat == Texture::R8_UNorm
                             || viewFormat == Texture::R8_SNorm);
        break;

    case Texture::FormatClass_RGTC1_R:
        viewFormatCompatible = (viewFormat == Texture::R_ATI1N_UNorm
                             || viewFormat == Texture::R_ATI1N_SNorm);
        break;

    case Texture::FormatClass_RGTC2_RG:
        viewFormatCompatible = (viewFormat == Texture::RG_ATI2N_UNorm
                             || viewFormat == Texture::RG_ATI2N_SNorm);
        break;

    case Texture::FormatClass_BPTC_Unorm:
        viewFormatCompatible = (viewFormat == Texture::RGB_BP_UNorm
                             || viewFormat == Texture::SRGB_BP_UNorm);
        break;

    case Texture::FormatClass_BPTC_Float:
        viewFormatCompatible = (viewFormat == Texture::RGB_BP_UNSIGNED_FLOAT
                             || viewFormat == Texture::RGB_BP_SIGNED_FLOAT);
        break;

    case Texture::FormatClass_S3TC_DXT1_RGB:
        viewFormatCompatible = (viewFormat == Texture::RGB_DXT1
                             || viewFormat == Texture::SRGB_DXT1);
        break;

    case Texture::FormatClass_S3TC_DXT1_RGBA:
        viewFormatCompatible = (viewFormat == Texture::RGBA_DXT1
                             || viewFormat == Texture::SRGB_Alpha_DXT1);
        break;

    case Texture::FormatClass_S3TC_DXT3_RGBA:
        viewFormatCompatible = (viewFormat == Texture::RGBA_DXT3
                             || viewFormat == Texture::SRGB_Alpha_DXT3);
        break;

    case Texture::FormatClass_S3TC_DXT5_RGBA:
        viewFormatCompatible = (viewFormat == Texture::RGBA_DXT5
                             || viewFormat == Texture::SRGB_Alpha_DXT5);
        break;

    case Texture::FormatClass_Unique:
        viewFormatCompatible = (viewFormat == format);
        break;
    }

    if (!viewFormatCompatible) {
        qWarning("Texture::createTextureView(): Incompatible source and view formats");
        return 0;
    }


    // Create a view
    Texture *view = new Texture(viewTarget);
    view->setFormat(viewFormat);
    view->create();
    view->d_ptr->textureView = true;
    texFuncs->glTextureView(view->textureId(), viewTarget, textureId, viewFormat,
                            minimumMipmapLevel, maximumMipmapLevel - minimumMipmapLevel + 1,
                            minimumLayer, maximumLayer - minimumLayer + 1);
    return view;
}


/*!
    \class Texture
    \inmodule QtGui
    \since 5.2
    \wrapper
    \brief The Texture class encapsulates an OpenGL texture object.

    Texture makes it easy to work with OpenGL textures and the myriad features
    and targets that they offer depending upon the capabilities of your OpenGL implementation.

    The typical usage pattern for Texture is
    \list
        \li Instantiate the object specifying the texture target type
        \li Set properties that affect the storage requirements e.g. storage format, dimensions
        \li Allocate the server-side storage
        \li Optionally upload pixel data
        \li Optionally set any additional properties e.g. filtering and border options
        \li Render with texture or render to texture
    \endlist

    In the common case of simply using a QImage as the source of texture pixel data
    most of the above steps are performed automatically.

    \code
    // Prepare texture
    Texture *texture = new Texture(QImage(fileName).mirrored());
    texture->setMinificationFilter(Texture::LinearMipMapLinear);
    texture->setMagnificationFilter(Texture::Linear);
    ...
    // Render with texture
    texture->bind();
    glDrawArrays(...);
    \endcode

    Note that the QImage is mirrored vertically to account for the fact that
    OpenGL and QImage use opposite directions for the y axis. Another option
    would be to transform your texture coordinates.
*/

/*!
    \enum Texture::Filter
    This enum defines the filtering parameters for a Texture object.
    \value Nearest Equivalent to GL_NEAREST
    \value Linear Equivalent to GL_LINEAR
    \value NearestMipMapNearest Equivalent to GL_NEAREST_MIPMAP_NEAREST
    \value NearestMipMapLinear Equivalent to GL_NEAREST_MIPMAP_LINEAR
    \value LinearMipMapNearest Equivalent to GL_LINEAR_MIPMAP_NEAREST
    \value LinearMipMapLinear Equivalent to GL_LINEAR_MIPMAP_LINEAR
*/

/*!
    \enum Texture::Target
    This enum defines the texture target of a Texture object.

    \value Target1D A 1-dimensional texture.
           Equivalent to GL_TEXTURE_1D.
    \value Target1DArray An array of 1-dimensional textures.
           Equivalent to GL_TEXTURE_1D_ARRAY
    \value Target2D A 2-dimensional texture.
           Equivalent to GL_TEXTURE_2D
    \value Target2DArray An array of 1-dimensional textures.
           Equivalent to GL_TEXTURE_2D_ARRAY
    \value Target3D A 3-dimensional texture.
           Equivalent to GL_TEXTURE_3D
    \value TargetCubeMap A cubemap texture.
           Equivalent to GL_TEXTURE_CUBE_MAP
    \value TargetCubeMapArray An array of cubemap textures.
           Equivalent to GL_TEXTURE_CUBE_MAP_ARRAY
    \value Target2DMultisample A 2-dimensional texture with multisample support.
           Equivalent to GL_TEXTURE_2D_MULTISAMPLE
    \value Target2DMultisampleArray An array of 2-dimensional textures with multisample support.
           Equivalent to GL_TEXTURE_2D_MULTISAMPLE_ARRAY
    \value TargetRectangle A rectangular 2-dimensional texture.
           Equivalent to GL_TEXTURE_RECTANGLE
    \value TargetBuffer A texture with data from an OpenGL buffer object.
           Equivalent to GL_TEXTURE_BUFFER
*/

/*!
    \enum Texture::BindingTarget
    This enum defines the possible binding targets of texture units.

    \value BindingTarget1D Equivalent to GL_TEXTURE_BINDING_1D
    \value BindingTarget1DArray Equivalent to GL_TEXTURE_BINDING_1D_ARRAY
    \value BindingTarget2D Equivalent to GL_TEXTURE_BINDING_2D
    \value BindingTarget2DArray Equivalent to GL_TEXTURE_BINDING_2D_ARRAY
    \value BindingTarget3D Equivalent to GL_TEXTURE_BINDING_3D
    \value BindingTargetCubeMap Equivalent to GL_TEXTURE_BINDING_CUBE_MAP
    \value BindingTargetCubeMapArray Equivalent to GL_TEXTURE_BINDING_CUBE_MAP_ARRAY
    \value BindingTarget2DMultisample Equivalent to GL_TEXTURE_BINDING_2D_MULTISAMPLE
    \value BindingTarget2DMultisampleArray Equivalent to GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY
    \value BindingTargetRectangle Equivalent to GL_TEXTURE_BINDING_RECTANGLE
    \value BindingTargetBuffer Equivalent to GL_TEXTURE_BINDING_BUFFER
*/

/*!
    \enum Texture::MipMapGeneration
    This enum defines the options to control mipmap generation.

    \value GenerateMipMaps Mipmaps should be generated
    \value DontGenerateMipMaps Mipmaps should not be generated
*/

/*!
    \enum Texture::TextureUnitReset
    This enum defines options ot control texture unit activation.

    \value ResetTextureUnit The previous active texture unit will be reset
    \value DontResetTextureUnit The previous active texture unit will not be rest
*/

/*!
    \enum Texture::TextureFormat
    This enum defines the possible texture formats. Depending upon your OpenGL
    implementation only a subset of these may be supported.

    \value NoFormat Equivalent to GL_NONE

    \value R8_UNorm Equivalent to GL_R8
    \value RG8_UNorm Equivalent to GL_RG8
    \value RGB8_UNorm Equivalent to GL_RGB8
    \value RGBA8_UNorm Equivalent to GL_RGBA8

    \value R16_UNorm Equivalent to GL_R16
    \value RG16_UNorm Equivalent to GL_RG16
    \value RGB16_UNorm Equivalent to GL_RGB16
    \value RGBA16_UNorm Equivalent to GL_RGBA16

    \value R8_SNorm Equivalent to GL_R8_SNORM
    \value RG8_SNorm Equivalent to GL_RG8_SNORM
    \value RGB8_SNorm Equivalent to GL_RGB8_SNORM
    \value RGBA8_SNorm Equivalent to GL_RGBA8_SNORM

    \value R16_SNorm Equivalent to GL_R16_SNORM
    \value RG16_SNorm Equivalent to GL_RG16_SNORM
    \value RGB16_SNorm Equivalent to GL_RGB16_SNORM
    \value RGBA16_SNorm Equivalent to GL_RGBA16_SNORM

    \value R8U Equivalent to GL_R8UI
    \value RG8U Equivalent to GL_RG8UI
    \value RGB8U Equivalent to GL_RGB8UI
    \value RGBA8U Equivalent to GL_RGBA8UI

    \value R16U Equivalent to GL_R16UI
    \value RG16U Equivalent to GL_RG16UI
    \value RGB16U Equivalent to GL_RGB16UI
    \value RGBA16U Equivalent to GL_RGBA16UI

    \value R32U Equivalent to GL_R32UI
    \value RG32U Equivalent to GL_RG32UI
    \value RGB32U Equivalent to GL_RGB32UI
    \value RGBA32U Equivalent to GL_RGBA32UI

    \value R8I Equivalent to GL_R8I
    \value RG8I Equivalent to GL_RG8I
    \value RGB8I Equivalent to GL_RGB8I
    \value RGBA8I Equivalent to GL_RGBA8I

    \value R16I Equivalent to GL_R16I
    \value RG16I Equivalent to GL_RG16I
    \value RGB16I Equivalent to GL_RGB16I
    \value RGBA16I Equivalent to GL_RGBA16I

    \value R32I Equivalent to GL_R32I
    \value RG32I Equivalent to GL_RG32I
    \value RGB32I Equivalent to GL_RGB32I
    \value RGBA32I Equivalent to GL_RGBA32I

    \value R16F Equivalent to GL_R16F
    \value RG16F Equivalent to GL_RG16F
    \value RGB16F Equivalent to GL_RGB16F
    \value RGBA16F Equivalent to GL_RGBA16F

    \value R32F Equivalent to GL_R32F
    \value RG32F Equivalent to GL_RG32F
    \value RGB32F Equivalent to GL_RGB32F
    \value RGBA32F Equivalent to GL_RGBA32F

    \value RGB9E5 Equivalent to GL_RGB9_E5
    \value RG11B10F Equivalent to GL_R11F_G11F_B10F
    \value RG3B2 Equivalent to GL_R3_G3_B2
    \value R5G6B5 Equivalent to GL_RGB565
    \value RGB5A1 Equivalent to GL_RGB5_A1
    \value RGBA4 Equivalent to GL_RGBA4
    \value RGB10A2 Equivalent to GL_RGB10_A2UI

    \value D16 Equivalent to GL_DEPTH_COMPONENT16
    \value D24 Equivalent to GL_DEPTH_COMPONENT24
    \value D24S8 Equivalent to GL_DEPTH24_STENCIL8
    \value D32 Equivalent to GL_DEPTH_COMPONENT32
    \value D32F Equivalent to GL_DEPTH_COMPONENT32F
    \value D32FS8X24 Equivalent to GL_DEPTH32F_STENCIL8
    \value S8 Equivalent to GL_STENCIL_INDEX8. Introduced in Qt 5.4

    \value RGB_DXT1 Equivalent to GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    \value RGBA_DXT1 Equivalent to GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
    \value RGBA_DXT3 Equivalent to GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
    \value RGBA_DXT5 Equivalent to GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
    \value R_ATI1N_UNorm Equivalent to GL_COMPRESSED_RED_RGTC1
    \value R_ATI1N_SNorm Equivalent to GL_COMPRESSED_SIGNED_RED_RGTC1
    \value RG_ATI2N_UNorm Equivalent to GL_COMPRESSED_RG_RGTC2
    \value RG_ATI2N_SNorm Equivalent to GL_COMPRESSED_SIGNED_RG_RGTC2
    \value RGB_BP_UNSIGNED_FLOAT Equivalent to GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB
    \value RGB_BP_SIGNED_FLOAT Equivalent to GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB
    \value RGB_BP_UNorm Equivalent to GL_COMPRESSED_RGBA_BPTC_UNORM_ARB
    \value R11_EAC_UNorm Equivalent to GL_COMPRESSED_R11_EAC
    \value R11_EAC_SNorm Equivalent to GL_COMPRESSED_SIGNED_R11_EAC
    \value RG11_EAC_UNorm Equivalent to GL_COMPRESSED_RG11_EAC
    \value RG11_EAC_SNorm Equivalent to GL_COMPRESSED_SIGNED_RG11_EAC
    \value RGB8_ETC2 Equivalent to GL_COMPRESSED_RGB8_ETC2
    \value SRGB8_ETC2 Equivalent to GL_COMPRESSED_SRGB8_ETC2
    \value RGB8_PunchThrough_Alpha1_ETC2 Equivalent to GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
    \value SRGB8_PunchThrough_Alpha1_ETC2 Equivalent to GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2
    \value RGBA8_ETC2_EAC Equivalent to GL_COMPRESSED_RGBA8_ETC2_EAC
    \value SRGB8_Alpha8_ETC2_EAC Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
    \value RGB8_ETC1 Equivalent to GL_ETC1_RGB8_OES
    \value RGBA_ASTC_4x4 Equivalent to GL_COMPRESSED_RGBA_ASTC_4x4_KHR
    \value RGBA_ASTC_5x4 Equivalent to GL_COMPRESSED_RGBA_ASTC_5x4_KHR
    \value RGBA_ASTC_5x5 Equivalent to GL_COMPRESSED_RGBA_ASTC_5x5_KHR
    \value RGBA_ASTC_6x5 Equivalent to GL_COMPRESSED_RGBA_ASTC_6x5_KHR
    \value RGBA_ASTC_6x6 Equivalent to GL_COMPRESSED_RGBA_ASTC_6x6_KHR
    \value RGBA_ASTC_8x5 Equivalent to GL_COMPRESSED_RGBA_ASTC_8x5_KHR
    \value RGBA_ASTC_8x6 Equivalent to GL_COMPRESSED_RGBA_ASTC_8x6_KHR
    \value RGBA_ASTC_8x8 Equivalent to GL_COMPRESSED_RGBA_ASTC_8x8_KHR
    \value RGBA_ASTC_10x5 Equivalent to GL_COMPRESSED_RGBA_ASTC_10x5_KHR
    \value RGBA_ASTC_10x6 Equivalent to GL_COMPRESSED_RGBA_ASTC_10x6_KHR
    \value RGBA_ASTC_10x8 Equivalent to GL_COMPRESSED_RGBA_ASTC_10x8_KHR
    \value RGBA_ASTC_10x10 Equivalent to GL_COMPRESSED_RGBA_ASTC_10x10_KHR
    \value RGBA_ASTC_12x10 Equivalent to GL_COMPRESSED_RGBA_ASTC_12x10_KHR
    \value RGBA_ASTC_12x12 Equivalent to GL_COMPRESSED_RGBA_ASTC_12x12_KHR
    \value SRGB8_Alpha8_ASTC_4x4 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR
    \value SRGB8_Alpha8_ASTC_5x4 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR
    \value SRGB8_Alpha8_ASTC_5x5 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR
    \value SRGB8_Alpha8_ASTC_6x5 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR
    \value SRGB8_Alpha8_ASTC_6x6 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR
    \value SRGB8_Alpha8_ASTC_8x5 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR
    \value SRGB8_Alpha8_ASTC_8x6 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR
    \value SRGB8_Alpha8_ASTC_8x8 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR
    \value SRGB8_Alpha8_ASTC_10x5 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR
    \value SRGB8_Alpha8_ASTC_10x6 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR
    \value SRGB8_Alpha8_ASTC_10x8 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR
    \value SRGB8_Alpha8_ASTC_10x10 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR
    \value SRGB8_Alpha8_ASTC_12x10 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR
    \value SRGB8_Alpha8_ASTC_12x12 Equivalent to GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR

    \value SRGB8 Equivalent to GL_SRGB8
    \value SRGB8_Alpha8 Equivalent to GL_SRGB8_ALPHA8
    \value SRGB_DXT1 Equivalent to GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
    \value SRGB_Alpha_DXT1 Equivalent to GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
    \value SRGB_Alpha_DXT3 Equivalent to GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
    \value SRGB_Alpha_DXT5 Equivalent to GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
    \value SRGB_BP_UNorm Equivalent to GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB

    \value DepthFormat Equivalent to GL_DEPTH_COMPONENT (only OpenGL ES 3 or ES 2 with OES_depth_texture)
    \value AlphaFormat Equivalent to GL_ALPHA (OpenGL ES 2 only)
    \value RGBFormat Equivalent to GL_RGB (OpenGL ES 2 only)
    \value RGBAFormat Equivalent to GL_RGBA (OpenGL ES 2 only)
    \value LuminanceFormat Equivalent to GL_LUMINANCE (OpenGL ES 2 only)
    \value LuminanceAlphaFormat Equivalent to GL_LUMINANCE_ALPHA (OpenGL ES 2 only)
*/

/*!
    \enum Texture::CubeMapFace
    This enum defines the possible CubeMap faces.

    \value CubeMapPositiveX Equivalent to GL_TEXTURE_CUBE_MAP_POSITIVE_X
    \value CubeMapNegativeX Equivalent to GL_TEXTURE_CUBE_MAP_NEGATIVE_X
    \value CubeMapPositiveY Equivalent to GL_TEXTURE_CUBE_MAP_POSITIVE_Y
    \value CubeMapNegativeY Equivalent to GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    \value CubeMapPositiveZ Equivalent to GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    \value CubeMapNegativeZ Equivalent to GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
*/

/*!
    \enum Texture::PixelFormat
    This enum defines the possible client-side pixel formats for a pixel
    transfer operation.

    \value NoSourceFormat Equivalent to GL_NONE
    \value Red Equivalent to GL_RED
    \value RG Equivalent to GL_RG
    \value RGB Equivalent to GL_RGB
    \value BGR Equivalent to GL_BGR
    \value RGBA Equivalent to GL_RGBA
    \value BGRA Equivalent to GL_BGRA
    \value Red_Integer Equivalent to GL_RED_INTEGER
    \value RG_Integer Equivalent to GL_RG_INTEGER
    \value RGB_Integer Equivalent to GL_RGB_INTEGER
    \value BGR_Integer Equivalent to GL_BGR_INTEGER
    \value RGBA_Integer Equivalent to GL_RGBA_INTEGER
    \value BGRA_Integer Equivalent to GL_BGRA_INTEGER
    \value Stencil Equivalent to GL_STENCIL_INDEX. Introduced in Qt 5.4
    \value Depth Equivalent to GL_DEPTH_COMPONENT
    \value DepthStencil Equivalent to GL_DEPTH_STENCIL
    \value Alpha Equivalent to GL_ALPHA (OpenGL ES 2 only)
    \value Luminance Equivalent to GL_LUMINANCE (OpenGL ES 2 only)
    \value LuminanceAlpha Equivalent to GL_LUMINANCE_ALPHA (OpenGL ES 2 only)

*/

/*!
    \enum Texture::PixelType
    This enum defines the possible pixel data types for a pixel transfer operation

    \value NoPixelType Equivalent to GL_NONE
    \value Int8 Equivalent to GL_BYTE
    \value UInt8 Equivalent to GL_UNSIGNED_BYTE
    \value Int16 Equivalent to GL_SHORT
    \value UInt16 Equivalent to GL_UNSIGNED_SHORT
    \value Int32 Equivalent to GL_INT
    \value UInt32 Equivalent to GL_UNSIGNED_INT
    \value Float16 Equivalent to GL_HALF_FLOAT
    \value Float16OES Equivalent to GL_HALF_FLOAT_OES
    \value Float32 Equivalent to GL_FLOAT
    \value UInt32_RGB9_E5 Equivalent to GL_UNSIGNED_INT_5_9_9_9_REV
    \value UInt32_RG11B10F Equivalent to GL_UNSIGNED_INT_10F_11F_11F_REV
    \value UInt8_RG3B2 Equivalent to GL_UNSIGNED_BYTE_3_3_2
    \value UInt8_RG3B2_Rev Equivalent to GL_UNSIGNED_BYTE_2_3_3_REV
    \value UInt16_RGB5A1 Equivalent to GL_UNSIGNED_SHORT_5_5_5_1
    \value UInt16_RGB5A1_Rev Equivalent to GL_UNSIGNED_SHORT_1_5_5_5_REV
    \value UInt16_R5G6B5 Equivalent to GL_UNSIGNED_SHORT_5_6_5
    \value UInt16_R5G6B5_Rev Equivalent to GL_UNSIGNED_SHORT_5_6_5_REV
    \value UInt16_RGBA4 Equivalent to GL_UNSIGNED_SHORT_4_4_4_4
    \value UInt16_RGBA4_Rev Equivalent to GL_UNSIGNED_SHORT_4_4_4_4_REV
    \value UInt32_RGBA8 Equivalent to GL_UNSIGNED_INT_8_8_8_8
    \value UInt32_RGBA8_Rev Equivalent to GL_UNSIGNED_INT_8_8_8_8_REV
    \value UInt32_RGB10A2 Equivalent to GL_UNSIGNED_INT_10_10_10_2
    \value UInt32_RGB10A2_Rev Equivalent to GL_UNSIGNED_INT_2_10_10_10_REV
    \value UInt32_D24S8 Equivalent to GL_UNSIGNED_INT_24_8. Introduced in Qt 5.4
    \value Float32_D32_UInt32_S8_X24 Equivalent to GL_FLOAT_32_UNSIGNED_INT_24_8_REV. Introduced in Qt 5.4
*/

/*!
    \enum Texture::Feature
    This enum defines the OpenGL texture-related features that can be tested for.

    \value ImmutableStorage Support for immutable texture storage
    \value ImmutableMultisampleStorage Support for immutable texture storage with
           multisample targets
    \value TextureRectangle Support for the GL_TEXTURE_RECTANGLE target
    \value TextureArrays Support for texture targets with array layers
    \value Texture3D Support for the 3 dimensional texture target
    \value TextureMultisample Support for texture targets that have multisample capabilities
    \value TextureBuffer Support for textures that use OpenGL buffer objects
           as their data source
    \value TextureCubeMapArrays Support for cubemap array texture target
    \value Swizzle Support for texture component swizzle masks
    \value StencilTexturing Support for stencil texturing (i.e. looking up depth or stencil
           components of a combined depth/stencil format texture in GLSL shaders).
    \value AnisotropicFiltering Support for anisotropic texture filtering
    \value NPOTTextures Basic support for non-power-of-two textures
    \value NPOTTextureRepeat Full support for non-power-of-two textures including texture
           repeat modes
    \value Texture1D Support for the 1 dimensional texture target
    \value TextureComparisonOperators Support for texture comparison operators
    \value TextureMipMapLevel Support for setting the base and maximum mipmap levels
*/

/*!
    \enum Texture::SwizzleComponent
    This enum defines the texture color components that can be assigned a swizzle mask.

    \value SwizzleRed The red component. Equivalent to GL_TEXTURE_SWIZZLE_R
    \value SwizzleGreen The green component. Equivalent to GL_TEXTURE_SWIZZLE_G
    \value SwizzleBlue The blue component. Equivalent to GL_TEXTURE_SWIZZLE_B
    \value SwizzleAlpha The alpha component. Equivalent to GL_TEXTURE_SWIZZLE_A
*/

/*!
    \enum Texture::SwizzleValue
    This enum defines the possible mask values for texture swizzling.

    \value RedValue Maps the component to the red channel. Equivalent to GL_RED
    \value GreenValue Maps the component to the green channel. Equivalent to GL_GREEN
    \value BlueValue Maps the component to the blue channel. Equivalent to GL_BLUE
    \value AlphaValue Maps the component to the alpha channel. Equivalent to GL_ALPHA
    \value ZeroValue Maps the component to a fixed value of 0. Equivalent to GL_ZERO
    \value OneValue Maps the component to a fixed value of 1. Equivalent to GL_ONE
*/

/*!
    \enum Texture::WrapMode
    This enum defines the possible texture coordinate wrapping modes.

    \value Repeat Texture coordinate is repeated. Equivalent to GL_REPEAT
    \value MirroredRepeat Texture coordinate is reflected about 0 and 1. Equivalent to GL_MIRRORED_REPEAT
    \value ClampToEdge Clamps the texture coordinates to [0,1]. Equivalent to GL_CLAMP_TO_EDGE
    \value ClampToBorder As for ClampToEdge but also blends samples at 0 and 1 with a
           fixed border color. Equivalent to GL_CLAMP_TO_BORDER
*/

/*!
    \enum Texture::CoordinateDirection
    This enum defines the possible texture coordinate directions

    \value DirectionS The horizontal direction. Equivalent to GL_TEXTURE_WRAP_S
    \value DirectionT The vertical direction. Equivalent to GL_TEXTURE_WRAP_T
    \value DirectionR The depth direction. Equivalent to GL_TEXTURE_WRAP_R
*/

/*!
    Creates a Texture object that can later be bound to \a target.

    This does not create the underlying OpenGL texture object. Therefore,
    construction using this constructor does not require a valid current
    OpenGL context.
*/
Texture::Texture(Target target)
    : d_ptr(new TexturePrivate(target, this))
{
}

/*!
    Creates a Texture object that can later be bound to the 2D texture
    target and contains the pixel data contained in \a image. If you wish
    to have a chain of mipmaps generated then set \a genMipMaps to \c true (this
    is the default).

    This does create the underlying OpenGL texture object. Therefore,
    construction using this constructor does require a valid current
    OpenGL context.
*/
Texture::Texture(const QImage& image, MipMapGeneration genMipMaps)
    : Texture(Texture::Target2D)
{
    setData(image, genMipMaps);
}

Texture::~Texture()
{
}

/*!
    Returns the binding target of this texture.

    \since 5.4
*/
Texture::Target Texture::target() const
{
    Q_D(const Texture);
    return d->target;
}

/*!
    Creates the underlying OpenGL texture object. This requires a current valid
    OpenGL context. If the texture object already exists, this function does
    nothing.

    Once the texture object is created you can obtain the object
    name from the textureId() function. This may be useful if you wish to make
    some raw OpenGL calls related to this texture.

    Normally it should not be necessary to call this function directly as all
    functions that set properties of the texture object implicitly call create()
    on your behalf.

    Returns \c true if the creation succeeded, otherwise returns \c false.

    \sa destroy(), isCreated(), textureId()
*/
bool Texture::create()
{
    Q_D(Texture);
    return d->create();
}

/*!
    Destroys the underlying OpenGL texture object. This requires a current valid
    OpenGL context.

    \sa create(), isCreated(), textureId()
*/
void Texture::destroy()
{
    Q_D(Texture);
    return d->destroy();
}

/*!
    Returns \c true if the underlying OpenGL texture object has been created.

    \sa create(), destroy(), textureId()
*/
bool Texture::isCreated() const
{
    Q_D(const Texture);
    return d->textureId != 0;
}

/*!
    Returns the name of the underlying OpenGL texture object or 0 if it has
    not yet been created.

    \sa create(), destroy(), isCreated()
*/
unsigned int Texture::textureId() const
{
    Q_D(const Texture);
    return d->textureId;
}

/*!
    Binds this texture to the currently active texture unit ready for
    rendering. Note that you do not need to bind Texture objects
    in order to modify them as the implementation makes use of the
    EXT_direct_state_access extension where available and simulates it
    where it is not.

    \sa release()
*/
void Texture::bind()
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->bind();
}

/*!
    Binds this texture to texture unit \a unit ready for
    rendering. Note that you do not need to bind Texture objects
    in order to modify them as the implementation makes use of the
    EXT_direct_state_access extension where available and simulates it
    where it is not.

    If parameter \a reset is \c true then this function will restore
    the active unit to the texture unit that was active upon entry.

    \sa release()
*/
void Texture::bind(uint unit, TextureUnitReset reset)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->bind(unit, reset);
}

/*!
    Unbinds this texture from the currently active texture unit.

    \sa bind()
*/
void Texture::release()
{
    Q_D(Texture);
    d->release();
}

/*!
    Unbinds this texture from texture unit \a unit.

    If parameter \a reset is \c true then this function
    will restore the active unit to the texture unit that was active
    upon entry.
*/
void Texture::release(uint unit, TextureUnitReset reset)
{
    Q_D(Texture);
    d->release(unit, reset);
}

/*!
    Returns \c true if this texture is bound to the corresponding target
    of the currently active texture unit.

    \sa bind(), release()
*/
bool Texture::isBound() const
{
    Q_D(const Texture);
    Q_ASSERT(d->textureId);
    return d->isBound();
}

/*!
    Returns \c true if this texture is bound to the corresponding target
    of texture unit \a unit.

    \sa bind(), release()
*/
bool Texture::isBound(uint unit)
{
    Q_D(const Texture);
    Q_ASSERT(d->textureId);
    return d->isBound(unit);
}

/*!
    Returns the textureId of the texture that is bound to the \a target
    of the currently active texture unit.
*/
unsigned int Texture::boundTextureId(BindingTarget target)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        qWarning("Texture::boundTextureId() requires a valid current context");
        return 0;
    }

    GLint textureId = 0;
    ctx->functions()->glGetIntegerv(target, &textureId);
    return static_cast<unsigned int>(textureId);
}

/*!
    Returns the textureId of the texture that is bound to the \a target
    of the texture unit \a unit.
*/
unsigned int Texture::boundTextureId(uint unit, BindingTarget target)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        qWarning("Texture::boundTextureId() requires a valid current context");
        return 0;
    }

    QOpenGLFunctions *funcs = ctx->functions();
    funcs->initializeOpenGLFunctions();

    GLint oldTextureUnit = 0;
    funcs->glGetIntegerv(GL_ACTIVE_TEXTURE, &oldTextureUnit);

    funcs->glActiveTexture(unit);
    GLint textureId = 0;
    funcs->glGetIntegerv(target, &textureId);
    funcs->glActiveTexture(oldTextureUnit);

    return static_cast<unsigned int>(textureId);
}

/*!
    Sets the format of this texture object to \a format. This function
    must be called before texture storage is allocated.

    Note that all formats may not be supported. The exact set of supported
    formats is dependent upon your OpenGL implementation and version.

    \sa format(), allocateStorage()
*/
void Texture::setFormat(TextureFormat format)
{
    Q_D(Texture);
    d->create();
    if (isStorageAllocated()) {
        qWarning("Texture::setFormat(): Cannot change format once storage has been allocated");
        return;
    }

    d->format = format;

    switch (format) {
    case NoFormat:
        d->formatClass = NoFormatClass;
        break;

    case RGBA32F:
    case RGBA32U:
    case RGBA32I:
        d->formatClass = FormatClass_128Bit;
        break;

    case RGB32F:
    case RGB32U:
    case RGB32I:
        d->formatClass = FormatClass_96Bit;
        break;

    case RGBA16F:
    case RG32F:
    case RGBA16U:
    case RG32U:
    case RGBA16I:
    case RG32I:
    case RGBA16_UNorm:
    case RGBA16_SNorm:
        d->formatClass = FormatClass_64Bit;
        break;

    case RGB16_UNorm:
    case RGB16_SNorm:
    case RGB16F:
    case RGB16U:
    case RGB16I:
        d->formatClass = FormatClass_48Bit;
        break;

    case RG16F:
    case RG11B10F:
    case R32F:
    case RGB10A2:
    case RGBA8U:
    case RG16U:
    case R32U:
    case RGBA8I:
    case RG16I:
    case R32I:
    case RGBA8_UNorm:
    case RG16_UNorm:
    case RGBA8_SNorm:
    case RG16_SNorm:
    case SRGB8_Alpha8:
    case RGB9E5:
        d->formatClass = FormatClass_32Bit;
        break;

    case RGB8_UNorm:
    case RGB8_SNorm:
    case SRGB8:
    case RGB8U:
    case RGB8I:
        d->formatClass = FormatClass_24Bit;
        break;

    case R16F:
    case RG8U:
    case R16U:
    case RG8I:
    case R16I:
    case RG8_UNorm:
    case R16_UNorm:
    case RG8_SNorm:
    case R16_SNorm:
        d->formatClass = FormatClass_16Bit;
        break;

    case R8U:
    case R8I:
    case R8_UNorm:
    case R8_SNorm:
        d->formatClass = FormatClass_8Bit;
        break;

    case R_ATI1N_UNorm:
    case R_ATI1N_SNorm:
        d->formatClass = FormatClass_RGTC1_R;
        break;

    case RG_ATI2N_UNorm:
    case RG_ATI2N_SNorm:
        d->formatClass = FormatClass_RGTC2_RG;
        break;

    case RGB_BP_UNorm:
    case SRGB_BP_UNorm:
        d->formatClass = FormatClass_BPTC_Unorm;
        break;

    case RGB_BP_UNSIGNED_FLOAT:
    case RGB_BP_SIGNED_FLOAT:
        d->formatClass = FormatClass_BPTC_Float;
        break;

    case RGB_DXT1:
    case SRGB_DXT1:
        d->formatClass = FormatClass_S3TC_DXT1_RGB;
        break;

    case RGBA_DXT1:
    case SRGB_Alpha_DXT1:
        d->formatClass = FormatClass_S3TC_DXT1_RGBA;
        break;

    case RGBA_DXT3:
    case SRGB_Alpha_DXT3:
        d->formatClass = FormatClass_S3TC_DXT3_RGBA;
        break;

    case RGBA_DXT5:
    case SRGB_Alpha_DXT5:
        d->formatClass = FormatClass_S3TC_DXT5_RGBA;
        break;

    case Texture::R11_EAC_UNorm:
    case Texture::R11_EAC_SNorm:
    case Texture::RG11_EAC_UNorm:
    case Texture::RG11_EAC_SNorm:
    case Texture::RGB8_ETC2:
    case Texture::SRGB8_ETC2:
    case Texture::RGB8_PunchThrough_Alpha1_ETC2:
    case Texture::SRGB8_PunchThrough_Alpha1_ETC2:
    case Texture::RGBA8_ETC2_EAC:
    case Texture::SRGB8_Alpha8_ETC2_EAC:
    case Texture::RGB8_ETC1:
    case RG3B2:
    case R5G6B5:
    case RGB5A1:
    case RGBA4:
    case D16:
    case D24:
    case D24S8:
    case D32:
    case D32F:
    case D32FS8X24:
    case S8:
    case DepthFormat:
    case AlphaFormat:
    case RGBFormat:
    case RGBAFormat:
    case LuminanceFormat:
    case LuminanceAlphaFormat:
    case Texture::RGBA_ASTC_4x4:
    case Texture::RGBA_ASTC_5x4:
    case Texture::RGBA_ASTC_5x5:
    case Texture::RGBA_ASTC_6x5:
    case Texture::RGBA_ASTC_6x6:
    case Texture::RGBA_ASTC_8x5:
    case Texture::RGBA_ASTC_8x6:
    case Texture::RGBA_ASTC_8x8:
    case Texture::RGBA_ASTC_10x5:
    case Texture::RGBA_ASTC_10x6:
    case Texture::RGBA_ASTC_10x8:
    case Texture::RGBA_ASTC_10x10:
    case Texture::RGBA_ASTC_12x10:
    case Texture::RGBA_ASTC_12x12:
    case Texture::SRGB8_Alpha8_ASTC_4x4:
    case Texture::SRGB8_Alpha8_ASTC_5x4:
    case Texture::SRGB8_Alpha8_ASTC_5x5:
    case Texture::SRGB8_Alpha8_ASTC_6x5:
    case Texture::SRGB8_Alpha8_ASTC_6x6:
    case Texture::SRGB8_Alpha8_ASTC_8x5:
    case Texture::SRGB8_Alpha8_ASTC_8x6:
    case Texture::SRGB8_Alpha8_ASTC_8x8:
    case Texture::SRGB8_Alpha8_ASTC_10x5:
    case Texture::SRGB8_Alpha8_ASTC_10x6:
    case Texture::SRGB8_Alpha8_ASTC_10x8:
    case Texture::SRGB8_Alpha8_ASTC_10x10:
    case Texture::SRGB8_Alpha8_ASTC_12x10:
    case Texture::SRGB8_Alpha8_ASTC_12x12:
        d->formatClass = FormatClass_Unique;
        break;
    }
}

/*!
    Returns the format of this texture object.

    \sa setFormat()
*/
Texture::TextureFormat Texture::format() const
{
    Q_D(const Texture);
    return d->format;
}

static bool isNpot(int width, int height = 1, int depth = 1)
{
    return width & (width-1) || height & (height-1) || depth & (depth-1);
}

/*!
    Sets the dimensions of this texture object to \a width,
    \a height, and \a depth. The default for each dimension is 1.
    The maximum allowable texture size is dependent upon your OpenGL
    implementation. Allocating storage for a texture less than the
    maximum size can still fail if your system is low on resources.

    If a non-power-of-two \a width, \a height or \a depth is provided and your
    OpenGL implementation doesn't have support for repeating non-power-of-two
    textures, then the wrap mode is automatically set to ClampToEdge.

    \sa width(), height(), depth()
*/
void Texture::setSize(int width, int height, int depth)
{
    Q_D(Texture);
    d->create();
    if (isStorageAllocated()) {
        qWarning("Cannot resize a texture that already has storage allocated.\n"
                 "To do so, destroy() the texture and then create() and setSize()");
        return;
    }

    if (isNpot(width, height, depth) && !hasFeature(Feature::NPOTTextureRepeat) && d->target != Target::TargetRectangle)
        d->setWrapMode(WrapMode::ClampToEdge);

    switch (d->target) {
    case Texture::Target1D:
    case Texture::Target1DArray:
    case Texture::TargetBuffer:
        d->dimensions[0] = width;
        Q_UNUSED(height);
        Q_UNUSED(depth);
        break;

    case Texture::Target2D:
    case Texture::Target2DArray:
    case Texture::TargetRectangle:
    case Texture::Target2DMultisample:
    case Texture::Target2DMultisampleArray:
        d->dimensions[0] = width;
        d->dimensions[1] = height;
        Q_UNUSED(depth);
        break;

    case Texture::TargetCubeMap:
    case Texture::TargetCubeMapArray:
        if (width != height)
            qWarning("QAbstractOpenGLTexture::setSize(): Cube map textures must be square");
        d->dimensions[0] = d->dimensions[1] = width;
        Q_UNUSED(depth);
        break;

    case Texture::Target3D:
        d->dimensions[0] = width;
        d->dimensions[1] = height;
        d->dimensions[2] = depth;
        break;
    }
}

/*!
    Returns the width of a 1D, 2D or 3D texture.

    \sa height(), depth(), setSize()
*/
int Texture::width() const
{
    Q_D(const Texture);
    return d->dimensions[0];
}

/*!
    Returns the height of a 2D or 3D texture.

    \sa width(), depth(), setSize()
*/
int Texture::height() const
{
    Q_D(const Texture);
    return d->dimensions[1];
}

/*!
    Returns the depth of a 3D texture.

    \sa width(), height(), setSize()
*/
int Texture::depth() const
{
    Q_D(const Texture);
    return d->dimensions[2];
}

/*!
    For texture targets that support mipmaps, this function
    sets the requested number of mipmap \a levels to allocate storage
    for. This function should be called before storage is allocated
    for the texture.

    If the texture target does not support mipmaps this function
    has no effect.

    \sa mipLevels(), maximumMipLevels(), isStorageAllocated()
*/
void Texture::setMipLevels(int levels)
{
    Q_D(Texture);
    d->create();
    if (isStorageAllocated()) {
        qWarning("Cannot set mip levels on a texture that already has storage allocated.\n"
                 "To do so, destroy() the texture and then create() and setMipLevels()");
        return;
    }

    switch (d->target) {
    case Texture::Target1D:
    case Texture::Target1DArray:
    case Texture::Target2D:
    case Texture::Target2DArray:
    case Texture::TargetCubeMap:
    case Texture::TargetCubeMapArray:
    case Texture::Target3D:
        d->requestedMipLevels = levels;
        break;

    case Texture::TargetBuffer:
    case Texture::TargetRectangle:
    case Texture::Target2DMultisample:
    case Texture::Target2DMultisampleArray:
        qWarning("QAbstractOpenGLTexture::setMipLevels(): This texture target does not support mipmaps");
        break;
    }
}

/*!
    Returns the number of mipmap levels for this texture. If storage
    has not yet been allocated for this texture it returns the
    requested number of mipmap levels.

    \sa setMipLevels(), maximumMipLevels(), isStorageAllocated()
*/
int Texture::mipLevels() const
{
    Q_D(const Texture);
    return isStorageAllocated() ? d->mipLevels : d->requestedMipLevels;
}

/*!
    Returns the maximum number of mipmap levels that this texture
    can have given the current dimensions.

    \sa setMipLevels(), mipLevels(), setSize()
*/
int Texture::maximumMipLevels() const
{
    Q_D(const Texture);
    return d->maximumMipLevelCount();
}

/*!
    Sets the number of array \a layers to allocate storage for. This
    function should be called before storage is allocated for the texture.

    For targets that do not support array layers this function has
    no effect.

    \sa layers(), isStorageAllocated()
*/
void Texture::setLayers(int layers)
{
    Q_D(Texture);
    d->create();
    if (isStorageAllocated()) {
        qWarning("Cannot set layers on a texture that already has storage allocated.\n"
                 "To do so, destroy() the texture and then create() and setLayers()");
        return;
    }

    switch (d->target) {
    case Texture::Target1DArray:
    case Texture::Target2DArray:
    case Texture::TargetCubeMapArray:
    case Texture::Target2DMultisampleArray:
        d->layers = layers;
        break;

    case Texture::Target1D:
    case Texture::Target2D:
    case Texture::Target3D:
    case Texture::TargetCubeMap:
    case Texture::TargetBuffer:
    case Texture::TargetRectangle:
    case Texture::Target2DMultisample:
        qWarning("Texture target does not support array layers");
        break;
    }
}

/*!
    Returns the number of array layers for this texture. If
    storage has not yet been allocated for this texture then
    this function returns the requested number of array layers.

    For texture targets that do not support array layers this
    will return 1.

    \sa setLayers(), isStorageAllocated()
*/
int Texture::layers() const
{
    Q_D(const Texture);
    return d->layers;
}

/*!
    Returns the number of faces for this texture. For cubemap
    and cubemap array type targets this will be 6.

    For non-cubemap type targets this will return 1.
*/
int Texture::faces() const
{
    Q_D(const Texture);
    return d->faces;
}

/*!
    Sets the number of \a samples to allocate storage for when rendering to
    a multisample capable texture target. This function should
    be called before storage is allocated for the texture.

    For targets that do not support multisampling this function has
    no effect.

    \sa samples(), isStorageAllocated()
*/
void Texture::setSamples(int samples)
{
    Q_D(Texture);
    d->create();
    if (isStorageAllocated()) {
        qWarning("Cannot set sample count on a texture that already has storage allocated.\n"
                 "To do so, destroy() the texture and then create() and setSamples()");
        return;
    }

    switch (d->target) {
    case Texture::Target2DMultisample:
    case Texture::Target2DMultisampleArray:
        d->samples = samples;
        break;

    case Texture::Target1D:
    case Texture::Target2D:
    case Texture::Target3D:
    case Texture::Target1DArray:
    case Texture::Target2DArray:
    case Texture::TargetCubeMap:
    case Texture::TargetCubeMapArray:
    case Texture::TargetBuffer:
    case Texture::TargetRectangle:

        qWarning("Texture target does not support multisampling");
        break;
    }
}

/*!
    Returns the number of multisample sample points for this texture.
    If storage has not yet been allocated for this texture then
    this function returns the requested number of samples.

    For texture targets that do not support multisampling this
    will return 0.

    \sa setSamples(), isStorageAllocated()
*/
int Texture::samples() const
{
    Q_D(const Texture);
    return d->samples;
}

/*!
    Sets whether the sample positions and number of samples used with
    a multisample capable texture target to \a fixed. If set to \c true
    the sample positions and number of samples used are the same for
    all texels in the image and will not depend upon the image size or
    internal format. This function should be called before storage is allocated
    for the texture.

    For targets that do not support multisampling this function has
    no effect.

    The default value is \c true.

    \sa isFixedSamplePositions(), isStorageAllocated()
*/
void Texture::setFixedSamplePositions(bool fixed)
{
    Q_D(Texture);
    d->create();
    if (isStorageAllocated()) {
        qWarning("Cannot set sample positions on a texture that already has storage allocated.\n"
                 "To do so, destroy() the texture and then create() and setFixedSamplePositions()");
        return;
    }

    switch (d->target) {
    case Texture::Target2DMultisample:
    case Texture::Target2DMultisampleArray:
        d->fixedSamplePositions = fixed;
        break;

    case Texture::Target1D:
    case Texture::Target2D:
    case Texture::Target3D:
    case Texture::Target1DArray:
    case Texture::Target2DArray:
    case Texture::TargetCubeMap:
    case Texture::TargetCubeMapArray:
    case Texture::TargetBuffer:
    case Texture::TargetRectangle:

        qWarning("Texture target does not support multisampling");
        break;
    }
}

/*!
    Returns whether this texture uses a fixed pattern of multisample
    samples. If storage has not yet been allocated for this texture then
    this function returns the requested fixed sample position setting.

    For texture targets that do not support multisampling this
    will return \c true.

    \sa setFixedSamplePositions(), isStorageAllocated()
*/
bool Texture::isFixedSamplePositions() const
{
    Q_D(const Texture);
    return d->fixedSamplePositions;
}

/*!
    Allocates server-side storage for this texture object taking
    into account, the format, dimensions, mipmap levels, array
    layers and cubemap faces.

    Once storage has been allocated it is no longer possible to change
    these properties.

    If supported Texture makes use of immutable texture
    storage.

    Once storage has been allocated for the texture then pixel data
    can be uploaded via one of the setData() overloads.

    \note If immutable texture storage is not available,
    then a default pixel format and pixel type will be used to
    create the mutable storage. You can use the other
    allocateStorage() overload to specify exactly the pixel format
    and the pixel type to use when allocating mutable storage;
    this is particulary useful under certain OpenGL ES implementations
    (notably, OpenGL ES 2), where the pixel format and the pixel type
    used at allocation time must perfectly match the format
    and the type passed to any subsequent setData() call.

    \sa isStorageAllocated(), setData()
*/
void Texture::allocateStorage()
{
    Q_D(Texture);
    if (d->create()) {
        const Texture::PixelFormat pixelFormat = pixelFormatCompatibleWithInternalFormat(d->format);
        const Texture::PixelType pixelType = pixelTypeCompatibleWithInternalFormat(d->format);
        d->allocateStorage(pixelFormat, pixelType);
    }
}

/*!
    \since 5.5

    Allocates server-side storage for this texture object taking
    into account, the format, dimensions, mipmap levels, array
    layers and cubemap faces.

    Once storage has been allocated it is no longer possible to change
    these properties.

    If supported Texture makes use of immutable texture
    storage. However, if immutable texture storage is not available,
    then the specified \a pixelFormat and \a pixelType will be used
    to allocate mutable storage; note that in certain OpenGL implementations
    (notably, OpenGL ES 2) they must perfectly match the format
    and the type passed to any subsequent setData() call.

    Once storage has been allocated for the texture then pixel data
    can be uploaded via one of the setData() overloads.

    \sa isStorageAllocated(), setData()
*/
void Texture::allocateStorage(Texture::PixelFormat pixelFormat, Texture::PixelType pixelType)
{
    Q_D(Texture);
    if (d->create())
        d->allocateStorage(pixelFormat, pixelType);
}

/*!
    Returns \c true if server-side storage for this texture as been
    allocated.

    The texture format, dimensions, mipmap levels and array layers
    cannot be altered once storage ihas been allocated.

    \sa allocateStorage(), setSize(), setMipLevels(), setLayers(), setFormat()
*/
bool Texture::isStorageAllocated() const
{
    Q_D(const Texture);
    return d->storageAllocated;
}

/*!
    Attempts to create a texture view onto this texture. A texture
    view is somewhat analogous to a view in SQL in that it presents
    a restricted or reinterpreted view of the original data. Texture
    views do not allocate any more server-side storage, insted relying
    on the storage buffer of the source texture.

    Texture views are only available when using immutable storage. For
    more information on texture views see
    http://www.opengl.org/wiki/Texture_Storage#Texture_views.

    The \a target argument specifies the target to use for the view.
    Only some targets can be used depending upon the target of the original
    target. For e.g. a view onto a Target1DArray texture can specify
    either Target1DArray or Target1D but for the latter the number of
    array layers specified with \a minimumLayer and \a maximumLayer must
    be exactly 1.

    Simpliar constraints apply for the \a viewFormat. See the above link
    and the specification for more details.

    The \a minimumMipmapLevel, \a maximumMipmapLevel, \a minimumLayer,
    and \a maximumLayer arguments serve to restrict the parts of the
    texture accessible by the texture view.

    If creation of the texture view fails this function will return
    0. If the function succeeds it will return a pointer to a new
    Texture object that will return \c true from its isTextureView()
    function.

    \sa isTextureView()
*/
Texture *Texture::createTextureView(Target target,
                                                  TextureFormat viewFormat,
                                                  int minimumMipmapLevel, int maximumMipmapLevel,
                                                  int minimumLayer, int maximumLayer) const
{
    Q_D(const Texture);
    if (!isStorageAllocated()) {
        qWarning("Cannot set create a texture view of a texture that does not have storage allocated.");
        return 0;
    }
    Q_ASSERT(maximumMipmapLevel >= minimumMipmapLevel);
    Q_ASSERT(maximumLayer >= minimumLayer);
    return d->createTextureView(target, viewFormat,
                                minimumMipmapLevel, maximumMipmapLevel,
                                minimumLayer, maximumLayer);
}

/*!
    Returns \c true if this texture object is actually a view onto another
    texture object.

    \sa createTextureView()
*/
bool Texture::isTextureView() const
{
    Q_D(const Texture);
    Q_ASSERT(d->textureId);
    return d->textureView;
}

/*!
    Uploads pixel \a data for this texture object \a mipLevel, array \a layer, and \a cubeFace.
    Storage must have been allocated before uploading pixel data. Some overloads of setData()
    will set appropriate dimensions, mipmap levels, and array layers and then allocate storage
    for you if they have enough information to do so. This will be noted in the function
    documentation.

    The structure of the pixel data pointed to by \a data is specified by \a sourceFormat
    and \a sourceType. The pixel data upload can optionally be controlled by \a options.

    If using a compressed format() then you should use setCompressedData() instead of this
    function.

    \since 5.3
    \sa setCompressedData()
*/
void Texture::setData(int mipLevel, int layer, CubeMapFace cubeFace,
                             PixelFormat sourceFormat, PixelType sourceType,
                             const void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    if (!isStorageAllocated()) {
        qWarning("Cannot set data on a texture that does not have storage allocated.\n"
                 "To do so call allocateStorage() before this function");
        return;
    }
    d->setData(mipLevel, layer, 1, cubeFace, sourceFormat, sourceType, data, options);
}

/*!
    \since 5.9
    \overload

    Parameter \a layerCount is the number of layers in a texture array
    that are being uploaded/populated by this call.
*/
void Texture::setData(int mipLevel, int layer, int layerCount, Texture::CubeMapFace cubeFace, Texture::PixelFormat sourceFormat, Texture::PixelType sourceType, const void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    if (!isStorageAllocated()) {
        qWarning("Cannot set data on a texture that does not have storage allocated.\n"
                 "To do so call allocateStorage() before this function");
        return;
    }
    d->setData(mipLevel, layer, layerCount, cubeFace, sourceFormat, sourceType, data, options);
}

/*!
    \since 5.3
    \overload
*/
void Texture::setData(int mipLevel, int layer,
                             PixelFormat sourceFormat, PixelType sourceType,
                             const void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->setData(mipLevel, layer, 1, Texture::CubeMapPositiveX, sourceFormat, sourceType, data, options);
}

/*!
    \since 5.3
    \overload
*/
void Texture::setData(int mipLevel,
                             PixelFormat sourceFormat, PixelType sourceType,
                             const void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->setData(mipLevel, 0, 1, Texture::CubeMapPositiveX, sourceFormat, sourceType, data, options);
}

/*!
    \since 5.3
    \overload
*/
void Texture::setData(PixelFormat sourceFormat, PixelType sourceType,
                             const void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->setData(0, 0, 1, Texture::CubeMapPositiveX, sourceFormat, sourceType, data, options);
}

#if QT_DEPRECATED_SINCE(5, 3)
/*!
    \obsolete
    \overload

    \sa setCompressedData()
*/
void Texture::setData(int mipLevel, int layer, CubeMapFace cubeFace,
                             PixelFormat sourceFormat, PixelType sourceType,
                             void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    if (!isStorageAllocated()) {
        qWarning("Cannot set data on a texture that does not have storage allocated.\n"
                 "To do so call allocateStorage() before this function");
        return;
    }
    d->setData(mipLevel, layer, 1, cubeFace, sourceFormat, sourceType, data, options);
}

/*!
    \obsolete
    \overload
*/
void Texture::setData(int mipLevel, int layer,
                             PixelFormat sourceFormat, PixelType sourceType,
                             void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->setData(mipLevel, layer, 1, Texture::CubeMapPositiveX, sourceFormat, sourceType, data, options);
}

/*!
    \obsolete
    \overload
*/
void Texture::setData(int mipLevel,
                             PixelFormat sourceFormat, PixelType sourceType,
                             void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->setData(mipLevel, 0, 1, Texture::CubeMapPositiveX, sourceFormat, sourceType, data, options);
}

/*!
    \obsolete
    \overload
*/
void Texture::setData(PixelFormat sourceFormat, PixelType sourceType,
                             void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->setData(0, 0, 1, Texture::CubeMapPositiveX, sourceFormat, sourceType, data, options);
}
#endif

/*!
    This overload of setData() will allocate storage for you.
    The pixel data is contained in \a image. Mipmaps are generated by default.
    Set \a genMipMaps to \l DontGenerateMipMaps to turn off mipmap generation.

    \overload
*/
void Texture::setData(const QImage& image, MipMapGeneration genMipMaps)
{
    QOpenGLContext *context = QOpenGLContext::currentContext();
    if (!context) {
        qWarning("Texture::setData() requires a valid current context");
        return;
    }

    if (image.isNull()) {
        qWarning("Texture::setData() tried to set a null image");
        return;
    }

    if (context->isOpenGLES() && context->format().majorVersion() < 3)
        setFormat(Texture::RGBAFormat);
    else
        setFormat(Texture::RGBA8_UNorm);

    setSize(image.width(), image.height());
    setMipLevels(genMipMaps == GenerateMipMaps ? maximumMipLevels() : 1);
    allocateStorage(Texture::RGBA, Texture::UInt8);

    // Upload pixel data and generate mipmaps
    QImage glImage = image.convertToFormat(QImage::Format_RGBA8888);
    QOpenGLPixelTransferOptions uploadOptions;
    uploadOptions.setAlignment(1);
    setData(0, Texture::RGBA, Texture::UInt8, glImage.constBits(), &uploadOptions);
}

/*!
    Uploads compressed pixel \a data to \a mipLevel, array \a layer, and \a cubeFace.
    The pixel transfer can optionally be controlled with \a options. The \a dataSize
    argument should specify the size of the data pointed to by \a data.

    If not using a compressed format() then you should use setData() instead of this
    function.

    \since 5.3
*/
void Texture::setCompressedData(int mipLevel, int layer, CubeMapFace cubeFace,
                                       int dataSize, const void *data,
                                       const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    if (!isStorageAllocated()) {
        qWarning("Cannot set data on a texture that does not have storage allocated.\n"
                 "To do so call allocateStorage() before this function");
        return;
    }
    d->setCompressedData(mipLevel, layer, 1, cubeFace, dataSize, data, options);
}

/*!
    \since 5.9
    \overload

    Parameter \a layerCount is the number of layers in a texture array
    that are being uploaded/populated by this call.
*/
void Texture::setCompressedData(int mipLevel, int layer, int layerCount, Texture::CubeMapFace cubeFace, int dataSize, const void *data, const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    if (!isStorageAllocated()) {
        qWarning("Cannot set data on a texture that does not have storage allocated.\n"
                 "To do so call allocateStorage() before this function");
        return;
    }
    d->setCompressedData(mipLevel, layer, layerCount, cubeFace, dataSize, data, options);
}

/*!
    \overload
*/
void Texture::setCompressedData(int mipLevel, int layer, int dataSize, const void *data,
                                       const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->setCompressedData(mipLevel, layer, 1, Texture::CubeMapPositiveX, dataSize, data, options);
}

/*!
    \overload
*/
void Texture::setCompressedData(int mipLevel, int dataSize, const void *data,
                                       const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->setCompressedData(mipLevel, 0, 1, Texture::CubeMapPositiveX, dataSize, data, options);
}

/*!
    \overload
*/
void Texture::setCompressedData(int dataSize, const void *data,
                                       const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->setCompressedData(0, 0, 1, Texture::CubeMapPositiveX, dataSize, data, options);
}

#if QT_DEPRECATED_SINCE(5, 3)
/*!
    \obsolete
    \overload
*/
void Texture::setCompressedData(int mipLevel, int layer, CubeMapFace cubeFace,
                                       int dataSize, void *data,
                                       const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    if (!isStorageAllocated()) {
        qWarning("Cannot set data on a texture that does not have storage allocated.\n"
                 "To do so call allocateStorage() before this function");
        return;
    }
    d->setCompressedData(mipLevel, layer, 1, cubeFace, dataSize, data, options);
}

/*!
    \obsolete
    \overload
*/
void Texture::setCompressedData(int mipLevel, int layer, int dataSize, void *data,
                                       const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->setCompressedData(mipLevel, layer, 1, Texture::CubeMapPositiveX, dataSize, data, options);
}

/*!
    \obsolete
    \overload
*/
void Texture::setCompressedData(int mipLevel, int dataSize, void *data,
                                       const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->setCompressedData(mipLevel, 0, 1, Texture::CubeMapPositiveX, dataSize, data, options);
}

/*!
    \obsolete
    \overload
*/
void Texture::setCompressedData(int dataSize, void *data,
                                       const QOpenGLPixelTransferOptions * const options)
{
    Q_D(Texture);
    Q_ASSERT(d->textureId);
    d->setCompressedData(0, 0, 1, Texture::CubeMapPositiveX, dataSize, data, options);
}
#endif

/*!
    Returns \c true if your OpenGL implementation and version supports the texture
    feature \a feature.
*/
bool Texture::hasFeature(Feature feature)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        qWarning("Texture::hasFeature() requires a valid current context");
        return false;
    }

    QSurfaceFormat f = ctx->format();

    bool supported = false;

#if !defined(QT_OPENGL_ES_2)
    if (!ctx->isOpenGLES()) {
        switch (feature) {
        case ImmutableMultisampleStorage:
            supported = f.version() >= qMakePair(4, 3)
                    || ctx->hasExtension(QByteArrayLiteral("GL_ARB_texture_storage_multisample"));
            break;

        case TextureBuffer:
            supported = f.version() >= qMakePair(3, 0)
                    || ctx->hasExtension(QByteArrayLiteral("GL_ARB_texture_buffer_object"));
            break;

        case StencilTexturing:
            supported = f.version() >= qMakePair(4, 3)
                    || ctx->hasExtension(QByteArrayLiteral("GL_ARB_stencil_texturing"));
            break;

        case ImmutableStorage:
            supported = f.version() >= qMakePair(4, 2)
                    || ctx->hasExtension(QByteArrayLiteral("GL_ARB_texture_storage"))
                    || ctx->hasExtension(QByteArrayLiteral("GL_EXT_texture_storage"));
            break;

        case TextureCubeMapArrays:
            supported = f.version() >= qMakePair(4, 0)
                    || ctx->hasExtension(QByteArrayLiteral("ARB_texture_cube_map_array"));
            break;

        case Swizzle:
            supported = f.version() >= qMakePair(3, 3)
                    || ctx->hasExtension(QByteArrayLiteral("GL_ARB_texture_swizzle"));
            break;

        case TextureMultisample:
            supported = f.version() >= qMakePair(3, 2)
                    || ctx->hasExtension(QByteArrayLiteral("GL_ARB_texture_multisample"));
            break;

        case TextureArrays:
            supported = f.version() >= qMakePair(3, 0)
                    || ctx->hasExtension(QByteArrayLiteral("GL_EXT_texture_array"));
            break;

        case TextureRectangle:
            supported = f.version() >= qMakePair(2, 1)
                    || ctx->hasExtension(QByteArrayLiteral("ARB_texture_rectangle"));
            break;

        case Texture3D:
            supported = f.version() >= qMakePair(1, 3);
            break;

        case AnisotropicFiltering:
            supported = ctx->hasExtension(QByteArrayLiteral("GL_EXT_texture_filter_anisotropic"));
            break;

        case NPOTTextures:
        case NPOTTextureRepeat:
            supported = ctx->hasExtension(QByteArrayLiteral("GL_ARB_texture_non_power_of_two"));
            break;

        case Texture1D:
            supported = f.version() >= qMakePair(1, 1);
            break;

        case TextureComparisonOperators:
            // GL 1.4 and GL_ARB_shadow alone support only LEQUAL and GEQUAL;
            // since we're talking about history anyhow avoid to be extra pedantic
            // in the feature set, and simply claim supported if we have the full set of operators
            // (which has been added into 1.5 / GL_EXT_shadow_funcs).
            supported = f.version() >= qMakePair(1, 5)
                    || (ctx->hasExtension(QByteArrayLiteral("GL_ARB_shadow"))
                        && ctx->hasExtension(QByteArrayLiteral("GL_EXT_shadow_funcs")));
            break;

        case TextureMipMapLevel:
            supported = f.version() >= qMakePair(1, 2);
            break;

        case MaxFeatureFlag:
            break;
        }
    }

    if (ctx->isOpenGLES())
#endif
    {
        const char *renderer = reinterpret_cast<const char *>(ctx->functions()->glGetString(GL_RENDERER));
        switch (feature) {
        case ImmutableStorage:
            supported = (f.version() >= qMakePair(3, 0) || ctx->hasExtension(QByteArrayLiteral("GL_EXT_texture_storage")))
                && !(renderer && strstr(renderer, "Mali")); // do not use on Mali: QTBUG-45106
            break;

        case ImmutableMultisampleStorage:
            supported = f.version() >= qMakePair(3, 1);
            break;

        case TextureRectangle:
            break;

        case TextureArrays:
            supported = f.version() >= qMakePair(3, 0);
            break;

        case Texture3D:
            supported = f.version() >= qMakePair(3, 0)
                    || ctx->hasExtension(QByteArrayLiteral("GL_OES_texture_3D"));
            break;

        case TextureMultisample:
            supported = f.version() >= qMakePair(3, 1);
            break;

        case TextureBuffer:
            break;

        case TextureCubeMapArrays:
            break;

        case Swizzle:
            supported = f.version() >= qMakePair(3, 0);
            break;

        case StencilTexturing:
            break;

        case AnisotropicFiltering:
            supported = ctx->hasExtension(QByteArrayLiteral("GL_EXT_texture_filter_anisotropic"));
            break;

        case NPOTTextures:
        case NPOTTextureRepeat:
            supported = f.version() >= qMakePair(3,0)
                    || ctx->hasExtension(QByteArrayLiteral("GL_OES_texture_npot"))
                    || ctx->hasExtension(QByteArrayLiteral("GL_ARB_texture_non_power_of_two"));
            break;

        case Texture1D:
            break;

        case TextureComparisonOperators:
            supported = f.version() >= qMakePair(3, 0)
                    || ctx->hasExtension(QByteArrayLiteral("GL_EXT_shadow_samplers"));
            break;

        case TextureMipMapLevel:
            supported = f.version() >= qMakePair(3, 0);
            break;

        case MaxFeatureFlag:
            break;
        }
    }

    return supported;
}

/*!
    Sets the base mipmap level used for all texture lookups with this texture to \a baseLevel.

    \note This function has no effect on Qt built for OpenGL ES 2.
    \sa mipBaseLevel(), setMipMaxLevel(), setMipLevelRange()
*/
void Texture::setMipBaseLevel(int baseLevel)
{
    Q_D(Texture);
    d->create();
    if (!d->features.testFlag(TextureMipMapLevel)) {
        qWarning("Texture::setMipBaseLevel: requires OpenGL >= 1.2 or OpenGL ES >= 3.0");
        return;
    }
    Q_ASSERT(d->textureId);
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(baseLevel <= d->maxLevel);
    d->baseLevel = baseLevel;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_BASE_LEVEL, baseLevel);
}

/*!
    Returns the mipmap base level used for all texture lookups with this texture.
    The default is 0.

    \sa setMipBaseLevel(), mipMaxLevel(), mipLevelRange()
*/
int Texture::mipBaseLevel() const
{
    Q_D(const Texture);
    return d->baseLevel;
}

/*!
    Sets the maximum mipmap level used for all texture lookups with this texture to \a maxLevel.

    \note This function has no effect on Qt built for OpenGL ES 2.
    \sa mipMaxLevel(), setMipBaseLevel(), setMipLevelRange()
*/
void Texture::setMipMaxLevel(int maxLevel)
{
    Q_D(Texture);
    d->create();
    if (!d->features.testFlag(TextureMipMapLevel)) {
        qWarning("Texture::setMipMaxLevel: requires OpenGL >= 1.2 or OpenGL ES >= 3.0");
        return;
    }
    Q_ASSERT(d->textureId);
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->baseLevel <= maxLevel);
    d->maxLevel = maxLevel;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAX_LEVEL, maxLevel);
}

/*!
    Returns the mipmap maximum level used for all texture lookups with this texture.

    \sa setMipMaxLevel(), mipBaseLevel(), mipLevelRange()
*/
int Texture::mipMaxLevel() const
{
    Q_D(const Texture);
    return d->maxLevel;
}

/*!
    Sets the range of mipmap levels that can be used for texture lookups with this texture
    to range from \a baseLevel to \a maxLevel.

    \note This function has no effect on Qt built for OpenGL ES 2.
    \sa setMipBaseLevel(), setMipMaxLevel(), mipLevelRange()
*/
void Texture::setMipLevelRange(int baseLevel, int maxLevel)
{
    Q_D(Texture);
    d->create();
    if (!d->features.testFlag(TextureMipMapLevel)) {
        qWarning("Texture::setMipLevelRange: requires OpenGL >= 1.2 or OpenGL ES >= 3.0");
        return;
    }
    Q_ASSERT(d->textureId);
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(baseLevel <= maxLevel);
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_BASE_LEVEL, baseLevel);
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAX_LEVEL, maxLevel);
}

/*!
    Returns the range of mipmap levels that can be used for texture lookups with this texture.

    \sa mipBaseLevel(), mipMaxLevel()
*/
QPair<int, int> Texture::mipLevelRange() const
{
    Q_D(const Texture);
    return qMakePair(d->baseLevel, d->maxLevel);
}

/*!
    If \a enabled is \c true, enables automatic mipmap generation for this texture object
    to occur whenever the level 0 mipmap data is set via setData().

    The automatic mipmap generation is enabled by default.

    \note Mipmap generation is not supported for compressed textures with OpenGL ES 2.0.

    \sa isAutoMipMapGenerationEnabled(), generateMipMaps()
*/
void Texture::setAutoMipMapGenerationEnabled(bool enabled)
{
    Q_D(Texture);
    d->autoGenerateMipMaps = enabled;
}

/*!
    Returns whether auto mipmap generation is enabled for this texture object.

    \sa setAutoMipMapGenerationEnabled(), generateMipMaps()
*/
bool Texture::isAutoMipMapGenerationEnabled() const
{
    Q_D(const Texture);
    return d->autoGenerateMipMaps;
}

/*!
    Generates mipmaps for this texture object from mipmap level 0. If you are
    using a texture target and filtering option that requires mipmaps and you
    have disabled automatic mipmap generation then you need to call this function
    or the overload to create the mipmap chain.

    \note Mipmap generation is not supported for compressed textures with OpenGL
    ES 2.0.

    \sa setAutoMipMapGenerationEnabled(), setMipLevels(), mipLevels()
*/
void Texture::generateMipMaps()
{
    Q_D(Texture);
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    if (isCompressedFormat(d->format)) {
        if (QOpenGLContext *ctx = QOpenGLContext::currentContext())
            if (ctx->isOpenGLES() && ctx->format().majorVersion() < 3)
                return;
    }
    d->texFuncs->glGenerateTextureMipmap(d->textureId, d->target, d->bindingTarget);
}

/*!
    Generates mipmaps for this texture object from mipmap level \a baseLevel. If you are
    using a texture target and filtering option that requires mipmaps and you
    have disabled automatic mipmap generation then you need to call this function
    or the overload to create the mipmap chain.

    The generation of mipmaps to above \a baseLevel is achieved by setting the mipmap
    base level to \a baseLevel and then generating the mipmap chain. If \a resetBaseLevel
    is \c true, then the baseLevel of the texture will be reset to its previous value.

    \sa setAutoMipMapGenerationEnabled(), setMipLevels(), mipLevels()
*/
void Texture::generateMipMaps(int baseLevel, bool resetBaseLevel)
{
    Q_D(Texture);
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    if (isCompressedFormat(d->format)) {
        if (QOpenGLContext *ctx = QOpenGLContext::currentContext())
            if (ctx->isOpenGLES() && ctx->format().majorVersion() < 3)
                return;
    }
    int oldBaseLevel;
    if (resetBaseLevel)
        oldBaseLevel = mipBaseLevel();
    setMipBaseLevel(baseLevel);
    d->texFuncs->glGenerateTextureMipmap(d->textureId, d->target, d->bindingTarget);
    if (resetBaseLevel)
        setMipBaseLevel(oldBaseLevel);
}

/*!
    GLSL shaders are able to reorder the components of the vec4 returned by texture
    functions. It is also desirable to be able to control this reordering from CPU
    side code. This is made possible by swizzle masks since OpenGL 3.3.

    Each component of the texture can be mapped to one of the SwizzleValue options.

    This function maps \a component to the output \a value.

    \note This function has no effect on Mac and Qt built for OpenGL ES 2.
    \sa swizzleMask()
*/
void Texture::setSwizzleMask(SwizzleComponent component, SwizzleValue value)
{
#if !defined(Q_OS_MAC) && !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(Texture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        if (!d->features.testFlag(Swizzle)) {
            qWarning("Texture::setSwizzleMask() requires OpenGL >= 3.3");
            return;
        }
        d->swizzleMask[component - SwizzleRed] = value;
        d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, component, value);
        return;
    }
#else
    Q_UNUSED(component);
    Q_UNUSED(value);
#endif
    qWarning("Texture: Texture swizzling is not supported");
}

/*!
    Parameters \a {r}, \a {g}, \a {b}, and \a {a}  are values used for setting
    the colors red, green, blue, and the alpha value.
    \overload
*/
void Texture::setSwizzleMask(SwizzleValue r, SwizzleValue g,
                                    SwizzleValue b, SwizzleValue a)
{
#if !defined(Q_OS_MAC) && !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(Texture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        if (!d->features.testFlag(Swizzle)) {
            qWarning("Texture::setSwizzleMask() requires OpenGL >= 3.3");
            return;
        }
        GLint swizzleMask[] = {GLint(r), GLint(g), GLint(b), GLint(a)};
        d->swizzleMask[0] = r;
        d->swizzleMask[1] = g;
        d->swizzleMask[2] = b;
        d->swizzleMask[3] = a;
        d->texFuncs->glTextureParameteriv(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        return;
    }
#else
    Q_UNUSED(r);
    Q_UNUSED(g);
    Q_UNUSED(b);
    Q_UNUSED(a);
#endif
    qWarning("Texture: Texture swizzling is not supported");
}

/*!
    Returns the swizzle mask for texture \a component.
*/
Texture::SwizzleValue Texture::swizzleMask(SwizzleComponent component) const
{
    Q_D(const Texture);
    return d->swizzleMask[component - SwizzleRed];
}

/*!
    \enum Texture::DepthStencilMode
    \since 5.4
    This enum specifies which component of a depth/stencil texture is
    accessed when the texture is sampled.

    \value DepthMode Equivalent to GL_DEPTH_COMPONENT.
    \value StencilMode Equivalent to GL_STENCIL_INDEX.
*/

/*!
    If using a texture that has a combined depth/stencil format this function sets
    which component of the texture is accessed to \a mode.

    When the parameter is set to DepthMode, then accessing it from the
    shader will access the depth component as a single float, as normal. But when
    the parameter is set to StencilMode, the shader will access the stencil component.

    \note This function has no effect on Mac and Qt built for OpenGL ES 2.
    \since 5.4
    \sa depthStencilMode()
*/
void Texture::setDepthStencilMode(Texture::DepthStencilMode mode)
{
#if !defined(Q_OS_MAC) && !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(Texture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        if (!d->features.testFlag(StencilTexturing)) {
            qWarning("Texture::setDepthStencilMode() requires OpenGL >= 4.3 or GL_ARB_stencil_texturing");
            return;
        }
        d->depthStencilMode = mode;
        d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_DEPTH_STENCIL_TEXTURE_MODE, mode);
        return;
    }
#else
    Q_UNUSED(mode);
#endif
    qWarning("Texture: DepthStencil Mode is not supported");
}

/*!
    Returns the depth stencil mode for textures using a combined depth/stencil format.

    \since 5.4
    \sa setDepthStencilMode()
*/
Texture::DepthStencilMode Texture::depthStencilMode() const
{
    Q_D(const Texture);
    return d->depthStencilMode;
}

/*!
    \enum Texture::ComparisonFunction
    \since 5.5
    This enum specifies which comparison operator is used when texture comparison
    is enabled on this texture.

    \value CompareLessEqual Equivalent to GL_LEQUAL.
    \value CompareGreaterEqual Equivalent to GL_GEQUAL.
    \value CompareLess Equivalent to GL_LESS.
    \value CompareGreater Equivalent to GL_GREATER.
    \value CompareEqual Equivalent to GL_EQUAL.
    \value CommpareNotEqual Equivalent to GL_NOTEQUAL.
    \value CompareAlways Equivalent to GL_ALWAYS.
    \value CompareNever Equivalent to GL_NEVER.

*/

/*
    \since 5.5

    Sets the texture comparison function on this texture to \a function. The texture
    comparison function is used by shadow samplers when sampling a depth texture.

    \sa comparisonFunction()
*/
void Texture::setComparisonFunction(Texture::ComparisonFunction function)
{
    Q_D(Texture);
    d->create();
    if (!d->features.testFlag(TextureComparisonOperators)) {
        qWarning("Texture::setComparisonFunction: requires OpenGL >= 1.5 or OpenGL ES >= 3.0");
        return;
    }
    d->comparisonFunction = function;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_COMPARE_FUNC, function);
}

/*!
    \since 5.5

    Returns the texture comparison operator set on this texture. By default, a
    texture has a CompareLessEqual comparison function.

    \sa setComparisonFunction()
*/
Texture::ComparisonFunction Texture::comparisonFunction() const
{
    Q_D(const Texture);
    return d->comparisonFunction;
}

/*!
    \enum Texture::ComparisonMode
    \since 5.5
    This enum specifies which comparison mode is used when sampling this texture.

    \value CompareRefToTexture Equivalent to GL_COMPARE_REF_TO_TEXTURE.
    \value CompareNone Equivalent to GL_NONE.
*/

/*!
    \since 5.5

    Sets the texture comparison mode on this texture to \a mode. The texture
    comparison mode is used by shadow samplers when sampling a depth texture.

    \sa comparisonMode()
*/
void Texture::setComparisonMode(Texture::ComparisonMode mode)
{
    Q_D(Texture);
    d->create();
    if (!d->features.testFlag(TextureComparisonOperators)) {
        qWarning("Texture::setComparisonMode: requires OpenGL >= 1.5 or OpenGL ES >= 3.0");
        return;
    }
    d->comparisonMode = mode;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_COMPARE_MODE, mode);
}

/*!
    \since 5.5

    Returns the texture comparison mode set on this texture. By default, a
    texture has a CompareNone comparison mode (i.e. comparisons are disabled).

    \sa setComparisonMode()
*/
Texture::ComparisonMode Texture::comparisonMode() const
{
    Q_D(const Texture);
    return d->comparisonMode;
}

/*!
    Sets the filter used for minification to \a filter.

    \sa minificationFilter(), setMagnificationFilter(), setMinMagFilters()
*/
void Texture::setMinificationFilter(Texture::Filter filter)
{
    Q_D(Texture);
    d->create();
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    d->minFilter = filter;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MIN_FILTER, filter);
}

/*!
    Returns the minification filter.

    \sa setMinificationFilter()
*/
Texture::Filter Texture::minificationFilter() const
{
    Q_D(const Texture);
    return d->minFilter;
}

/*!
    Sets the magnification filter to \a filter.

    \sa magnificationFilter(), setMinificationFilter(), setMinMagFilters()
*/
void Texture::setMagnificationFilter(Texture::Filter filter)
{
    Q_D(Texture);
    d->create();
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    d->magFilter = filter;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAG_FILTER, filter);
}

/*!
    Returns the magnification filter.

    \sa setMagnificationFilter()
*/
Texture::Filter Texture::magnificationFilter() const
{
    Q_D(const Texture);
    return d->magFilter;
}

/*!
    Sets the minification filter to \a minificationFilter and the magnification filter
    to \a magnificationFilter.

    \sa minMagFilters(), setMinificationFilter(), setMagnificationFilter()
*/
void Texture::setMinMagFilters(Texture::Filter minificationFilter,
                                      Texture::Filter magnificationFilter)
{
    Q_D(Texture);
    d->create();
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    d->minFilter = minificationFilter;
    d->magFilter = magnificationFilter;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MIN_FILTER, minificationFilter);
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAG_FILTER, magnificationFilter);
}

/*!
    Returns the current minification and magnification filters.

    \sa setMinMagFilters()
*/
QPair<Texture::Filter, Texture::Filter> Texture::minMagFilters() const
{
    Q_D(const Texture);
    return QPair<Texture::Filter, Texture::Filter>(d->minFilter, d->magFilter);
}

/*!
    If your OpenGL implementation supports the GL_EXT_texture_filter_anisotropic extension
    this function sets the maximum anisotropy level to \a anisotropy.

    \sa maximumAnisotropy()
*/
void Texture::setMaximumAnisotropy(float anisotropy)
{
    Q_D(Texture);
    d->create();
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    if (!d->features.testFlag(AnisotropicFiltering)) {
        qWarning("Texture::setMaximumAnisotropy() requires GL_EXT_texture_filter_anisotropic");
        return;
    }
    d->maxAnisotropy = anisotropy;
    d->texFuncs->glTextureParameteri(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
}

/*!
    Returns the maximum level of anisotropy to be accounted for when performing texture lookups.
    This requires the GL_EXT_texture_filter_anisotropic extension.

    \sa setMaximumAnisotropy()
*/
float Texture::maximumAnisotropy() const
{
    Q_D(const Texture);
    return d->maxAnisotropy;
}

/*!
    Sets the wrap (or repeat mode) for all texture dimentions to \a mode.

    \sa wrapMode()
*/
void Texture::setWrapMode(Texture::WrapMode mode)
{
    Q_D(Texture);
    d->create();
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    d->setWrapMode(mode);
}

/*!
    Holds the texture dimension \a direction.
    \overload
*/
void Texture::setWrapMode(Texture::CoordinateDirection direction, Texture::WrapMode mode)
{
    Q_D(Texture);
    d->create();
    Q_ASSERT(d->texFuncs);
    Q_ASSERT(d->textureId);
    d->setWrapMode(direction, mode);
}

/*!
    Returns the wrap mode for the texture dimension \a direction.

    \sa setWrapMode()
*/
Texture::WrapMode Texture::wrapMode(Texture::CoordinateDirection direction) const
{
    Q_D(const Texture);
    return d->wrapMode(direction);
}

/*!
    Sets the border color of the texture to \a color.

    \note This function has no effect on Mac and Qt built for OpenGL ES 2.
    \sa borderColor()
*/
void Texture::setBorderColor(QColor color)
{
    setBorderColor(static_cast<float>(color.redF()), static_cast<float>(color.greenF()),
                   static_cast<float>(color.blueF()), static_cast<float>(color.alphaF()));
}

/*!
    Sets the color red to \a {r}, green to \a {g}, blue to \a {b}, and \a {a} to the
    alpha value.
    \overload
*/
void Texture::setBorderColor(float r, float g, float b, float a)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(Texture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        float values[4];
        values[0] = r;
        values[1] = g;
        values[2] = b;
        values[3] = a;
        d->borderColor.clear();
        for (int i = 0; i < 4; ++i)
            d->borderColor.append(QVariant(values[i]));
        d->texFuncs->glTextureParameterfv(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_BORDER_COLOR, values);
        return;
    }
#else
    Q_UNUSED(r);
    Q_UNUSED(g);
    Q_UNUSED(b);
    Q_UNUSED(a);
#endif
    qWarning("Texture: Border color is not supported");
}

/*!
    Sets the color red to \a {r}, green to \a {g}, blue to \a {b}, and the alpha
    value to \a {a}.
    \overload
*/
void Texture::setBorderColor(int r, int g, int b, int a)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(Texture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        int values[4];
        values[0] = r;
        values[1] = g;
        values[2] = b;
        values[3] = a;
        d->borderColor.clear();
        for (int i = 0; i < 4; ++i)
            d->borderColor.append(QVariant(values[i]));
        d->texFuncs->glTextureParameteriv(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_BORDER_COLOR, values);
        return;
    }
#else
    Q_UNUSED(r);
    Q_UNUSED(g);
    Q_UNUSED(b);
    Q_UNUSED(a);
#endif
    qWarning("Texture: Border color is not supported");

    // TODO Handle case of using glTextureParameterIiv() based on format
}

/*!
    Sets the color red to \a {r}, green to \a {g}, blue to \a {b}, and the alpha
    value to \a {a}.
    \overload
*/
void Texture::setBorderColor(uint r, uint g, uint b, uint a)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(Texture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        int values[4];
        values[0] = int(r);
        values[1] = int(g);
        values[2] = int(b);
        values[3] = int(a);
        d->borderColor.clear();
        for (int i = 0; i < 4; ++i)
            d->borderColor.append(QVariant(values[i]));
        d->texFuncs->glTextureParameteriv(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_BORDER_COLOR, values);
        return;
    }
#else
    Q_UNUSED(r);
    Q_UNUSED(g);
    Q_UNUSED(b);
    Q_UNUSED(a);
#endif
    qWarning("Texture: Border color is not supported");

    // TODO Handle case of using glTextureParameterIuiv() based on format
}

/*!
    Returns the borderColor of this texture.

    \sa setBorderColor()
*/
QColor Texture::borderColor() const
{
    Q_D(const Texture);
    QColor c(0.0f, 0.0f, 0.0f, 0.0f);
    if (!d->borderColor.isEmpty()) {
        c.setRedF(d->borderColor.at(0).toFloat());
        c.setGreenF(d->borderColor.at(1).toFloat());
        c.setBlueF(d->borderColor.at(2).toFloat());
        c.setAlphaF(d->borderColor.at(3).toFloat());
    }
    return c;
}

/*!
    Writes the texture border color into the first four elements
    of the array pointed to by \a border.

    \sa setBorderColor()
*/
void Texture::borderColor(float *border) const
{
    Q_D(const Texture);
    Q_ASSERT(border);
    if (d->borderColor.isEmpty()) {
        for (int i = 0; i < 4; ++i)
            border[i] = 0.0f;
    } else {
        for (int i = 0; i < 4; ++i)
            border[i] = d->borderColor.at(i).toFloat();
    }
}

/*!
    Writes the texture border color into the first four elements
    of the array pointed to by \a border.

    \overload
*/
void Texture::borderColor(int *border) const
{
    Q_D(const Texture);
    Q_ASSERT(border);
    if (d->borderColor.isEmpty()) {
        for (int i = 0; i < 4; ++i)
            border[i] = 0;
    } else {
        for (int i = 0; i < 4; ++i)
            border[i] = d->borderColor.at(i).toInt();
    }
}

/*!
    Writes the texture border color into the first four elements
    of the array pointed to by \a border.

    \overload
*/
void Texture::borderColor(unsigned int *border) const
{
    Q_D(const Texture);
    Q_ASSERT(border);
    if (d->borderColor.isEmpty()) {
        for (int i = 0; i < 4; ++i)
            border[i] = 0;
    } else {
        for (int i = 0; i < 4; ++i)
            border[i] = d->borderColor.at(i).toUInt();
    }
}

/*!
    Sets the minimum level of detail to \a value. This limits the selection of highest
    resolution mipmap (lowest mipmap level). The default value is -1000.

    \note This function has no effect on Qt built for OpenGL ES 2.
    \sa minimumLevelOfDetail(), setMaximumLevelOfDetail(), setLevelOfDetailRange()
*/
void Texture::setMinimumLevelOfDetail(float value)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(Texture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        Q_ASSERT(value < d->maxLevelOfDetail);
        d->minLevelOfDetail = value;
        d->texFuncs->glTextureParameterf(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MIN_LOD, value);
        return;
    }
#else
    Q_UNUSED(value);
#endif
    qWarning("Texture: Detail level is not supported");
}

/*!
    Returns the minimum level of detail parameter.

    \sa setMinimumLevelOfDetail(), maximumLevelOfDetail(), levelOfDetailRange()
*/
float Texture::minimumLevelOfDetail() const
{
    Q_D(const Texture);
    return d->minLevelOfDetail;
}

/*!
    Sets the maximum level of detail to \a value. This limits the selection of lowest
    resolution mipmap (highest mipmap level). The default value is 1000.

    \note This function has no effect on Qt built for OpenGL ES 2.
    \sa maximumLevelOfDetail(), setMinimumLevelOfDetail(), setLevelOfDetailRange()
*/
void Texture::setMaximumLevelOfDetail(float value)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(Texture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        Q_ASSERT(value > d->minLevelOfDetail);
        d->maxLevelOfDetail = value;
        d->texFuncs->glTextureParameterf(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAX_LOD, value);
        return;
    }
#else
    Q_UNUSED(value);
#endif
    qWarning("Texture: Detail level is not supported");
}

/*!
    Returns the maximum level of detail parameter.

    \sa setMaximumLevelOfDetail(), minimumLevelOfDetail(), levelOfDetailRange()
*/
float Texture::maximumLevelOfDetail() const
{
    Q_D(const Texture);
    return d->maxLevelOfDetail;
}

/*!
    Sets the minimum level of detail parameters to \a min and the maximum level
    to \a max.
    \note This function has no effect on Qt built for OpenGL ES 2.
    \sa levelOfDetailRange(), setMinimumLevelOfDetail(), setMaximumLevelOfDetail()
*/
void Texture::setLevelOfDetailRange(float min, float max)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(Texture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        Q_ASSERT(min < max);
        d->minLevelOfDetail = min;
        d->maxLevelOfDetail = max;
        d->texFuncs->glTextureParameterf(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MIN_LOD, min);
        d->texFuncs->glTextureParameterf(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_MAX_LOD, max);
        return;
    }
#else
    Q_UNUSED(min);
    Q_UNUSED(max);
#endif
    qWarning("Texture: Detail level is not supported");
}

/*!
    Returns the minimum and maximum level of detail parameters.

    \sa setLevelOfDetailRange(), minimumLevelOfDetail(), maximumLevelOfDetail()
*/
QPair<float, float> Texture::levelOfDetailRange() const
{
    Q_D(const Texture);
    return qMakePair(d->minLevelOfDetail, d->maxLevelOfDetail);
}

/*!
    Sets the level of detail bias to \a bias.
    Level of detail bias affects the point at which mipmapping levels change.
    Increasing values for level of detail bias makes the overall images blurrier
    or smoother. Decreasing values make the overall images sharper.

    \note This function has no effect on Qt built for OpenGL ES 2.
    \sa levelofDetailBias()
*/
void Texture::setLevelofDetailBias(float bias)
{
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES()) {
        Q_D(Texture);
        d->create();
        Q_ASSERT(d->texFuncs);
        Q_ASSERT(d->textureId);
        d->levelOfDetailBias = bias;
        d->texFuncs->glTextureParameterf(d->textureId, d->target, d->bindingTarget, GL_TEXTURE_LOD_BIAS, bias);
        return;
    }
#else
    Q_UNUSED(bias);
#endif
    qWarning("Texture: Detail level is not supported");
}

/*!
    Returns the level of detail bias parameter.

    \sa setLevelofDetailBias()
*/
float Texture::levelofDetailBias() const
{
    Q_D(const Texture);
    return d->levelOfDetailBias;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const Texture *t)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Texture(";
    if (t) {
        const TexturePrivate *d = t->d_ptr.data();
        debug << d->target << ", bindingTarget=" << d->bindingTarget
            << ", size=[" << d->dimensions[0]
            << ", " << d->dimensions[1];
        if (d->target == Texture::Target3D)
            debug << ", " << d->dimensions[2];
        debug << "], format=" << d->format << ", formatClass=" << d->formatClass;
        if (t->isCreated())
            debug << ", textureId=" << d->textureId;
        if (t->isBound())
            debug << ", [bound]";
        if (t->isTextureView())
            debug << ", [view]";
        if (d->fixedSamplePositions)
            debug << ", [fixedSamplePositions]";
        debug << ", mipLevels=" << d->requestedMipLevels << ", layers=" << d->layers
            << ", faces=" << d->faces << ", samples=" << d->samples
            << ", depthStencilMode=" << d->depthStencilMode << ", comparisonFunction="
            << d->comparisonFunction << ", comparisonMode=" << d->comparisonMode
            << ", features=" << d->features << ", minificationFilter=" << d->minFilter
            << ", magnificationFilter=" << d->magFilter << ", wrapMode=" << d->wrapModes[0];
    } else {
        debug << '0';
    }
    debug << ')';
    return debug;
}
#endif // QT_NO_DEBUG_STREAM


}
