#include "render-util.h"
#include "logger.h"
#include "reader.h"
#include "renderer.h"
#include "window.h"
#include <map>
#include <set>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace jet::ru
{

	SwapChainSupportDetails vQuerySwapChainSupport(const Renderer& renderer)
	{
		const vk::PhysicalDevice& device = renderer.GetPhysicalDevice();
		const vk::SurfaceKHR& surface = renderer.GetSurfaceKHR();

		SwapChainSupportDetails details;

		details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
		details.formats = device.getSurfaceFormatsKHR(surface);
		details.presentModes = device.getSurfacePresentModesKHR(surface);

		return details;
	}

	bool vIsDeviceSuitable(const Renderer& renderer)
	{
		const vk::PhysicalDevice& device = renderer.GetPhysicalDevice();
		const vk::SurfaceKHR& surface = renderer.GetSurfaceKHR();

		vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
		vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

		bool isCorrectDeviceType =
			deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && deviceFeatures.geometryShader;

		QueueFamilyIndices indices = vFindQueueFamilies(device, surface);

		bool extensionsSupported = vCheckDeviceExtensionSupport(renderer);

		if (!indices.isComplete() || !extensionsSupported)
		{
			return false;
		}

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails supportDetails = vQuerySwapChainSupport(renderer);
			swapChainAdequate = !supportDetails.formats.empty() && !supportDetails.presentModes.empty();
		}

		return isCorrectDeviceType && indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	void vPickPhysicalDevice(vk::Instance& instance, vk::PhysicalDevice& physicalDevice)
	{
		std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

		if (devices.size() == 0)
		{
			throw std::runtime_error("Failed to find GPUs with Vulkan support...Time to upgrade your hardware! :D");
		}

		std::multimap<i32, vk::PhysicalDevice> candidates;

		for (const vk::PhysicalDevice& device : devices)
		{
			i32 score = vRateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}

		if (candidates.rbegin()->first > 0)
		{
			physicalDevice = candidates.rbegin()->second;
			Logger::Info("Selected GPU: {}", physicalDevice.getProperties().deviceName.data());
		}
		else
		{
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

	i32 vRateDeviceSuitability(const vk::PhysicalDevice& device)
	{
		vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
		vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

		i32 score = 0;

		if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		{
			score += 1000;
		}

		score += deviceProperties.limits.maxImageDimension2D;

		if (!deviceFeatures.geometryShader)
		{
			return 0;
		}

		return score;
	}

	QueueFamilyIndices vFindQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface)
	{
		QueueFamilyIndices indices;

		std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

		u32 i = 0;
		for (const vk::QueueFamilyProperties& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				indices.graphicsFamily = i;
			}

			if (device.getSurfaceSupportKHR(i, surface))
			{
				indices.presentFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	std::vector<const char*> vGetDeviceExtensions()
	{
		return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	}

	bool vCheckDeviceExtensionSupport(const Renderer& renderer)
	{
		std::vector<vk::ExtensionProperties> availableExtensions =
			renderer.GetPhysicalDevice().enumerateDeviceExtensionProperties();

		const std::vector<const char*>& extensions = vGetDeviceExtensions();

		std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

		for (const vk::ExtensionProperties& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	vk::SurfaceFormatKHR vChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
	{
		for (const vk::SurfaceFormatKHR& format : availableFormats)
		{
			if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return format;
			}
		}
		return availableFormats[0];
	}

	vk::PresentModeKHR vChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
	{
		for (const vk::PresentModeKHR& mode : availablePresentModes)
		{
			if (mode == vk::PresentModeKHR::eMailbox)
			{
				return mode;
			}
		}

		return availablePresentModes[0];
	}

	vk::Extent2D vChooseSwapExtent(const Renderer& renderer, const vk::SurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(renderer.GetWindow().GetGlfwWindow(), &width, &height);

			VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			actualExtent.width =
				std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height =
				std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	vk::ShaderModule CreateShaderModule(const vk::Device& device, const std::string& shaderName)
	{
		std::vector<char> code = Reader::ReadShader(shaderName);

		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.setCodeSize(code.size());
		createInfo.setPCode(reinterpret_cast<const u32*>(code.data()));

		vk::ShaderModule module = device.createShaderModule(createInfo);

		if (!module)
		{
			throw std::runtime_error("Failed to create shader module: " + shaderName);
		}

		return module;
	}

	void vCreateBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
	{
		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.setSize(size);
		bufferInfo.setUsage(usage);
		bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

		buffer = device.createBuffer(bufferInfo);

		vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(buffer);

		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.setAllocationSize(memRequirements.size);
		allocInfo.setMemoryTypeIndex(vFindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties));

		bufferMemory = device.allocateMemory(allocInfo);
		device.bindBufferMemory(buffer, bufferMemory, 0);
	}

	void vCopyBuffer(const vk::Device& device, const vk::CommandPool& commandPool, const vk::Queue& graphicsQueue, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, vk::DeviceSize size)
	{
		vk::CommandBufferAllocateInfo allocateInfo{};
		allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
		allocateInfo.setCommandPool(commandPool);
		allocateInfo.setCommandBufferCount(1);

		vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocateInfo)[0];

		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		commandBuffer.begin(beginInfo);

		vk::BufferCopy copyRegion{};
		copyRegion.setSrcOffset(0).setDstOffset(0).setSize(size);
		commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

		commandBuffer.end();

		vk::SubmitInfo submitInfo{};
		submitInfo.setCommandBufferCount(1);
		submitInfo.setPCommandBuffers(&commandBuffer);

		graphicsQueue.submit(submitInfo, nullptr);
		graphicsQueue.waitIdle();

		device.freeCommandBuffers(commandPool, commandBuffer);
	}

	u32 vFindMemoryType(const vk::PhysicalDevice& physicalDevice, u32 typeFilter, vk::MemoryPropertyFlags properties)
	{
		vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

		for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		return 0;
	}



} // namespace jet::ru
