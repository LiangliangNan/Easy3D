#include "vk_shaders.h"

#include <fstream>
#include <sstream>
#include <algorithm>

#include <spirv_reflect.h>

#include <logging.h>
#include <vk_initializers.h>


namespace lve {

    // Returns the size in bytes of the provided VkFormat.
    // As this is only intended for vertex attribute formats, not all VkFormats are supported.
    static uint32_t FormatSize(VkFormat format) {
        uint32_t result = 0;
        switch (format) {
            case VK_FORMAT_UNDEFINED:
                result = 0;
                break;
            case VK_FORMAT_R4G4_UNORM_PACK8:
                result = 1;
                break;
            case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_R5G6B5_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_B5G6R5_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_R8_UNORM:
                result = 1;
                break;
            case VK_FORMAT_R8_SNORM:
                result = 1;
                break;
            case VK_FORMAT_R8_USCALED:
                result = 1;
                break;
            case VK_FORMAT_R8_SSCALED:
                result = 1;
                break;
            case VK_FORMAT_R8_UINT:
                result = 1;
                break;
            case VK_FORMAT_R8_SINT:
                result = 1;
                break;
            case VK_FORMAT_R8_SRGB:
                result = 1;
                break;
            case VK_FORMAT_R8G8_UNORM:
                result = 2;
                break;
            case VK_FORMAT_R8G8_SNORM:
                result = 2;
                break;
            case VK_FORMAT_R8G8_USCALED:
                result = 2;
                break;
            case VK_FORMAT_R8G8_SSCALED:
                result = 2;
                break;
            case VK_FORMAT_R8G8_UINT:
                result = 2;
                break;
            case VK_FORMAT_R8G8_SINT:
                result = 2;
                break;
            case VK_FORMAT_R8G8_SRGB:
                result = 2;
                break;
            case VK_FORMAT_R8G8B8_UNORM:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8_SNORM:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8_USCALED:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8_SSCALED:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8_UINT:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8_SINT:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8_SRGB:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_UNORM:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_SNORM:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_USCALED:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_SSCALED:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_UINT:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_SINT:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_SRGB:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8A8_UNORM:
                result = 4;
                break;
            case VK_FORMAT_R8G8B8A8_SNORM:
                result = 4;
                break;
            case VK_FORMAT_R8G8B8A8_USCALED:
                result = 4;
                break;
            case VK_FORMAT_R8G8B8A8_SSCALED:
                result = 4;
                break;
            case VK_FORMAT_R8G8B8A8_UINT:
                result = 4;
                break;
            case VK_FORMAT_R8G8B8A8_SINT:
                result = 4;
                break;
            case VK_FORMAT_R8G8B8A8_SRGB:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_UNORM:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_SNORM:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_USCALED:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_SSCALED:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_UINT:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_SINT:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_SRGB:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_UINT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_SINT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2R10G10B10_UINT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2R10G10B10_SINT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2B10G10R10_UINT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2B10G10R10_SINT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_R16_UNORM:
                result = 2;
                break;
            case VK_FORMAT_R16_SNORM:
                result = 2;
                break;
            case VK_FORMAT_R16_USCALED:
                result = 2;
                break;
            case VK_FORMAT_R16_SSCALED:
                result = 2;
                break;
            case VK_FORMAT_R16_UINT:
                result = 2;
                break;
            case VK_FORMAT_R16_SINT:
                result = 2;
                break;
            case VK_FORMAT_R16_SFLOAT:
                result = 2;
                break;
            case VK_FORMAT_R16G16_UNORM:
                result = 4;
                break;
            case VK_FORMAT_R16G16_SNORM:
                result = 4;
                break;
            case VK_FORMAT_R16G16_USCALED:
                result = 4;
                break;
            case VK_FORMAT_R16G16_SSCALED:
                result = 4;
                break;
            case VK_FORMAT_R16G16_UINT:
                result = 4;
                break;
            case VK_FORMAT_R16G16_SINT:
                result = 4;
                break;
            case VK_FORMAT_R16G16_SFLOAT:
                result = 4;
                break;
            case VK_FORMAT_R16G16B16_UNORM:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16_SNORM:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16_USCALED:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16_SSCALED:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16_UINT:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16_SINT:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16_SFLOAT:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16A16_UNORM:
                result = 8;
                break;
            case VK_FORMAT_R16G16B16A16_SNORM:
                result = 8;
                break;
            case VK_FORMAT_R16G16B16A16_USCALED:
                result = 8;
                break;
            case VK_FORMAT_R16G16B16A16_SSCALED:
                result = 8;
                break;
            case VK_FORMAT_R16G16B16A16_UINT:
                result = 8;
                break;
            case VK_FORMAT_R16G16B16A16_SINT:
                result = 8;
                break;
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                result = 8;
                break;
            case VK_FORMAT_R32_UINT:
                result = 4;
                break;
            case VK_FORMAT_R32_SINT:
                result = 4;
                break;
            case VK_FORMAT_R32_SFLOAT:
                result = 4;
                break;
            case VK_FORMAT_R32G32_UINT:
                result = 8;
                break;
            case VK_FORMAT_R32G32_SINT:
                result = 8;
                break;
            case VK_FORMAT_R32G32_SFLOAT:
                result = 8;
                break;
            case VK_FORMAT_R32G32B32_UINT:
                result = 12;
                break;
            case VK_FORMAT_R32G32B32_SINT:
                result = 12;
                break;
            case VK_FORMAT_R32G32B32_SFLOAT:
                result = 12;
                break;
            case VK_FORMAT_R32G32B32A32_UINT:
                result = 16;
                break;
            case VK_FORMAT_R32G32B32A32_SINT:
                result = 16;
                break;
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                result = 16;
                break;
            case VK_FORMAT_R64_UINT:
                result = 8;
                break;
            case VK_FORMAT_R64_SINT:
                result = 8;
                break;
            case VK_FORMAT_R64_SFLOAT:
                result = 8;
                break;
            case VK_FORMAT_R64G64_UINT:
                result = 16;
                break;
            case VK_FORMAT_R64G64_SINT:
                result = 16;
                break;
            case VK_FORMAT_R64G64_SFLOAT:
                result = 16;
                break;
            case VK_FORMAT_R64G64B64_UINT:
                result = 24;
                break;
            case VK_FORMAT_R64G64B64_SINT:
                result = 24;
                break;
            case VK_FORMAT_R64G64B64_SFLOAT:
                result = 24;
                break;
            case VK_FORMAT_R64G64B64A64_UINT:
                result = 32;
                break;
            case VK_FORMAT_R64G64B64A64_SINT:
                result = 32;
                break;
            case VK_FORMAT_R64G64B64A64_SFLOAT:
                result = 32;
                break;
            case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
                result = 4;
                break;

            default:
                LOGE("Unrecognized VkFormat (vertex attribute format), ask Liangliang to add the format to the list");
                break;
        }
        return result;
    }


