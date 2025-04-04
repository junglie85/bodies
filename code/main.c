#include <SDL3/SDL.h>
#include <cglm/struct.h>

#include "application.h"
#include "error.h"
#include "log.h"
#include "memory.h"
#include "window.h"

// todo: perspective camera (game and editor).
// todo: cube with index buffers
// todo: move camera's with mouse
// todo: draw rect onto editor UI render target and composite onto swapchain
// todo: handle window resize (keep perspective, then just increase viewport width)
// todo: tidy up code.

typedef struct uniform_t uniform_t;
struct uniform_t
{
    mat4s mvp;
};

typedef struct camera_t camera_t;
struct camera_t
{
    vec2s position;
    vec2s size;
    float rotation;
    float zoom;
};

camera_t make_camera(const vec2s position, const vec2s size)
{
    return (camera_t){
        .position = position,
        .size = size,
        .rotation = 0.0f,
        .zoom = 1.0f,
    };
}

mat4s get_camera_view_matrix(const camera_t *const camera)
{
    const vec2s size = glms_vec2_scale(camera->size, camera->zoom);
    const mat4s projection = glms_ortho(0.0f, size.x, size.y, 0.0f, 0.0f, 1.0f);
    return projection;
}

mat4s get_camera_projection_matrix(const camera_t *const camera)
{
    vec2s offset = glms_vec2_scale(camera->size, 0.5f);
    vec2s origin = glms_vec2_add(camera->position, offset);

    // todo: use glms_lookat?
    mat4s view = glms_translate_make(glms_vec3_make((float[]){ camera->position.x, camera->position.y, 0.0f }));
    view = glms_mat4_mul(view, glms_translate_make(glms_vec3_make((float[]){ origin.x, origin.y, 0.0f })));
    view = glms_mat4_mul(view, glms_rotate_make(camera->rotation, glms_vec3_make((float[]){ 0.0f, 0.0f, 1.0f })));
    view = glms_mat4_mul(view, glms_translate_make(glms_vec3_negate(glms_vec3_make((float[]){ origin.x, origin.y, 0.0f }))));
    view = glms_mat4_mul(view, glms_scale_make(glms_vec3_make((float[]){ 1.0f, 1.0f, 1.0f })));
    view = glms_mat4_mul(view, glms_mat4_inv(view));
    return view;
}

typedef struct material_vertex_t material_vertex_t;
struct material_vertex_t
{
    float position[2];
    float uv[2];
    float color[4];
};

