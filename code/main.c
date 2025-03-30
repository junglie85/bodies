#include <SDL3/SDL.h>

#include "application.h"
#include "error.h"
#include "log.h"
#include "window.h"

typedef struct material_vertex_t material_vertex_t;
struct material_vertex_t
{
    float position[2];
    float color[4];
};

SDL_GPUShader *load_shader(SDL_GPUDevice *device, const char *filename)
{
    SDL_GPUShaderStage stage;
    if (SDL_strstr(filename, ".vert")) {
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    } else if (SDL_strstr(filename, ".frag")) {
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    } else {
        log_error(LOG_CATEGORY_GPU, "Invalid shader stage for %s.", filename);
        return NULL;
    }

    // todo: use SDL_vsnprintf to get the len we need then allocate in scratch.
    char full_path[1024];
    SDL_GPUShaderFormat backend_formats = SDL_GetGPUShaderFormats(device);
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    const char *entrypoint;

    // todo: this is currently the build directory but can we add a feature flag and inject the project or debugger cwd?
    const char* base_path = SDL_GetBasePath();

    if (backend_formats & SDL_GPU_SHADERFORMAT_SPIRV) {
        SDL_snprintf(full_path, sizeof(full_path), "%s../data/%s.spv", base_path, filename);
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entrypoint = "main";
    } else if (backend_formats & SDL_GPU_SHADERFORMAT_MSL) {
        SDL_snprintf(full_path, sizeof(full_path), "%s../data/%s.msl", base_path, filename);
        format = SDL_GPU_SHADERFORMAT_MSL;
        entrypoint = "main0";
    } else if (backend_formats & SDL_GPU_SHADERFORMAT_DXIL) {
        SDL_snprintf(full_path, sizeof(full_path), "%s../data/%s.dxil", base_path, filename);
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entrypoint = "main";
    } else {
        log_error(LOG_CATEGORY_GPU, "Unsupported backend shader format.");
        return NULL;
    }

    size_t code_size;
    void *code = SDL_LoadFile(full_path, &code_size);
    if (code == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to load shader from path %s.", full_path);
        return NULL;
    }

    SDL_GPUShaderCreateInfo shader_info = {
    .code = code,
    .code_size = code_size,
    .entrypoint = entrypoint,
    .format = format,.stage = stage,
    .num_samplers = 0,.num_uniform_buffers = 0, .num_storage_buffers = 0, .num_storage_textures = 0};

    SDL_GPUShader *shader = SDL_CreateGPUShader(device, &shader_info);
    if (shader == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to create shader.");
        SDL_free(code);
        return NULL;
    }

    SDL_free(code);
    return shader;
}

