#include "tilt.h"

// INPUT PARAMS
struct Params {
    float NA;
    uint32_t trunc_flag;
};

static size_t angles_buffer_len;
static size_t factors_buffer_len;

// CREATING BIND GROUP AND LAYOUT
static wgpu::BindGroupLayout createBindGroupLayout(wgpu::Device& device) {
    wgpu::BindGroupLayoutEntry anglesBufferLayout = {};
    anglesBufferLayout.binding = 0;
    anglesBufferLayout.visibility = wgpu::ShaderStage::Compute;
    anglesBufferLayout.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

    wgpu::BindGroupLayoutEntry shapeBufferLayout = {};
    shapeBufferLayout.binding = 1;
    shapeBufferLayout.visibility = wgpu::ShaderStage::Compute;
    shapeBufferLayout.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

    wgpu::BindGroupLayoutEntry resBufferLayout = {};
    resBufferLayout.binding = 2;
    resBufferLayout.visibility = wgpu::ShaderStage::Compute;
    resBufferLayout.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

    wgpu::BindGroupLayoutEntry factorsBufferLayout = {};
    factorsBufferLayout.binding = 3;
    factorsBufferLayout.visibility = wgpu::ShaderStage::Compute;
    factorsBufferLayout.buffer.type = wgpu::BufferBindingType::Storage;

    wgpu::BindGroupLayoutEntry uniformNALayout = {};
    uniformNALayout.binding = 4;
    uniformNALayout.visibility = wgpu::ShaderStage::Compute;
    uniformNALayout.buffer.type = wgpu::BufferBindingType::Uniform;

    wgpu::BindGroupLayoutEntry uniformTruncLayout = {};
    uniformTruncLayout.binding = 5;
    uniformTruncLayout.visibility = wgpu::ShaderStage::Compute;
    uniformTruncLayout.buffer.type = wgpu::BufferBindingType::Uniform;

    wgpu::BindGroupLayoutEntry entries[] = {
        anglesBufferLayout,
        shapeBufferLayout,
        resBufferLayout,
        factorsBufferLayout,
        uniformNALayout,
        uniformTruncLayout
    };

    wgpu::BindGroupLayoutDescriptor layoutDesc = {};
    layoutDesc.entryCount = 6;
    layoutDesc.entries = entries;

    return device.createBindGroupLayout(layoutDesc);
}

static wgpu::BindGroup createBindGroup(
    wgpu::Device& device, 
    wgpu::BindGroupLayout bindGroupLayout,
    wgpu::Buffer anglesBuffer,
    wgpu::Buffer shapeBuffer,
    wgpu::Buffer resBuffer,
    wgpu::Buffer factorsBuffer,
    wgpu::Buffer uniformNABuffer,
    wgpu::Buffer uniformTruncBuffer
) {
    wgpu::BindGroupEntry anglesEntry = {};
    anglesEntry.binding = 0;
    anglesEntry.buffer = anglesBuffer;
    anglesEntry.offset = 0;
    anglesEntry.size = sizeof(float) * angles_buffer_len;

    wgpu::BindGroupEntry shapeEntry = {};
    shapeEntry.binding = 1;
    shapeEntry.buffer = shapeBuffer;
    shapeEntry.offset = 0;
    shapeEntry.size = sizeof(uint32_t) * 2; // Always 2 elements for shape

    wgpu::BindGroupEntry resEntry = {};
    resEntry.binding = 2;
    resEntry.buffer = resBuffer;
    resEntry.offset = 0;
    resEntry.size = sizeof(float) * 3; // Always 3 elements for res

    wgpu::BindGroupEntry factorsEntry = {};
    factorsEntry.binding = 3;
    factorsEntry.buffer = factorsBuffer;
    factorsEntry.offset = 0;
    factorsEntry.size = sizeof(float) * factors_buffer_len;

    wgpu::BindGroupEntry uniformNAEntry = {};
    uniformNAEntry.binding = 4;
    uniformNAEntry.buffer = uniformNABuffer;
    uniformNAEntry.offset = 0;
    uniformNAEntry.size = sizeof(float);

    wgpu::BindGroupEntry uniformTruncEntry = {};
    uniformTruncEntry.binding = 5;
    uniformTruncEntry.buffer = uniformTruncBuffer;
    uniformTruncEntry.offset = 0;
    uniformTruncEntry.size = sizeof(uint32_t);

    wgpu::BindGroupEntry entries[] = {
        anglesEntry,
        shapeEntry,
        resEntry,
        factorsEntry,
        uniformNAEntry,
        uniformTruncEntry
    };

    wgpu::BindGroupDescriptor bindGroupDesc = {};
    bindGroupDesc.layout = bindGroupLayout;
    bindGroupDesc.entryCount = 6;
    bindGroupDesc.entries = entries;

    return device.createBindGroup(bindGroupDesc);
}

