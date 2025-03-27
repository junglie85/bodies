#include <SDL3/SDL.h>

#include "application.h"
#include "error.h"
#include "log.h"
#include "window.h"

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

    while (run_window_event_loop()) {
        if (close_window_requested()) {
            exit_window_event_loop();
            continue;
        }

        SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(device);
        if (cmdbuf == NULL)
        {
            log_error(LOG_CATEGORY_GPU, "AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            exit_window_event_loop();
            continue;
        }

        SDL_GPUTexture* swapchain_texture;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, window_handle(), &swapchain_texture, NULL, NULL)) {
            log_error(LOG_CATEGORY_GPU, "WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
            exit_window_event_loop();
            continue;
        }

        if (swapchain_texture != NULL)
        {
            SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
            colorTargetInfo.texture = swapchain_texture;
            colorTargetInfo.clear_color = (SDL_FColor){ 0.3f, 0.4f, 0.5f, 1.0f };
            colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
            colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

            SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
            SDL_EndGPURenderPass(renderPass);
        }

        SDL_SubmitGPUCommandBuffer(cmdbuf);
    }

    SDL_ReleaseWindowFromGPUDevice(device, window_handle());
    SDL_DestroyGPUDevice(device);
    destroy_window();
    stop_application();

    return 0;
}
