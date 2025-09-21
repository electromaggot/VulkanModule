**VulkanModule** provides a foundation upon which to build a Vulkan graphics project or game.

Vulkan itself requires a large amount of code and data structures to initialize and configure before any graphics can be displayed.  VulkanModule answers this need with two core parts:

1. **VulkanModule/Objects** encapsulate the various subsystems in an object-oriented fashion.  For example, the **`VulkanInstance`** class serves `VkInstance`, while **`GraphicsDevice`** administers `VkDevice` or `VkPhysicalDevice`, and so on.  Reasonable assumptions can be made from the names of 
the classes about their purpose, but if still in question, further detail is provided below.</br>
These classes not only focus on Creation and clean Destruction, but also reuse those to **`Recreate()`**, which is essential if the window resizes/minimizes/drags-across-displays, etc.  (The modules has other niceties too: like live resizing, where rendering continues during resize, or clean transitions upon phone rotation.)

2. **VulkanModule/Setup** combines the above Objects to set up the Vulkan system in an RAII fashion, the highlight being the **`VulkanSetup`** class to initialize all the base pieces in dependency order. 
This means first, progressively instantiating components that "won't change much" after they are constructed, such as:

   <table border="0">
     <tr>
       <td>
         <ol type="1">
           <li>vulkan instance</li>
           <li>window & surface</li>
           <li>physical & logical device</li>
           <li>swapchain</li>
         </ol>
       </td>
       <td>
         <ol  type="1" start="5">
           <li>depth buffer</li>
           <li>render pass</li>
           <li>frame buffers</li>
           <li>synchronization objects (like semaphores, fences)</li>
         </ol>
       </td>
     </tr>
   </table>

After the above core initialization, further Vulkan components incorporate to customize rendering and start defining the 3D Objects to be drawn:

3. **VulkanModule/Adjunct** includes these.  Some specify access to subsequent files, like to define model meshes or load textures.  Also here are various VertexTypes to fill a VertexBuffer, the format of which a corresponding Vertex Shader will expect.
