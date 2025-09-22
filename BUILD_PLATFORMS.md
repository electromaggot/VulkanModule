# Multi-Platform Build Guide for VulkanModule

This guide provides detailed instructions for building VulkanModule on different platforms.

## Supported Platforms

- **macOS** (Intel/Apple Silicon) with MoltenVK
- **Windows** (x64) with native Vulkan support
- **Linux** (x64/ARM64) with native Vulkan support
- **Raspberry Pi 5** (ARM64) with native Vulkan support

## Platform-Specific Setup

### macOS

**Requirements:**
- macOS 10.15+ (Catalina)
- Xcode Command Line Tools
- Homebrew package manager

**Installation:**
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install dependencies via Homebrew
brew install cmake vulkan-headers vulkan-loader molten-vk
brew install sdl2 sdl2_image glm pkg-config
```

**Build:**
```bash
cd TestHarness
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

**Notes:**
- Uses MoltenVK for Vulkan-to-Metal translation
- Automatically enables portability extensions for MoltenVK compatibility
- Debug validation layers may not be available on some macOS versions

---

### Windows

**Requirements:**
- Windows 10/11 (x64)
- Visual Studio 2019/2022 or Build Tools
- Vulkan SDK from LunarG

**Installation:**
1. **Install Visual Studio Community 2022** with C++ development tools
2. **Install Vulkan SDK:**
   - Download from https://vulkan.lunarg.com/
   - Install to default location (usually `C:\VulkanSDK\<version>`)
3. **Install vcpkg for dependencies:**
   ```cmd
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install

   # Install dependencies
   .\vcpkg install sdl2:x64-windows sdl2-image:x64-windows glm:x64-windows
   ```

**Build (Command Line):**
```cmd
cd TestHarness
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release -j %NUMBER_OF_PROCESSORS%
```

**Build (Visual Studio):**
```cmd
cd TestHarness
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
# Open VulkanModule.sln in Visual Studio and build
```

**Notes:**
- Executable will be in `build/Release/VulkanTester.exe`
- Requires VULKAN_SDK environment variable (set by Vulkan SDK installer)
- Uses Win32 platform extensions for native Vulkan support

---

### Linux (Ubuntu/Debian)

**Requirements:**
- Ubuntu 20.04+ or equivalent
- GCC 9+ or Clang 10+
- Vulkan drivers for your GPU

**Installation:**
```bash
# Update package manager
sudo apt update

# Install build tools
sudo apt install build-essential cmake pkg-config git

# Install Vulkan development packages
sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools

# Install dependencies
sudo apt install libsdl2-dev libsdl2-image-dev libglm-dev

# For NVIDIA users (optional)
sudo apt install nvidia-driver-535 vulkan-utils
```

**Build:**
```bash
cd TestHarness
mkdir build && cd build
cmake ..
make -j$(nproc)
```

**GPU Driver Notes:**
- **NVIDIA:** Install proprietary drivers (nvidia-driver-XXX)
- **AMD:** Install mesa-vulkan-drivers or amdvlk-drivers
- **Intel:** Install intel-media-va-driver-non-free mesa-vulkan-drivers

**Verification:**
```bash
# Test Vulkan installation
vulkaninfo --summary
vkcube  # Should display a spinning cube
```

---

### Raspberry Pi 5 (ARM64)

**Requirements:**
- Raspberry Pi OS 64-bit (Bookworm or newer)
- 8GB RAM recommended for compilation
- Updated GPU firmware with Vulkan support

**Installation:**
```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install build tools
sudo apt install build-essential cmake pkg-config git

# Install Vulkan packages
sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev

# Install dependencies
sudo apt install libsdl2-dev libsdl2-image-dev libglm-dev

# ARM64 cross-compilation tools (if needed)
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

**Enable Vulkan on Pi 5:**
```bash
# Add to /boot/config.txt
echo "dtoverlay=vc4-kms-v3d-pi5" | sudo tee -a /boot/config.txt
echo "gpu_mem=128" | sudo tee -a /boot/config.txt

# Reboot to apply changes
sudo reboot
```

**Build:**
```bash
cd TestHarness
mkdir build && cd build

# Native ARM64 build
cmake ..
make -j$(nproc)

# Or with explicit ARM64 optimizations
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

**Performance Notes:**
- ARM64 Cortex-A76 optimizations automatically enabled
- Vulkan 1.2+ supported on Pi 5 with recent firmware
- Consider using `-j2` or `-j3` for compilation if RAM is limited

**Verification:**
```bash
# Check Vulkan support
vulkaninfo --summary
# Should show "V3D 7.1.x" or similar VideoCore driver
```

---

## Troubleshooting

### Common Issues

**macOS: "VK_ERROR_INCOMPATIBLE_DRIVER"**
```bash
# Reinstall MoltenVK
brew reinstall molten-vk
```

**Windows: "vulkan-1.dll not found"**
- Ensure Vulkan SDK is installed
- Check PATH includes `%VULKAN_SDK%/bin`

**Linux: "libvulkan.so.1 not found"**
```bash
sudo apt install vulkan-loader
```

**Raspberry Pi: "No Vulkan devices found"**
```bash
# Update firmware
sudo rpi-update
# Ensure vc4-kms-v3d-pi5 overlay is enabled
```

### Build Debugging

**Enable verbose CMake output:**
```bash
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
```

**Debug library linking:**
```bash
ldd ./build/VulkanTester  # Linux/macOS
objdump -p ./build/VulkanTester.exe  # Windows
```

**Check Vulkan validation layers:**
```bash
export VK_LAYER_PATH=$VULKAN_SDK/etc/vulkan/explicit_layer.d
export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation
./VulkanTester
```

## Cross-Compilation

The CMakeLists.txt supports cross-compilation with appropriate toolchain files:

**Windows from Linux:**
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=mingw-w64-toolchain.cmake
```

**ARM64 from x64:**
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=aarch64-linux-gnu-toolchain.cmake
```

## Platform-Specific Features

| Platform | Vulkan Implementation | Platform Extensions | Validation Layers |
|----------|----------------------|---------------------|-------------------|
| macOS | MoltenVK (Vulkan→Metal) | VK_KHR_portability_enumeration | Limited |
| Windows | Native | VK_KHR_win32_surface | Full support |
| Linux | Native | VK_KHR_xcb_surface | Full support |
| Raspberry Pi | V3D driver | VK_KHR_xcb_surface | Full support |

The build system automatically detects the platform and configures appropriate extensions and optimizations.