//
// DearImGui.cpp
//	General App Chassis, Platform Layer, GUI System, Vulkan-centric
//
// See header for overview.
//
// Created 3/22/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "DearImGui.h"
#include "FileSystem.h"
#include "CommandObjects.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl2.h"

extern void MainGUI(DearImGui&);


static void check_vk_result(VkResult err)
{
	if (err == 0) return;
	printf("VkResult %d\n", err);
	if (err < 0)
		abort();
}

static VkAllocationCallbacks*   g_Allocator = nullptr;


DearImGui::DearImGui(VulkanSetup& vulkan, iPlatform& platform)
	:	device(vulkan.device.getLogical())
	  , platform(platform)
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

	// Setup Dear ImGui context.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void) io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;		// Enable Gamepad Controls

	// Setup Dear ImGui style.
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings.
	platform.InitGUISystem();	// (e.g. for SDL, should in turn call ImGui_ImplSDL2_InitForVulkan(window);)

	// Prep ImGui docking (see Note A at bottom).
	ImGui_ImplVulkan_PipelineInfo pipelineInfo = {};
	pipelineInfo.RenderPass = vulkan.renderPass.getVkRenderPass();
	pipelineInfo.Subpass = 0;
	pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_InitInfo init_info = {
		.ApiVersion		= VK_API_VERSION_1_0,
		.Instance		= vulkan.vulkan.getVkInstance(),
		.PhysicalDevice	= vulkan.device.getGPU(),
		.Device			= device,
		.QueueFamily	= vulkan.device.Queues.getFamilyIndex(),
		.Queue			= vulkan.device.Queues.getCurrent(),
		.DescriptorPool	= createDescriptorPool(device),
		.DescriptorPoolSize = 0,  // Using pre-created descriptor pool.
		.MinImageCount	= 2,
		.ImageCount		= guiWindow.ImageCount,
		.PipelineCache	= VK_NULL_HANDLE,
		.PipelineInfoMain = pipelineInfo,
		.UseDynamicRendering = false,
		.Allocator		= NULL,
		.CheckVkResultFn = check_vk_result,
		.MinAllocationSize = 0
	};
	ImGui_ImplVulkan_Init(&init_info);

	/// Although we set up Vulkan ourselves, Dear ImGui assigns its internal Vulkan-related operational variables here.
	///  ...including things like its own shader modules and pipeline, which we must let it have independently
	///  because it operates with its own render parameters, like 2D projection.

	iniFileName = FileSystem::AppLocalStorageDirectory() + io.IniFilename;
	io.IniFilename = iniFileName.c_str();
}

// Copy constructor, useful e.g. for newConcretion() add to renderables.
//
DearImGui::DearImGui(const DearImGui& other)
	:	device(other.device)
	  , iniFileName(other.iniFileName)
	  , platform(other.platform)
{
	// Copy constructor for renderables system - shares the same ImGui context.
	// Note: This is a shallow copy that shares the ImGui context, not a deep copy.
}

DearImGui::~DearImGui()					// (CommandBuffer should get destroyed when commandPool does.)
{
	err = vkDeviceWaitIdle(device);		// Cleanup
	check_vk_result(err);

	ImGui_ImplVulkan_Shutdown();		// Free/destroy internals (like font/descriptor/pipeline
	ImGui_ImplSDL2_Shutdown();			//	or SDL clipboard/mouseCursors) that ImGui allocated.

	ImGui::DestroyContext();
}


bool DearImGui::Update(GameClock& time)
{		// (Don't really need time or deltaSeconds, as Dear ImGui tracks its own time.)
	preRender(MainGUI, platform);
	return true;
}

// Start the Dear ImGui frame ...and Rendering.
//
void DearImGui::preRender(void (*pfnLayOutGui)(DearImGui&), iPlatform& platform)
{
	ImGui_ImplVulkan_NewFrame();
	platform.GUISystemNewFrame();	// (e.g. for SDL, should in turn call ImGui_ImplSDL2_NewFrame(window);)

	ImGui::NewFrame();

	pfnLayOutGui(*this);

	ImGui::Render();
}

// Record Imgui Draw Data and draw funcs into command buffer.
// ImGui renders LAST within the same render pass as the scene, after all
//	batched renderables, ensuring ImGui overlays correctly on top of the 3D scene.
//
void DearImGui::IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex)
{
	auto drawData = ImGui::GetDrawData();
	if (drawData)
		ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
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
	const int SIZE = 1000;	// arbitrary default pool size

	VkDescriptorPool descriptorPool;
	VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER,				 SIZE },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SIZE },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,			 SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,			 SIZE },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,	 SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,	 SIZE },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,		 SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,		 SIZE },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, SIZE },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,		 SIZE }
	};
	VkDescriptorPoolCreateInfo pool_info = {
		.sType	 = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags	 = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = SIZE * IM_ARRAYSIZE(pool_sizes),
		.poolSizeCount	= (uint32_t) IM_ARRAYSIZE(pool_sizes),
		.pPoolSizes		= pool_sizes
	};
	VkResult err = vkCreateDescriptorPool(device, &pool_info, g_Allocator, &descriptorPool);
	check_vk_result(err);
	return descriptorPool;
}


/* Note: Font upload is now automatic in ImGui docking branch (2025+)
// Fonts are automatically uploaded on first NewFrame() call.
// This function is no longer needed and has been commented out.
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

	bool fontsCreated = ImGui_ImplVulkan_CreateFontsTexture();	// Note that ImGui 1.90+ changed API:
	if (! fontsCreated)									//	^ CreateFontsTexture no longer takes command buffer.
		printf("Failed to create ImGui fonts texture\n");

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

	ImGui_ImplVulkan_DestroyFontsTexture();						// Note: ImGui 1.90+ renamed this function.
}*/


/* DEV NOTE - Specifics for ImGui docking branch API (2025+)

ImVector fields removed in newer branch:
	guiWindow.Frames		  = NULL;
	guiWindow.FrameSemaphores = NULL;
 
Note A - Updated initialization:
	- RenderPass and MSAASamples moved into PipelineInfoMain.
	- Init takes only one parameter now (no longer includes render pass parameter).
	- Font upload is now automatic in newer ImGui (happens on first NewFrame() call).
	  (The old uploadFonts() method is no longer needed (commented-out above.)
*/
