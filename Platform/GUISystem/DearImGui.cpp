//
// DearImGui.cpp
//	General App Chassis, Platform Layer, GUI System, Vulkan-centric
//
// Created 3/22/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "DearImGui.h"

#include "CommandObjects.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl.h"

extern void MainGUI(iPlatform&);


static void check_vk_result(VkResult err)
{
    if (err == 0) return;
    printf("VkResult %d\n", err);
    if (err < 0)
        abort();
}

static VkAllocationCallbacks*   g_Allocator = NULL;


DearImGui::DearImGui(VulkanSetup& vulkan, iPlatform& platform)
	:	platform(platform),
		device(vulkan.device.getLogical())
{
	ImGui_ImplVulkanH_Window guiWindow;
	guiWindow.Width			= platform.pixelsWide;
	guiWindow.Height		= platform.pixelsHigh;
	guiWindow.Swapchain		= vulkan.swapchain.getVkSwapchain();
	guiWindow.Surface		= vulkan.windowSurface.getVkSurface();
	guiWindow.SurfaceFormat = vulkan.device.getProfile().selectedSurfaceFormat;
	guiWindow.RenderPass	= vulkan.renderPass.getVkRenderPass();
	guiWindow.PresentMode	= VK_PRESENT_MODE_MAX_ENUM_KHR;
	guiWindow.ClearEnable	= false;	// <-- This keeps GUI from clearing screen before it draws!
	guiWindow.ClearValue	= VkClearValue();
	guiWindow.FrameIndex	= 0;
	guiWindow.ImageCount	= vulkan.swapchain.getNumImages();
	guiWindow.SemaphoreIndex = 0;
	guiWindow.Frames		 = NULL;
	guiWindow.FrameSemaphores = NULL;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
	(void) io;
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
		.QueueFamily	= vulkan.device.Queues.getFamilyIndex(),
		.Queue			= vulkan.device.Queues.getCurrent(),
		.PipelineCache	= VK_NULL_HANDLE,
		.DescriptorPool	= createDescriptorPool(device),
		.MinImageCount	= 2,	//g_MinImageCount,
		.ImageCount		= guiWindow.ImageCount,
		.MSAASamples	= VK_SAMPLE_COUNT_1_BIT,
		.Allocator		= NULL,
		.CheckVkResultFn = check_vk_result,
	};
    ImGui_ImplVulkan_Init(&init_info, vulkan.renderPass.getVkRenderPass());

	/// Although we set up Vulkan ourselves, Dear ImGui assigns its internal Vulkan-related operational variables here.
	///  ...including things like its own shader modules and pipeline, which we must let it have independently
	///  because it operates with its own render parameters, like 2D projection.

	// Get our own CommandBuffer from the existing Pool, for existing Queue:
	VkCommandPool commandPool	= vulkan.command.vkPool();
	VkQueue		  graphicsQueue	= vulkan.device.Queues.getCurrent();

	VkCommandBuffer commandBuffer = allocateCommandBuffer(commandPool, device);

	uploadFonts(commandPool, commandBuffer, graphicsQueue);
}

DearImGui::~DearImGui()					// (CommandBuffer should get destroyed when commandPool does.)
{
    err = vkDeviceWaitIdle(device);		// Cleanup
    check_vk_result(err);

    ImGui_ImplVulkan_Shutdown();		// Free/destroy internals (like font/descriptor/pipeline
    ImGui_ImplSDL2_Shutdown();			//	or SDL clipboard/mouseCursors) that ImGui allocated.

    ImGui::DestroyContext();
}


void DearImGui::Update(float deltaSeconds)
{		// (Don't really need deltaSeconds, as Dear ImGui seems to track its own time.)
	preRender(MainGUI, platform);
}

// Start the Dear ImGui frame ...and Rendering.
//
void DearImGui::preRender(void (*pfnLayOutGui)(iPlatform&), iPlatform& platform)
{
	ImGui_ImplVulkan_NewFrame();
	platform.GUISystemNewFrame();	// (e.g. for SDL, should in turn call ImGui_ImplSDL2_NewFrame(window);)

	ImGui::NewFrame();

	pfnLayOutGui(platform);

	ImGui::Render();
}

// Record Imgui Draw Data and draw funcs into command buffer.
//
void DearImGui::IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
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

VkCommandBuffer DearImGui::allocateCommandBuffer(VkCommandPool commandPool, VkDevice device)
{
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

VkDescriptorPool DearImGui::createDescriptorPool(VkDevice device)
{
	VkDescriptorPool descriptorPool;
	VkDescriptorPoolSize pool_sizes[] = {
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
