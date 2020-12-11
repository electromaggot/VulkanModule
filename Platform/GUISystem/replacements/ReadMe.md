Dear ImGui Drop-in Replacement Files
------------------------------------

As of: **dear imgui, v1.80 WIP**  
these Vulkan-specific implementation files provide the following additional capability:

	//  [X] Renderer: User texture binding. Changes of ImTextureID ARE supported by this binding! See https://github.com/ocornut/imgui/pull/914
	     ↑                                                         ↑↑↑
	     These files insert the X here - and change from "aren't" this word.

That is, under Vulkan, `ImGui::Image(...)`, `ImGui::ImageButton(...)`, etc. didn't actually work.  All they did display would be the
currently loaded font atlas, which was the first (and only) image loaded.  So changing `ImTextureID` had no effect (despite what it says
here [https://github.com/ocornut/imgui/issues/1848](https://github.com/ocornut/imgui/issues/1848) and here 
[https://github.com/ocornut/imgui/commit/948009a8b2e98ef35fb8ddfe19299535a16c0834](https://github.com/ocornut/imgui/commit/948009a8b2e98ef35fb8ddfe19299535a16c0834)).

These files, when subsituted into your "imgui" tree, change that operation by providing this functionality.  You may notice one anomaly, however: the image loads and appears correctly at full-size,
but when scaled-down using ImGui, it "fades to red" the smaller it gets.  This seems to be due to ImGui using mipmapping, but your "user texture"
is not, so your image blends into a non-existent lower-resolution version of itself.  Fix this by setting `TextureSpec.filterMode = MIPMAP`, as seen below, for this module's code to enable mipmapping and generate those additional lower-resolution textures for you.

> Vulkan: switching between images for rendering #914  
> https://github.com/ocornut/imgui/pull/914

The changes folded into these most-recent files originated from (and full credit goes to) Marcell Kiss or [https://github.com/martty](https://github.com/martty)
[https://github.com/martty/imgui/commit/f1f948bea715754ad5e83d4dd9f928aecb4ed1d3](https://github.com/martty/imgui/commit/f1f948bea715754ad5e83d4dd9f928aecb4ed1d3)

Unlike his implementation, however, I use `VkDescriptorImageInfo` to pass `VkSampler`, `VkImageView`, and `VkImageLayout` all at once.
So usage in this VulkanModule looks like this:

    TextureSpec texspec;
    texspec.filename = "image.png";   // ← your filename of course goes here
    texspec.filterMode = MIPMAP;
    TextureImage* pImage = new TextureImage(texspec, vulkan.command.vkPool(), vulkan.device, platform);
    ImTextureID userTextureId = ImGui_ImplVulkan_AddTexture(pImage->getDescriptorImageInfo());

Or:

    VkDescriptorImageInfo descriptorImageInfo(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    ImTextureId userTextureId = ImGui_ImplVulkan_AddTexture(descriptorImageInfo);

Or just:

    ImTextureId userTextureId = ImGui_ImplVulkan_AddTexture( { sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL } );
