//
// DearImGui.cpp
//	General App Chassis, Platform Layer, GUI System, Vulkan-centric
//
// Created 3/22/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "DearImGui.h"

//#include "imgui.h"
//#include "imgui_impl_vulkan.h"


static void check_vk_result(VkResult err)
{
    if (err == 0) return;
    printf("VkResult %d\n", err);
    if (err < 0)
        abort();
}

static VkAllocationCallbacks*   g_Allocator = NULL;

VkCommandBuffer tempAllocateCommandBuffer(VkCommandPool commandPool, VkDevice device) {
	VkCommandBuffer commandBuffer;
	VkCommandBufferAllocateInfo allocInfo = {
		.sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext	= nullptr,
		.commandPool		= commandPool,
		.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	VkResult err = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
	check_vk_result(err);
	return commandBuffer;
}

VkDescriptorPool tempCreateDescriptorPool(VkDevice device)
{
	VkDescriptorPool descriptorPool;
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER,				 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,			 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,			 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,	 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,	 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,		 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,		 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,		 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes),
		.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes),
		.pPoolSizes = pool_sizes
	};
	VkResult err = vkCreateDescriptorPool(device, &pool_info, g_Allocator, &descriptorPool);
	check_vk_result(err);
	return descriptorPool;
}


DearImGui::DearImGui(VulkanSetup& vulkan, iPlatform& platform)
	//(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue, VkRenderPass renderPass)
	: device(vulkan.device.getLogical())
/*TEMPORARY?*/ , framebuffers(&vulkan.framebuffers.getVkFramebuffers()[0])
{
	guiWindow.Width			= platform.pixelsWide;
	guiWindow.Height		= platform.pixelsHigh;
	guiWindow.Swapchain		= vulkan.swapchain.getVkSwapchain();
	guiWindow.Surface		= vulkan.windowSurface.getVkSurface();
	guiWindow.SurfaceFormat = vulkan.device.getProfile().selectedSurfaceFormat;
	guiWindow.RenderPass	= vulkan.renderPass.getVkRenderPass();
	guiWindow.PresentMode	= VK_PRESENT_MODE_MAX_ENUM_KHR;
	guiWindow.ClearEnable	= true;
	guiWindow.ClearValue	= VkClearValue();
	guiWindow.FrameIndex	= 0;
	guiWindow.ImageCount	= vulkan.swapchain.getNumImages();
	guiWindow.SemaphoreIndex = 0;
	guiWindow.Frames		 = NULL;
	guiWindow.FrameSemaphores = NULL;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
	platform.InitGUISystem();	// (e.g. for SDL, should in turn call ImGui_ImplSDL2_InitForVulkan(window);)

    ImGui_ImplVulkan_InitInfo init_info = {
		.Instance		= vulkan.vulkan.getVkInstance(),
		.PhysicalDevice	= vulkan.device.getGPU(),
		.Device			= device,
		.QueueFamily	= vulkan.device.Queues.GraphicsIndex(),
		.Queue			= vulkan.device.Queues.Graphics(),
		.PipelineCache	= VK_NULL_HANDLE,
		.DescriptorPool	= tempCreateDescriptorPool(device),
		.Allocator		= NULL,
		.MinImageCount	= 2,	//g_MinImageCount,
		.ImageCount		= guiWindow.ImageCount,
		.CheckVkResultFn = check_vk_result,
	};
    ImGui_ImplVulkan_Init(&init_info, vulkan.renderPass.getVkRenderPass());
//TJ: although we set up Vulkan ourselves, ImGui assigns its internal Vulkan-related operational variables here.
//TJ:  including things like its own shader modules and pipeline, which we must let it have independently
//TJ:	because it operates with its own render parameters like 2D projection

	// Get our own CommandBuffer from the existing Pool, for existing Queue:
	commandPool	  = vulkan.commandPool.getVkInstance();
	commandBuffer = tempAllocateCommandBuffer(commandPool, device);
	graphicsQueue = vulkan.device.Queues.Graphics();

	uploadFonts(commandPool, commandBuffer, graphicsQueue);
}

DearImGui::~DearImGui() {			// (CommandBuffer should get destroyed when commandPool does.)

    // Cleanup
    err = vkDeviceWaitIdle(device);
    check_vk_result(err);

    ImGui_ImplVulkan_Shutdown();	// Free/destroy internals (like font/descriptor/pipeline
    ImGui_ImplSDL2_Shutdown();		//	or SDL clipboard/mouseCursors) that ImGui allocated.

    ImGui::DestroyContext();
}

