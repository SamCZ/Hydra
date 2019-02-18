/* 
* Copyright (c) 2012-2018, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/ 

#pragma once
#include "GFSDK_NVRHI.h"
#include <d3d11_1.h>
#include <wrl.h>
#include <map>
#include <vector>
#include <set>
#include <mutex>

namespace NVRHI
{
  using namespace Microsoft::WRL;

  struct StageMask
  {
      enum Enum
      {
          DENY_SHADER_VERTEX = 0x1 << ShaderType::SHADER_VERTEX,
          DENY_SHADER_HULL = 0x1 << ShaderType::SHADER_HULL,
          DENY_SHADER_DOMAIN = 0x1 << ShaderType::SHADER_DOMAIN,
          DENY_SHADER_GEOMETRY = 0x1 << ShaderType::SHADER_GEOMETRY,
          DENY_SHADER_PIXEL = 0x1 << ShaderType::SHADER_PIXEL,
          DENY_INPUT_STATE = 0x20,
          DENY_RENDER_STATE = 0x40
      };
  };

  class Texture;
  class Buffer;

  struct ShaderResourceLimits;

  class RendererInterfaceD3D11 : public IRendererInterface
  {
  public:
    //The user-visible API
    RendererInterfaceD3D11(IErrorCallback* errorCB, ID3D11DeviceContext* context);
    virtual ~RendererInterfaceD3D11();

    void forgetAboutTexture(ID3D11Resource* resource);
    void forgetAboutBuffer(ID3D11Buffer* resource);

    //You should not call this while the client is initialized or else resources it is using may be deleted
    void clearCachedData();

    inline ID3D11DeviceContext* GetDeviceContext() const { return context.Get(); }
    inline ID3D11Device* GetDevice() const { return device.Get(); }

	ID3D11Resource* getResourceForTexture(TextureHandle handle);
    ID3D11ShaderResourceView* getSRVForTexture(TextureHandle handle, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, uint32_t mipLevel = 0);
    ID3D11RenderTargetView* getRTVForTexture(TextureHandle handle, uint32_t arrayItem = 0, uint32_t mipLevel = 0);
    ID3D11DepthStencilView* getDSVForTexture(TextureHandle handle, uint32_t arrayItem = 0, uint32_t mipLevel = 0);
    ID3D11UnorderedAccessView* getUAVForTexture(TextureHandle handle, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, uint32_t mipLevel = 0);

	ID3D11ShaderResourceView* getSRVForBuffer(BufferHandle resource, Format::Enum format);
	ID3D11UnorderedAccessView* getUAVForBuffer(BufferHandle resource);

	BufferHandle getHandleForBuffer(ID3D11Buffer* resource) { return getHandleForBufferInternal(resource, nullptr); }

  private:
    RendererInterfaceD3D11& operator=(const RendererInterfaceD3D11& other); //undefined
  protected:
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<ID3D11Device> device;
    IErrorCallback* errorCB;
    bool nvapiIsInitalized;
    ComPtr<ID3DUserDefinedAnnotation> userDefinedAnnotation;

    void signalError(const char* file, int line, const char* errorDesc);

    std::set<Texture*> textures;
	std::set<Buffer*> buffers;
    
    std::map<uint32_t, ComPtr<ID3D11BlendState>> blendStates;
    std::map<uint32_t, ComPtr<ID3D11DepthStencilState>> depthStencilStates;
    std::map<uint32_t, ComPtr<ID3D11RasterizerState>> rasterizerStates;

    std::set<PerformanceQueryHandle> perfQueries;

	std::mutex mutex;

    bool insideRenderingPass;
    
    D3D11_BLEND convertBlendValue(BlendState::BlendValue value);
    D3D11_BLEND_OP convertBlendOp(BlendState::BlendOp value);
    D3D11_STENCIL_OP convertStencilOp(DepthStencilState::StencilOp value);
    D3D11_COMPARISON_FUNC convertComparisonFunc(DepthStencilState::ComparisonFunc value);
    ID3D11BlendState* getBlendState(const BlendState& blendState);
    ID3D11DepthStencilState* getDepthStencilState(const DepthStencilState& depthStencilState);
    ID3D11RasterizerState* getRasterizerState(const RasterState& rasterState);

    //if there are multiple views to clear you can keep calling index until you get all null
    void getClearViewForTexture(TextureHandle resource, uint32_t index, bool asUINT, ID3D11UnorderedAccessView*& outUAV, ID3D11RenderTargetView*& outRTV, ID3D11DepthStencilView*& outDSV);

    
    //If we just created this texture pass in the texture desc, otherwise deduce it from D3D11
	TextureHandle getHandleForTextureInternal(ID3D11Resource* resource, const TextureDesc* textureDesc, Format::Enum formatOverride);
    BufferHandle getHandleForBufferInternal(ID3D11Buffer* resource, const BufferDesc* bufferDesc);

    TextureDesc getTextureDescFromD3D11Resource(ID3D11Resource* resource, Format::Enum formatOverride);
    BufferDesc getBufferDescFromD3D11Buffer(ID3D11Buffer* buffer);

    D3D_PRIMITIVE_TOPOLOGY getPrimType(PrimitiveType::Enum pt);
    
    void disableSLIResouceSync(ID3D11Resource* resource);
  public:
  
    //These are the methods in the in the IRendererInteface inteface that are implemented by us

    virtual TextureHandle createTexture(const TextureDesc& d, const void* data);
    virtual const TextureDesc& describeTexture(TextureHandle t);
    virtual void clearTextureFloat(TextureHandle t, const Color& clearColor);
    virtual void clearTextureUInt(TextureHandle t, uint32_t clearColor);
    virtual void writeTexture(TextureHandle t, uint32_t subresource, const void* data, uint32_t rowPitch, uint32_t depthPitch);
    virtual bool readTexture(TextureHandle t, void* data, size_t rowPitch);
    virtual void destroyTexture(TextureHandle t);
	virtual void resolveTexture(TextureHandle dst, TextureHandle src, Format::Enum format, uint32_t dstSubres, uint32_t srcSubres);
    virtual void* handoffTexture(TextureHandle t) override;
    virtual TextureHandle getHandleForTexture(void* resource, Format::Enum formatOverride = Format::UNKNOWN) override;

    virtual BufferHandle createBuffer(const BufferDesc& d, const void* data);
    virtual void writeBuffer(BufferHandle b, const void* data, size_t dataSize);
    virtual void clearBufferUInt(BufferHandle b, uint32_t clearValue);
    virtual void copyToBuffer(BufferHandle dest, uint32_t destOffsetBytes, BufferHandle src, uint32_t srcOffsetBytes, size_t dataSizeBytes);
    virtual void readBuffer(BufferHandle b, void* data, size_t* dataSize);
    virtual void destroyBuffer(BufferHandle b);

    virtual ConstantBufferHandle createConstantBuffer(const ConstantBufferDesc& d, const void* data);
    virtual void writeConstantBuffer(ConstantBufferHandle b, const void* data, size_t dataSize);
    virtual void destroyConstantBuffer(ConstantBufferHandle b);

    virtual ShaderHandle createShader(const ShaderDesc& d, const void* binary, const size_t binarySize);
    virtual ShaderHandle createShaderFromAPIInterface(ShaderType::Enum shaderType, const void* apiInterface);
    virtual void destroyShader(ShaderHandle s);

    virtual SamplerHandle createSampler(const SamplerDesc& d);
    virtual void destroySampler(SamplerHandle s);
    
    virtual InputLayoutHandle createInputLayout(const VertexAttributeDesc* d, uint32_t attributeCount, const void* vertexShaderBinary, const size_t binarySize);
    virtual void destroyInputLayout(InputLayoutHandle i);
    
    virtual PerformanceQueryHandle createPerformanceQuery(const char* name);
    virtual void destroyPerformanceQuery(PerformanceQueryHandle query);
    virtual void beginPerformanceQuery(PerformanceQueryHandle query, bool onlyAnnotation);
    virtual void endPerformanceQuery(PerformanceQueryHandle query);
    virtual float getPerformanceQueryTimeMS(PerformanceQueryHandle query);

    virtual GraphicsAPI::Enum getGraphicsAPI();
    virtual void* getAPISpecificInterface(APISpecificInterface::Enum interfaceType);
    virtual bool isOpenGLExtensionSupported(const char* name);
    virtual void* getOpenGLProcAddress(const char* procname);
    
    virtual void draw(const DrawCallState& state, const DrawArguments* args, uint32_t numDrawCalls);
    virtual void drawIndexed(const DrawCallState& state, const DrawArguments* args, uint32_t numDrawCalls);
    virtual void drawIndirect(const DrawCallState& state, BufferHandle indirectParams, uint32_t offsetBytes);
    virtual void dispatch(const DispatchState& state, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ);
    virtual void dispatchIndirect(const DispatchState& state, BufferHandle indirectParams, uint32_t offsetBytes);

	virtual void setModifiedWMode(bool enabled, uint32_t numViewports, const float* pA, const float* pB) override;

	virtual void setSinglePassStereoMode(bool enabled, uint32_t renderTargetIndexOffset, bool independentViewportMask) override;

    virtual uint32_t getNumberOfAFRGroups();
    virtual uint32_t getAFRGroupOfCurrentFrame(uint32_t numAFRGroups);

    virtual void setEnableUavBarriers(bool enableBarriers, const TextureHandle* textures = nullptr, size_t numTextures = 0, const BufferHandle* buffers = nullptr, size_t numBuffers = 0);

    virtual void beginRenderingPass();
    virtual void endRenderingPass();
    
    //These do not handle the pre/post commands
    void applyState(const DrawCallState& state, uint32_t denyStageMask = 0);
    void applyState(const DispatchState& state, ShaderResourceLimits& limits);
    void unapplyDispatchState(const ShaderResourceLimits& limits);
    void clearState();
  };

  struct UserState
  {
    void save(ID3D11DeviceContext* context);
    void restore(ID3D11DeviceContext* context);

    ID3D11InputLayout *pInputLayout;
    ID3D11Buffer *pIndexBuffer;
    DXGI_FORMAT IBFormat;
    uint32_t IBOffset;
    D3D11_PRIMITIVE_TOPOLOGY primitiveTopology;

    uint32_t numViewports;
    D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
    uint32_t numScissorRects;
    D3D11_RECT scissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
    ID3D11RasterizerState *pRS;

    ID3D11BlendState *pBlendState;
    float blendFactor[4];
    uint32_t sampleMask;
    ID3D11DepthStencilState *pDepthStencilState;
    uint32_t stencilRef;

    ID3D11VertexShader* pVS;
    ID3D11GeometryShader* pGS;
    ID3D11PixelShader* pPS;
    ID3D11ComputeShader* pCS;

    ID3D11Buffer* constantBuffersVS[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
    ID3D11Buffer* constantBuffersGS[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
    ID3D11Buffer* constantBuffersPS[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
    ID3D11Buffer* constantBuffersCS[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];

    ID3D11ShaderResourceView* shaderResourceViewsVS[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
    ID3D11ShaderResourceView* shaderResourceViewsGS[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
    ID3D11ShaderResourceView* shaderResourceViewsPS[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
    ID3D11ShaderResourceView* shaderResourceViewsCS[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
    ID3D11UnorderedAccessView* unorderedAccessViewsCS[D3D11_PS_CS_UAV_REGISTER_COUNT];

    ID3D11SamplerState* samplersVS[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
    ID3D11SamplerState* samplersGS[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
    ID3D11SamplerState* samplersPS[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
    ID3D11SamplerState* samplersCS[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];

    ID3D11RenderTargetView *pRTVs[8];
    ID3D11DepthStencilView* pDSV;
  };

} // namespace NVRHI