void tilt(
    WebGPUContext& context,
    wgpu::Buffer& factorsBuffer,
    const std::vector<float>& angles,
    const std::vector<uint32_t>& shape,
    float NA,
    const std::vector<float>& res,
    bool trunc
) {
    // Validate inputs
    assert(shape.size() == 2 && "Shape must be 2D (height, width)");
    assert(res.size() == 3 && "Resolution must have 3 components");
    
    angles_buffer_len = angles.size();
    factors_buffer_len = 2 * angles.size();
    
    Params params = {
        NA,
        trunc ? 1u : 0u
    };

    // INITIALIZING WEBGPU
    wgpu::Device device = context.device;
    wgpu::Queue queue = context.queue;

    // LOADING AND COMPILING SHADER CODE
    std::string shaderCode = readShaderFile("src/tilt/tilt.wgsl");
    wgpu::ShaderModule shaderModule = createShaderModule(device, shaderCode);
    
    // CREATING BUFFERS FOR TILT
    wgpu::Buffer anglesBuffer = createBuffer(device, angles.data(), sizeof(float) * angles_buffer_len, wgpu::BufferUsage::Storage);
    wgpu::Buffer shapeBuffer = createBuffer(device, shape.data(), sizeof(uint32_t) * 2, wgpu::BufferUsage::Storage);
    wgpu::Buffer resBuffer = createBuffer(device, res.data(), sizeof(float) * 3, wgpu::BufferUsage::Storage);
    wgpu::Buffer uniformNABuffer = createBuffer(device, &params.NA, sizeof(float), wgpu::BufferUsage::Uniform);
    wgpu::Buffer uniformTruncBuffer = createBuffer(device, &params.trunc_flag, sizeof(uint32_t), wgpu::BufferUsage::Uniform);

    // CREATING BIND GROUP AND LAYOUT
    wgpu::BindGroupLayout bindGroupLayout = createBindGroupLayout(device);
    wgpu::BindGroup bindGroup = createBindGroup(
        device, bindGroupLayout,
        anglesBuffer, shapeBuffer, resBuffer,
        factorsBuffer, uniformNABuffer, uniformTruncBuffer
    );

    // CREATING COMPUTE PIPELINE
    wgpu::ComputePipeline computePipeline = createComputePipeline(device, shaderModule, bindGroupLayout);

    // ENCODING AND DISPATCHING COMPUTE COMMANDS
    wgpu::CommandEncoderDescriptor encoderDesc = {};
    wgpu::CommandEncoder commandEncoder = device.createCommandEncoder(encoderDesc);

    wgpu::ComputePassDescriptor computePassDesc = {};
    wgpu::ComputePassEncoder computePass = commandEncoder.beginComputePass(computePassDesc);
    computePass.setPipeline(computePipeline);
    computePass.setBindGroup(0, bindGroup, 0, nullptr);
    computePass.dispatchWorkgroups(std::ceil(double(angles_buffer_len)/256.0),1,1);
    computePass.end();

    wgpu::CommandBufferDescriptor cmdBufferDesc = {};
    wgpu::CommandBuffer commandBuffer = commandEncoder.finish(cmdBufferDesc);

    queue.submit(1, &commandBuffer);
    
    // RELEASE RESOURCES
    bindGroup.release();
    bindGroupLayout.release();
    anglesBuffer.release();
    shapeBuffer.release();
    resBuffer.release();
    uniformNABuffer.release();
    uniformTruncBuffer.release();
    shaderModule.release();
    computePipeline.release();
    commandBuffer.release();
}