// Upload fonts using any command queue... but watch out, it'll kill one that's already set-up.
//
void DearImGui::uploadFonts(VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue)
{
    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

	err = vkResetCommandPool(device, commandPool, 0);
	check_vk_result(err);

	VkCommandBufferBeginInfo beginfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	err = vkBeginCommandBuffer(commandBuffer, &beginfo);
	check_vk_result(err);

	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

	VkSubmitInfo endInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer
	};
	err = vkEndCommandBuffer(commandBuffer);
	check_vk_result(err);

	err = vkQueueSubmit(queue, 1, &endInfo, VK_NULL_HANDLE);
	check_vk_result(err);

	err = vkDeviceWaitIdle(device);
	check_vk_result(err);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}


void DearImGui::Update() {

}

void LoopStuff()
{
    // Main loop
//    bool done = false;
//    while (!done)
//    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.


/*TJ PROBABLY DO WANT THIS IN MY MAIN LOOP FOR WINDOW RESIZES  and to pass them to IMGUI
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
//TJ: AND OF COURSE I NEED THIS IN MY MAIN LOOP:
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(window))
            {
                g_SwapChainResizeWidth = (int)event.window.data1;
                g_SwapChainResizeHeight = (int)event.window.data2;
                g_SwapChainRebuild = true;
            }
        }

        if (g_SwapChainRebuild)
        {
            g_SwapChainRebuild = false;
            ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
            ImGui_ImplVulkanH_CreateWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData, g_QueueFamily, g_Allocator, g_SwapChainResizeWidth, g_SwapChainResizeHeight, g_MinImageCount);
            g_MainWindowData.FrameIndex = 0;
        }
	}*/
}

void DearImGui::PreRender(void (*pfnLayOutGui)(), iPlatform& platform)
{
	extern ImVec4 clear_color;

	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	platform.GUISystemNewFrame();	// (e.g. for SDL, should in turn call ImGui_ImplSDL2_NewFrame(window);)

	ImGui::NewFrame();

	pfnLayOutGui();

	// Rendering
	ImGui::Render();

	// ...although remaining render operations completed by caller, like actually
	// submitting the CommandBuffer pooled above to the ... UH WAIT NO

/*TJ: Let's assume my Render/Present can already incorporate all of this:*/
	memcpy(&guiWindow.ClearValue.color.float32[0], &clear_color, 4 * sizeof(float));
	FrameRender();
//	FramePresent();
/**/
}

void DearImGui::FrameRender()
{
    VkResult err;

	/*TJ_ALREADY HAVE ALL THIS
    VkSemaphore image_acquired_semaphore  = guiWindow.FrameSemaphores[guiWindow.SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = guiWindow.FrameSemaphores[guiWindow.SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(device, guiWindow.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &guiWindow.FrameIndex);
    check_vk_result(err);

    ImGui_ImplVulkanH_Frame* fd = &guiWindow.Frames[guiWindow.FrameIndex];
    {
        err = vkWaitForFences(device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(device, 1, &fd->Fence);
        check_vk_result(err);
    }*/


    {
        //err = vkResetCommandPool(device, fd->CommandPool, 0); // <--- TJ: DON'T WANT THIS
        //check_vk_result(err);

        VkCommandBufferBeginInfo info = {
        	.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        	.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};
        err = vkBeginCommandBuffer(commandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {
        	.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        	.renderPass = guiWindow.RenderPass,
        	.framebuffer = framebuffers[0], // crashable if wrong index
        	.renderArea.extent.width = (uint32_t) guiWindow.Width,
        	.renderArea.extent.height = (uint32_t) guiWindow.Height,
        	.clearValueCount = 1,
        	.pClearValues = &guiWindow.ClearValue
		};
        vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record Imgui Draw Data and draw funcs into command buffer
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(commandBuffer);

	/*TJ_ALREADY HAVE ALL THIS
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {
        	.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        	.waitSemaphoreCount = 1,
        	.pWaitSemaphores = &image_acquired_semaphore,
        	.pWaitDstStageMask = &wait_stage,
        	.commandBufferCount = 1,
        	.pCommandBuffers = &fd->CommandBuffer,
        	.signalSemaphoreCount = 1,
        	.pSignalSemaphores = &render_complete_semaphore
		};*/
        err = vkEndCommandBuffer(commandBuffer); // <--- TJ: SEEM OUT OF PLACE!
        /*check_vk_result(err);
        err = vkQueueSubmit(graphicsQueue, 1, &info, fd->Fence);
        check_vk_result(err);
    }*/
}

//TJ: doesn't do anything mine already does
void DearImGui::FramePresent()
{
    VkSemaphore render_complete_semaphore = guiWindow.FrameSemaphores[guiWindow.SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &guiWindow.Swapchain;
    info.pImageIndices = &guiWindow.FrameIndex;
    VkResult err = vkQueuePresentKHR(graphicsQueue, &info);
    check_vk_result(err);
    guiWindow.SemaphoreIndex = (guiWindow.SemaphoreIndex + 1) % guiWindow.ImageCount; // Now we can use the next set of semaphores
}