    namespace vkutil {
        VkPipelineShaderStageCreateInfo load_shader(VkDevice device, const std::string& file_path, VkShaderStageFlagBits stage) {
            auto module = ShaderManager::loadShaderModule(device, file_path).module;
            assert(module != VK_NULL_HANDLE);
            return vkinit::pipeline_shader_stage_create_info(stage, module);
        }
    }


    void ShaderReflect::addStage(ShaderModule *shaderModule, VkShaderStageFlagBits stage) {
        stages.emplace_back(ShaderStage{shaderModule, stage});
    }


    void ShaderReflect::clear() {
        bindingDescription = {};
        attributeDescriptions.clear();
        reflectedBindings.clear();
        descriptorSetLayoutCreateInfos.clear();
        constantRanges.clear();
        shaderStages.clear();
    }


    void ShaderReflect::reflect(VkDevice device) {
        clear();

        struct DescriptorSetLayoutData {
            uint32_t set_number;
            VkDescriptorSetLayoutCreateInfo create_info;
            std::vector<VkDescriptorSetLayoutBinding> bindings;
        };
        std::vector<DescriptorSetLayoutData> set_layouts;

        for (auto &stage: stages) {
            shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(stage.stage, stage.shaderModule->module));

            SpvReflectShaderModule spvmodule;
            SpvReflectResult result = spvReflectCreateShaderModule(
                    stage.shaderModule->code.size() * sizeof(uint32_t),
                    stage.shaderModule->code.data(),
                    &spvmodule
            );
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            uint32_t count = 0;
            result = spvReflectEnumerateDescriptorSets(&spvmodule, &count, nullptr);
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            std::vector<SpvReflectDescriptorSet *> sets(count);
            result = spvReflectEnumerateDescriptorSets(&spvmodule, &count, sets.data());
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            for (std::size_t i_set = 0; i_set < sets.size(); ++i_set) {
                const SpvReflectDescriptorSet &refl_set = *(sets[i_set]);
                DescriptorSetLayoutData layout = {};

                layout.bindings.resize(refl_set.binding_count);
                for (uint32_t i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) {
                    const SpvReflectDescriptorBinding &refl_binding = *(refl_set.bindings[i_binding]);
                    VkDescriptorSetLayoutBinding &layout_binding = layout.bindings[i_binding];
                    layout_binding.binding = refl_binding.binding;
                    layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);

//                    for (std::size_t ov = 0; ov < overrides.size(); ov++) {
//                        if (strcmp(refl_binding.name, overrides[ov].name) == 0) {
//                            layout_binding.descriptorType = overrides[ov].overridenType;
//                        }
//                    }

                    layout_binding.descriptorCount = 1;
                    for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
                        layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
                    }
                    layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(spvmodule.shader_stage);

                    ReflectedBinding reflected;
                    reflected.binding = layout_binding.binding;
                    reflected.set = refl_set.set;
                    reflected.type = layout_binding.descriptorType;
                    reflected.stageFlags = layout_binding.stageFlags;

                    reflectedBindings[refl_binding.name] = reflected;
                }
                layout.set_number = refl_set.set;
                layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layout.create_info.bindingCount = refl_set.binding_count;
                layout.create_info.pBindings = layout.bindings.data();
                set_layouts.push_back(layout);