int main(void)
{
    start_application();
    create_window("Bodies", 1920, 1080);

    // todo: add FEATURE_GPU_DEBUG_MODE
    SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
                                                false,
                                                NULL);
    if (device == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to create GPU device.");
        exit(GPU_DEVICE_CREATION_ERROR);
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window_handle())) {
        log_error(LOG_CATEGORY_GPU, "Failed to claim window for GPU device.");
        exit(GPU_WINDOW_CLAIM_ERROR);
    }

    // -----
    SDL_GPUShader *vertex_shader = load_shader(device, "material.vert");
    if (vertex_shader == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to create vertex shader.");
        exit_application(GPU_SHADER_CREATION_ERROR);
    }

    SDL_GPUShader *fragment_shader = load_shader(device, "material.frag");
    if (fragment_shader == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to create fragment shader.");
        exit_application(GPU_SHADER_CREATION_ERROR);
    }

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]){
                { .format = SDL_GetGPUSwapchainTextureFormat(device, window_handle()) },
            },
        },
        .vertex_input_state = (SDL_GPUVertexInputState){
            .num_vertex_buffers = 1,
            .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){
                {
                    .slot = 0,
                    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                    .instance_step_rate = 0,
                    .pitch = sizeof(material_vertex_t),
                },
            },
            .num_vertex_attributes = 2,
            .vertex_attributes = (SDL_GPUVertexAttribute[]){
                {
                    .buffer_slot = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                    .location = 0,
                    .offset = 0,
                },
                {
                    .buffer_slot = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                    .location = 1,
                    .offset = sizeof(float) * 2,
                },
            },
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,
    };

    SDL_GPUGraphicsPipeline *pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeline_info);
    ;
    if (pipeline == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to create graphics pipeline.");
        exit_application(GPU_GRAPHICS_PIPELINE_CREATION_ERROR);
    }

    SDL_ReleaseGPUShader(device, vertex_shader);
    SDL_ReleaseGPUShader(device, fragment_shader);

    SDL_GPUBuffer *vertex_buffer = SDL_CreateGPUBuffer(
        device,
        &(SDL_GPUBufferCreateInfo){
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            .size = sizeof(material_vertex_t) * 3,
        });

    // -- Upload vertex data to vertex buffer via a transfer buffer.
    SDL_GPUTransferBuffer *transfer_buffer = SDL_CreateGPUTransferBuffer(
        device,
        &(SDL_GPUTransferBufferCreateInfo){
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = sizeof(material_vertex_t) * 3,
        });

    material_vertex_t *transfer_data = SDL_MapGPUTransferBuffer(device, transfer_buffer, false);

    transfer_data[0] = (material_vertex_t){ 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f };
    transfer_data[1] = (material_vertex_t){ -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f };
    transfer_data[2] = (material_vertex_t){ 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f };

    SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

    SDL_GPUCommandBuffer *upload_cmd_buf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(upload_cmd_buf);

    SDL_UploadToGPUBuffer(
        copy_pass,
        &(SDL_GPUTransferBufferLocation){
            .transfer_buffer = transfer_buffer,
            .offset = 0,
        },
        &(SDL_GPUBufferRegion){
            .buffer = vertex_buffer,
            .offset = 0,
            .size = sizeof(material_vertex_t) * 3,
        },
        false);

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(upload_cmd_buf);
    SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);

    while (run_window_event_loop()) {
        if (close_window_requested()) {
            exit_window_event_loop();
            continue;
        }

        SDL_GPUCommandBuffer *cmd_buf = SDL_AcquireGPUCommandBuffer(device);
        if (cmd_buf == NULL) {
            log_error(LOG_CATEGORY_GPU, "AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            exit_window_event_loop();
            continue;
        }

        SDL_GPUTexture *swapchain_texture;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buf, window_handle(), &swapchain_texture, NULL, NULL)) {
            log_error(LOG_CATEGORY_GPU, "WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
            exit_window_event_loop();
            continue;
        }

        if (swapchain_texture != NULL) {
            SDL_GPUColorTargetInfo colorTargetInfo = {
                .texture = swapchain_texture,
                .clear_color = (SDL_FColor){ 0.3f, 0.4f, 0.5f, 1.0f },
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
            };

            SDL_GPURenderPass *rpass = SDL_BeginGPURenderPass(cmd_buf, &colorTargetInfo, 1, NULL);

            SDL_BindGPUGraphicsPipeline(rpass, pipeline);
            SDL_BindGPUVertexBuffers(rpass, 0, &(SDL_GPUBufferBinding){ .buffer = vertex_buffer, .offset = 0 }, 1);
            SDL_DrawGPUPrimitives(rpass, 3, 1, 0, 0);

            SDL_EndGPURenderPass(rpass);
        }

        SDL_SubmitGPUCommandBuffer(cmd_buf);
    }

    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    SDL_ReleaseGPUBuffer(device, vertex_buffer);

    SDL_ReleaseWindowFromGPUDevice(device, window_handle());
    SDL_DestroyGPUDevice(device);
    destroy_window();
    stop_application();

    return 0;
}