SDL_GPUShader *load_shader(SDL_GPUDevice *device, const char *filename, const int32_t sampler_count, const int32_t uniform_buffer_count)
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

    // todo: this is currently the build directory but can we add a feature flag and inject the project or debugger cwd?
    const char *base_path = SDL_GetBasePath();

    SDL_GPUShaderFormat backend_formats = SDL_GetGPUShaderFormats(device);
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    const char *entrypoint;

    // todo: use SDL_vsnprintf to get the len we need then allocate in scratch.
    // char full_path[1024];
    char *full_path = NULL;

    stack_allocator_t *scratch = mem_scratch_allocator();

    if (backend_formats & SDL_GPU_SHADERFORMAT_SPIRV) {
        int32_t len = SDL_snprintf(NULL, 0, "%s../data/%s.spv", base_path, filename);
        full_path = stack_alloc(scratch, len + 1, MEM_DEFAULT_ALIGN);
        SDL_snprintf(full_path, len + 1, "%s../data/%s.spv", base_path, filename);
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
        stack_dealloc(scratch, full_path);
        return NULL;
    }

    stack_dealloc(scratch, full_path);

    SDL_GPUShaderCreateInfo shader_info = {
        .code = code,
        .code_size = code_size,
        .entrypoint = entrypoint,
        .format = format,
        .stage = stage,
        .num_samplers = sampler_count,
        .num_uniform_buffers = uniform_buffer_count,
        .num_storage_buffers = 0,
        .num_storage_textures = 0
    };

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

    int32_t window_width = 0;
    int32_t window_height = 0;

    // ----- Swapchain render pipeline
    // ----- create shaders
    SDL_GPUShader *swapchain_vertex_shader = load_shader(device, "swapchain.vert", 0, 1);
    if (swapchain_vertex_shader == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to create swapchain vertex shader.");
        exit_application(GPU_SHADER_CREATION_ERROR);
    }

    SDL_GPUShader *swapchain_fragment_shader = load_shader(device, "swapchain.frag", 1, 0);
    if (swapchain_fragment_shader == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to create swapchain fragment shader.");
        exit_application(GPU_SHADER_CREATION_ERROR);
    }

    SDL_GPUGraphicsPipelineCreateInfo swapchain_pipeline_info = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]){
                { .format = SDL_GetGPUSwapchainTextureFormat(device, window_handle()) },
            },
        },
        .vertex_input_state = (SDL_GPUVertexInputState){
            .num_vertex_buffers = 0,
            .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){},
            .num_vertex_attributes = 0,
            .vertex_attributes = (SDL_GPUVertexAttribute[]){},
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = swapchain_vertex_shader,
        .fragment_shader = swapchain_fragment_shader,
    };

    SDL_GPUGraphicsPipeline *swapchain_pipeline = SDL_CreateGPUGraphicsPipeline(device, &swapchain_pipeline_info);

    if (swapchain_pipeline == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to create swapchain graphics pipeline.");
        exit_application(GPU_GRAPHICS_PIPELINE_CREATION_ERROR);
    }

    SDL_ReleaseGPUShader(device, swapchain_vertex_shader);
    SDL_ReleaseGPUShader(device, swapchain_fragment_shader);

    // ----- 3D render target pipeline
    // Render target texture needs to have the same dimensions as the swapchain.
    get_window_size(&window_width, &window_height);
    SDL_GPUTexture *render_target = SDL_CreateGPUTexture(
        device,
        &(SDL_GPUTextureCreateInfo){
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
            .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
            .width = window_width,
            .height = window_height,
            .layer_count_or_depth = 1,
            .num_levels = 1,
            .sample_count = 1,
        });
    if (render_target == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to create render target texture.");
        exit_application(GPU_TEXTURE_CREATION_ERROR);
    }

    // ----- create shaders
    SDL_GPUShader *material_vertex_shader = load_shader(device, "material.vert", 0, 1);
    if (material_vertex_shader == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to create material vertex shader.");
        exit_application(GPU_SHADER_CREATION_ERROR);
    }

    SDL_GPUShader *material_fragment_shader = load_shader(device, "material.frag", 1, 0);
    if (material_fragment_shader == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to create material fragment shader.");
        exit_application(GPU_SHADER_CREATION_ERROR);
    }

    SDL_GPUGraphicsPipelineCreateInfo material_pipeline_info = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]){
                { .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM },
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
            .num_vertex_attributes = 3,
            .vertex_attributes = (SDL_GPUVertexAttribute[]){
                {
                    .buffer_slot = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                    .location = 0,
                    .offset = 0,
                },
                {
                    .buffer_slot = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                    .location = 1,
                    .offset = sizeof(float) * 2,
                },
                {
                    .buffer_slot = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                    .location = 2,
                    .offset = sizeof(float) * 4,
                },
            },
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = material_vertex_shader,
        .fragment_shader = material_fragment_shader,
    };

    SDL_GPUGraphicsPipeline *material_pipeline = SDL_CreateGPUGraphicsPipeline(device, &material_pipeline_info);

    if (material_pipeline == NULL) {
        log_error(LOG_CATEGORY_GPU, "Failed to create material graphics pipeline.");
        exit_application(GPU_GRAPHICS_PIPELINE_CREATION_ERROR);
    }

    SDL_ReleaseGPUShader(device, material_vertex_shader);
    SDL_ReleaseGPUShader(device, material_fragment_shader);

    // Create samplers.

    SDL_GPUSampler *sampler = SDL_CreateGPUSampler(
        device,
        &(SDL_GPUSamplerCreateInfo){
            .min_filter = SDL_GPU_FILTER_NEAREST,
            .mag_filter = SDL_GPU_FILTER_NEAREST,
            .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
            .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        });

    // Create triangle.

    SDL_GPUBuffer *vertex_buffer = SDL_CreateGPUBuffer(
        device,
        &(SDL_GPUBufferCreateInfo){
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            .size = sizeof(material_vertex_t) * 3,
        });
    SDL_SetGPUBufferName(device, vertex_buffer, "triangle vertex buffer");

    // -- Write vertex data to vertex buffer via a transfer buffer.
    SDL_GPUTransferBuffer *vertex_transfer_buffer = SDL_CreateGPUTransferBuffer(
        device,
        &(SDL_GPUTransferBufferCreateInfo){
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = sizeof(material_vertex_t) * 3,
        });

    material_vertex_t *vertex_transfer_data = SDL_MapGPUTransferBuffer(device, vertex_transfer_buffer, false);

    vertex_transfer_data[0] = (material_vertex_t){ 0.0f, 0.0f, 0.0f, 0.0f, 0.7f, 0.1f, 0.1f, 1.0f };
    vertex_transfer_data[1] = (material_vertex_t){ 50.0f, 100.0f, 0.5f, 1.0f, 0.7f, 0.1f, 0.1f, 1.0f };
    vertex_transfer_data[2] = (material_vertex_t){ 100.0f, 0.0f, 1.0f, 0.0f, 0.7f, 0.1f, 0.1f, 1.0f };

    SDL_UnmapGPUTransferBuffer(device, vertex_transfer_buffer);

    // Create default material.
    uint8_t default_material_data[4] = { 0xff, 0xff, 0xff, 0xff }; // RGBA
    SDL_Surface *default_material_surface = SDL_CreateSurfaceFrom(1, 1, SDL_PIXELFORMAT_RGBA8888, default_material_data, 4);

    SDL_GPUTexture *default_material_texture = SDL_CreateGPUTexture(
        device,
        &(SDL_GPUTextureCreateInfo){
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, // todo: how do we use SRGB in SDL?
            .width = default_material_surface->w,
            .height = default_material_surface->h,
            .layer_count_or_depth = 1,
            .num_levels = 1,
            .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
        });
    SDL_SetGPUTextureName(device, default_material_texture, "default material");

    SDL_GPUTransferBuffer *texture_transfer_buffer = SDL_CreateGPUTransferBuffer(
        device,
        &(SDL_GPUTransferBufferCreateInfo){
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = default_material_surface->w * default_material_surface->h * 4,
        });

    uint8_t *texture_transfer_data = SDL_MapGPUTransferBuffer(device, texture_transfer_buffer, false);

    SDL_memcpy(texture_transfer_data, default_material_surface->pixels, default_material_surface->w * default_material_surface->h * 4);

    SDL_UnmapGPUTransferBuffer(device, texture_transfer_buffer);

    // Upload the data to the GPU.
    SDL_GPUCommandBuffer *upload_cmd_buf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(upload_cmd_buf);

    SDL_UploadToGPUBuffer(
        copy_pass,
        &(SDL_GPUTransferBufferLocation){
            .transfer_buffer = vertex_transfer_buffer,
            .offset = 0,
        },
        &(SDL_GPUBufferRegion){
            .buffer = vertex_buffer,
            .offset = 0,
            .size = sizeof(material_vertex_t) * 3,
        },
        false);

    SDL_UploadToGPUTexture(
        copy_pass,
        &(SDL_GPUTextureTransferInfo){
            .transfer_buffer = texture_transfer_buffer,
            .offset = 0,
        },
        &(SDL_GPUTextureRegion){
            .texture = default_material_texture,
            .w = default_material_surface->w,
            .h = default_material_surface->h,
            .d = 1,
        },
        false);

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(upload_cmd_buf);
    SDL_ReleaseGPUTransferBuffer(device, vertex_transfer_buffer);

    // ------------

    camera_t camera = make_camera(glms_vec2_make((float[]){ 0.0f, 0.0f }), glms_vec2_make((float[]){ 1920.0f, 1080.0f }));
    mat4s projection = get_camera_projection_matrix(&camera);
    mat4s view = get_camera_view_matrix(&camera);

    mat4s model = glms_mat4_mulN(
        (mat4s *[]){
            glms_scale_make(glms_vec3_make((float[]){ 1.0f, 1.0f, 0.0f })).raw,
            glms_rotate_make(0.0f, glms_vec3_make((float[]){ 0.0f, 0.0f, 1.0f })).raw,
            glms_translate_make(glms_vec3_make((float[]){ 0.0f, 0.0f, 0.0f })).raw,
        },
        3);

    mat4s mvp = glms_mat4_mulN((mat4s *[]){ projection.raw, view.raw, model.raw }, 3);

    uniform_t uniform = { .mvp = mvp };

    while (run_window_event_loop()) {
        if (close_window_requested()) {
            exit_window_event_loop();
            continue;
        }

        if (window_was_resized()) {
            get_window_size(&window_width, &window_height);
            // todo: resize render targets
        }

        SDL_GPUCommandBuffer *cmd_buf = SDL_AcquireGPUCommandBuffer(device);
        if (cmd_buf == NULL) {
            log_error(LOG_CATEGORY_GPU, "AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            exit_window_event_loop();
            continue;
        }

        if (render_target != NULL) {
            SDL_GPUColorTargetInfo colorTargetInfo = {
                .texture = render_target,
                .clear_color = (SDL_FColor){ 0.3f, 0.9f, 0.3f, 1.0f },
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
            };

            SDL_GPURenderPass *rpass = SDL_BeginGPURenderPass(cmd_buf, &colorTargetInfo, 1, NULL);

            SDL_BindGPUGraphicsPipeline(rpass, material_pipeline);
            SDL_BindGPUVertexBuffers(rpass, 0, &(SDL_GPUBufferBinding){ .buffer = vertex_buffer, .offset = 0 }, 1);
            SDL_PushGPUVertexUniformData(cmd_buf, 0, &uniform, sizeof(uniform_t));
            SDL_BindGPUFragmentSamplers(rpass, 0, &(SDL_GPUTextureSamplerBinding){ .texture = default_material_texture, .sampler = sampler }, 1);
            SDL_DrawGPUPrimitives(rpass, 3, 1, 0, 0);

            SDL_EndGPURenderPass(rpass);
        }

        SDL_GPUTexture *swapchain_texture;
        uint32_t swapchain_width;
        uint32_t swapchain_height;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buf, window_handle(), &swapchain_texture, &swapchain_width, &swapchain_height)) {
            log_error(LOG_CATEGORY_GPU, "WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
            exit_window_event_loop();
            continue;
        }

        if (swapchain_width != window_width || swapchain_height != window_height) {
            log_warn(LOG_CATEGORY_GPU, "Swapchain size (%d x %d) differs from window size (%d x %d).", swapchain_width, swapchain_height, window_width, window_height);
        }

        if (swapchain_texture != NULL) {
            SDL_GPUColorTargetInfo colorTargetInfo = {
                .texture = swapchain_texture,
                .clear_color = (SDL_FColor){ 0.3f, 0.3f, 0.9f, 1.0f },
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
            };

            SDL_GPURenderPass *rpass = SDL_BeginGPURenderPass(cmd_buf, &colorTargetInfo, 1, NULL);

            SDL_BindGPUGraphicsPipeline(rpass, swapchain_pipeline);
            SDL_BindGPUFragmentSamplers(rpass, 0, &(SDL_GPUTextureSamplerBinding){ .texture = render_target, .sampler = sampler }, 1);
            SDL_DrawGPUPrimitives(rpass, 3, 1, 0, 0);

            SDL_EndGPURenderPass(rpass);
        }

        SDL_SubmitGPUCommandBuffer(cmd_buf);
    }

    SDL_ReleaseGPUGraphicsPipeline(device, material_pipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, swapchain_pipeline);
    SDL_ReleaseGPUBuffer(device, vertex_buffer);

    SDL_ReleaseWindowFromGPUDevice(device, window_handle());
    SDL_DestroyGPUDevice(device);
    destroy_window();
    stop_application();

    return 0;
}