                // Nothing further is done with set_layouts.
                // In a real application, they would be merged with similar structures from other shader stages and/or pipelines
                // to create a VkPipelineLayout.
            }

            //pushconstants

            result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, nullptr);
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            std::vector<SpvReflectBlockVariable *> pconstants(count);
            result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, pconstants.data());
            assert(result == SPV_REFLECT_RESULT_SUCCESS);

            if (count > 0) {
                VkPushConstantRange pcs{};
                pcs.offset = pconstants[0]->offset;
                pcs.size = pconstants[0]->size;
                pcs.stageFlags = stage.stage;
                constantRanges.push_back(pcs);
            }

            // inputs: generate all necessary data structures to populate a VkPipelineVertexInputStateCreateInfo
            // structure, given the module's expected input variables.
            if (spvmodule.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
                count = 0;
                result = spvReflectEnumerateInputVariables(&spvmodule, &count, nullptr);
                assert(result == SPV_REFLECT_RESULT_SUCCESS);

                std::vector<SpvReflectInterfaceVariable *> input_vars(count);
                result = spvReflectEnumerateInputVariables(&spvmodule, &count, input_vars.data());
                assert(result == SPV_REFLECT_RESULT_SUCCESS);

                // Simplifying assumptions:
                // - All vertex input attributes are sourced from a single vertex buffer,
                //   bound to VB slot 0.
                // - Each vertex's attribute are laid out in ascending order by location.
                // - The format of each attribute matches its usage in the shader;
                //   float4 -> VK_FORMAT_R32G32B32A32_FLOAT, etc. No attribute compression is applied.
                // - All attributes are provided per-vertex, not per-instance.
                bindingDescription.binding = 0;
                bindingDescription.stride = 0;  // computed below
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                attributeDescriptions.reserve(input_vars.size());
                for (size_t i_var = 0; i_var < input_vars.size(); ++i_var) {
                    const SpvReflectInterfaceVariable &refl_var = *(input_vars[i_var]);
                    // ignore built-in variables
                    if (refl_var.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
                        continue;
                    }
                    VkVertexInputAttributeDescription attr_desc{};
                    attr_desc.location = refl_var.location;
                    attr_desc.binding = bindingDescription.binding;
                    attr_desc.format = static_cast<VkFormat>(refl_var.format);
                    attr_desc.offset = 0;  // final offset computed below after sorting.
                    attributeDescriptions.push_back(attr_desc);
                }
                // Sort attributes by location
                std::sort(std::begin(attributeDescriptions), std::end(attributeDescriptions),
                          [](const VkVertexInputAttributeDescription &a, const VkVertexInputAttributeDescription &b) {
                              return a.location < b.location;
                          });
                // Compute final offsets of each attribute, and total vertex stride.
                for (auto &attribute: attributeDescriptions) {
                    uint32_t format_size = FormatSize(attribute.format);
                    attribute.offset = bindingDescription.stride;
                    bindingDescription.stride += format_size;
                }

//                VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
//                vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//                vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
//                vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
//                vertexInputInfo.vertexBindingDescriptionCount = 1;
//                vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
                // Nothing further is done with attributeDescriptions or bindingDescription.
                // A real application would probably derive this information from its
                // mesh format(s); a similar mechanism could be used to ensure mesh/shader compatibility.
            }

            spvReflectDestroyShaderModule(&spvmodule);
        }

        // todo: only 4?
        std::array<DescriptorSetLayoutData, 4> merged_layouts;
        for (int i = 0; i < 4; i++) {
            DescriptorSetLayoutData &ly = merged_layouts[i];
            ly.set_number = i;
            ly.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

            std::unordered_map<int, VkDescriptorSetLayoutBinding> binds;
            for (auto &s: set_layouts) {
                if (s.set_number == i) {
                    for (auto &b: s.bindings) {
                        auto it = binds.find(b.binding);
                        if (it == binds.end()) {
                            binds[b.binding] = b;
                        } else {
                            //merge flags
                            binds[b.binding].stageFlags |= b.stageFlags;
                        }

                    }
                }
            }
            for (auto [k, v]: binds) {
                ly.bindings.push_back(v);
            }
            //sort the bindings, for hash purposes
            std::sort(ly.bindings.begin(), ly.bindings.end(),
                      [](VkDescriptorSetLayoutBinding &a, VkDescriptorSetLayoutBinding &b) {
                          return a.binding < b.binding;
                      });

            ly.create_info.bindingCount = (uint32_t) ly.bindings.size();
            ly.create_info.pBindings = ly.bindings.data();
            ly.create_info.flags = 0;
            ly.create_info.pNext = 0;

            if (ly.create_info.bindingCount > 0) {
                descriptorSetLayoutCreateInfos.push_back(ly.create_info);
            }
        }
    }


    ShaderManager::~ShaderManager() {
        for (auto &shader: module_cache)
            vkDestroyShaderModule(_device, shader.second.module, nullptr);
    }


    ShaderModule *ShaderManager::getShader(const std::string & file_name) {
        auto it = module_cache.find(file_name);
        if (it == module_cache.end()) {
            ShaderModule newShader = loadShaderModule(_device, file_name);
            module_cache[file_name] = newShader;
        }
        return &module_cache[file_name];
    }


    ShaderModule ShaderManager::loadShaderModule(VkDevice device, const std::string& filename) {
        ShaderModule shaderModule;

        // Extract extension name from the shader file
        const auto ext = filename.substr(filename.find_last_of('.') + 1);
        if (ext == "spv") { // it is binary SPV format
            //open the file. With cursor at the end
            std::ifstream file(filename, std::ios::ate | std::ios::binary);
            if (!file.is_open()) {
                LOGE("Failed to open shader file \"{}\"", filename);
                return shaderModule;
            }

            //find what the size of the file is by looking up the location of the cursor
            //because the cursor is at the end, it gives the size directly in bytes
            std::size_t fileSize = static_cast<std::size_t>(file.tellg());
            //spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
            std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
            //put file cursor at beginning
            file.seekg(0);
            //load the entire file into the buffer
            file.read((char*)buffer.data(), fileSize);
            //now that the file is loaded into the buffer, we can close it
            file.close();
            if (buffer.empty()) {
                LOGE("No data exists in shader file\'{}\'", filename);
                return shaderModule;
            }

            //create a new shader module, using the buffer we loaded
            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.pNext = nullptr;
            //codeSize has to be in bytes, so multply the ints in the buffer by size of int to know the real size of the buffer
            createInfo.codeSize = fileSize; // it can also be buffer.size() * sizeof(uint32_t);
            createInfo.pCode = buffer.data();
            //check that the creation goes well.
            VkShaderModule module;
            VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &module));

            shaderModule.code = std::move(buffer);
            shaderModule.module = module;
            return shaderModule;
        }
        else { 
            // then ASCII format (vert, frag, geom ...), we use glslangValidator to compile the shader into SPIR-V format.
            // we also save it into a file. Todo: have a GLSLCompiler based on glslang. 
            auto spv_filename = filename + ".spv";
            auto cmd = "glslangValidator --quiet --target-env vulkan1.0 -V " + filename + " -o " + spv_filename;
            std::system(cmd.c_str());
            std::ifstream file(spv_filename);
            if (file.is_open()) {
                LOGI("GLSL shader code compiled to SPIR-V format");
            }
            else {
                LOGE("Error executing command: {}", cmd);
                LOGE("Failed to compile shader code into SPIR-V format: {}", filename);
                return shaderModule;
            }

            return loadShaderModule(device, spv_filename); // now load the spv shader
        }
    }
}