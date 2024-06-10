#include "renderer.h"

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "core.h"
#include "dynamic-array.h"
#include "logger.h"
#include "render-util.h"
#include "window.h"

#include <set>
#include <vulkan/vulkan_structs.hpp>

namespace jet
{

	const vk::PhysicalDevice& Renderer::GetPhysicalDevice() const
	{
		return mPhysicalDevice;
	}

	const vk::Device& Renderer::GetDevice() const
	{
		return mDevice;
	}
	const vk::SurfaceKHR& Renderer::GetSurfaceKHR() const
	{
		return mSurface;
	}

	const Window& Renderer::GetWindow() const
	{
		return *mWindow;
	}

	void Renderer::Init(Window* window)
	{
		CHECK_IN();

		mWindow = window;

		vCreateInstance();
		vSetupDebugMessenger();
		vCreateSurface();
		ru::vPickPhysicalDevice(mInstance, mPhysicalDevice);
		vCreateLogicalDevice();
		vCreateSwapchain();
		vCreateImageViews();
		vCreateRenderPass();
		vCreateGraphicsPipeline();
		vCreateFrameBuffers();
		vCreateCommandPool();
		vCreateCommandBuffers();
		vCreateSyncObjects();
	}

	void Renderer::vCreateInstance()
	{
		if (mEnableValidationLayers && !vCheckValidationLayerSupport())
		{
			throw std::runtime_error("Validation layers requested, but not available!");
		}

		vk::ApplicationInfo appInfo{};
		appInfo.sType = vk::StructureType::eApplicationInfo;
		appInfo.pApplicationName = "Jet";
		appInfo.applicationVersion = vk::makeApiVersion(0, 0, 0, 1);
		appInfo.pEngineName = "Jet";
		appInfo.engineVersion = vk::makeApiVersion(0, 0, 0, 1);
		appInfo.apiVersion = vk::ApiVersion13;

		vk::InstanceCreateInfo createInfo{};
		createInfo.sType = vk::StructureType::eInstanceCreateInfo;
		createInfo.pApplicationInfo = &appInfo;

		createInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;

		std::vector<const char*> requiredExtensions = vGetRequiredExtensions();

		createInfo.enabledExtensionCount = (u32)requiredExtensions.size();
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		createInfo.enabledLayerCount = 0;

		if (mEnableValidationLayers)
		{
			createInfo.enabledLayerCount = (u32)mValidationLayers.size();
			createInfo.ppEnabledLayerNames = mValidationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VK_TRY(vk::createInstance(&createInfo, nullptr, &mInstance), "Failed to create VkInstance");
	}

	std::vector<const char*> Renderer::vGetRequiredExtensions()
	{
		u32 glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

		if (mEnableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::vDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			Logger::Error("Validation Layer: {}", pCallbackData->pMessage);
		else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			Logger::Warning("Validation Layer: {}", pCallbackData->pMessage);
		else
			Logger::Info("Validation Layer: {}", pCallbackData->pMessage);

		return VK_FALSE;
	}

	void Renderer::vGetExtensions()
	{
		u32 extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		DynamicArray<VkExtensionProperties> extensions(extensionCount);

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.Data());

		Logger::Info("Available Extensions");

		for (const VkExtensionProperties& extension : extensions)
		{
			Logger::Message("{}", extension.extensionName);
		}
	}

	void Renderer::vSetValidationLayers()
	{
		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
	}

	bool Renderer::vCheckValidationLayerSupport()
	{
		u32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		DynamicArray<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.Data());

		Logger::Info("Available Validation Layers");

		for (const VkLayerProperties& layer : availableLayers)
		{
			Logger::Message("{}", layer.layerName);
		}

		for (const char* layerName : mValidationLayers)
		{
			bool layerFound = false;

			for (const VkLayerProperties& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				Logger::Error("Validation Layer {} not found", layerName);
				return false;
			}
		}

		return true;
	}

	void Renderer::vCreateSurface()
	{
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(mInstance, mWindow->GetGlfwWindow(), nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface!");
		}

		mSurface = surface;
	}

	void Renderer::vSetupDebugMessenger()
	{
		if (!mEnableValidationLayers)
			return;

		vk::DispatchLoaderDynamic dispatchLoader(mInstance, vkGetInstanceProcAddr);

		vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
		createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		createInfo.pfnUserCallback = vDebugCallback;

		vk::DebugUtilsMessengerEXT debugMessenger;
		VK_TRY(mInstance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &debugMessenger, dispatchLoader),
			"Failed to create Debug Messenger!");
	}

	void Renderer::vCreateLogicalDevice()
	{
		ru::QueueFamilyIndices indices = ru::vFindQueueFamilies(mPhysicalDevice, mSurface);

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

		std::set<u32> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		f32 queuePriority = 1.0f;

		for (u32 queueFamily : uniqueQueueFamilies)
		{
			vk::DeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		vk::PhysicalDeviceFeatures deviceFeatures{};
		vk::DeviceCreateInfo createInfo{};
		createInfo.queueCreateInfoCount = queueCreateInfos.size();
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		const std::vector<const char*>& deviceExtensions = ru::vGetDeviceExtensions();

		createInfo.enabledExtensionCount = (u32)deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (mEnableValidationLayers)
		{
			createInfo.enabledLayerCount = (u32)mValidationLayers.size();
			createInfo.ppEnabledLayerNames = mValidationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VK_TRY(mPhysicalDevice.createDevice(&createInfo, nullptr, &mDevice), "Failed to create logical device!");

		mGraphicsQueue = mDevice.getQueue(indices.graphicsFamily.value(), 0);
		mPresentQueue = mDevice.getQueue(indices.presentFamily.value(), 0);
	}

	void Renderer::vCreateSwapchain()
	{
		ru::SwapChainSupportDetails swapChainSupport = ru::vQuerySwapChainSupport(*this);

		vk::SurfaceFormatKHR surfaceFormat = ru::vChooseSwapSurfaceFormat(swapChainSupport.formats);
		vk::PresentModeKHR presentMode = ru::vChooseSwapPresentMode(swapChainSupport.presentModes);
		vk::Extent2D extent = ru::vChooseSwapExtent(*this, swapChainSupport.capabilities);

		u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo{};
		createInfo.surface = mSurface;

		mSwapchainImageFormat = surfaceFormat.format;
		mSwapchainExtent = extent;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		ru::QueueFamilyIndices indices = ru::vFindQueueFamilies(mPhysicalDevice, mSurface);

		u32 queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
			createInfo.queueFamilyIndexCount = 0;     // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = presentMode;
		createInfo.clipped = vk::True;
		createInfo.oldSwapchain = nullptr;

		mSwapchain = mDevice.createSwapchainKHR(createInfo);

		mSwapchainImages = mDevice.getSwapchainImagesKHR(mSwapchain);
	}

	void Renderer::vCreateImageViews()
	{
		mSwapchainImageViews.resize(mSwapchainImages.size());

		for (size_t i = 0; i < mSwapchainImages.size(); i++)
		{
			vk::ImageViewCreateInfo createInfo;
			createInfo.image = mSwapchainImages[i];
			createInfo.viewType = vk::ImageViewType::e2D;
			createInfo.format = mSwapchainImageFormat;
			createInfo.components.r = vk::ComponentSwizzle::eIdentity;
			createInfo.components.g = vk::ComponentSwizzle::eIdentity;
			createInfo.components.b = vk::ComponentSwizzle::eIdentity;
			createInfo.components.a = vk::ComponentSwizzle::eIdentity;
			createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			mSwapchainImageViews[i] = mDevice.createImageView(createInfo);
		}
	}

	void Renderer::vCreateGraphicsPipeline()
	{
		vk::ShaderModule vertModule = ru::CreateShaderModule(mDevice, "vert");
		vk::ShaderModule fragModule = ru::CreateShaderModule(mDevice, "frag");

		vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex);
		vertShaderStageInfo.setModule(vertModule);
		vertShaderStageInfo.setPName("main");

		vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment);
		fragShaderStageInfo.setModule(fragModule);
		fragShaderStageInfo.setPName("main");

		vk::PipelineShaderStageCreateInfo shaderStages[] = {
			vertShaderStageInfo,
			fragShaderStageInfo,
		};

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.setVertexBindingDescriptions(nullptr);
		vertexInputInfo.setVertexAttributeDescriptions(nullptr);

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);
		inputAssembly.setPrimitiveRestartEnable(vk::False);

		vk::Viewport viewport{};
		viewport.setX(0.0f);
		viewport.setY(0.0f);
		viewport.setWidth((f32)mSwapchainExtent.width);
		viewport.setHeight((f32)mSwapchainExtent.height);
		viewport.setMinDepth(0.0f);
		viewport.setMaxDepth(1.0f);

		vk::Rect2D scissor{};
		scissor.setOffset({ 0, 0 });
		scissor.setExtent(mSwapchainExtent);

		std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor,
		};

		vk::PipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.setDynamicStates(dynamicStates);

		vk::PipelineViewportStateCreateInfo viewportState{};
		viewportState.setViewports(viewport);
		viewportState.setScissors(scissor);

		vk::PipelineRasterizationStateCreateInfo rasterizer;
		rasterizer.setDepthClampEnable(vk::False);
		rasterizer.setRasterizerDiscardEnable(vk::False);
		rasterizer.setPolygonMode(vk::PolygonMode::eFill);
		rasterizer.setLineWidth(1.0f);
		rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
		rasterizer.setFrontFace(vk::FrontFace::eClockwise);

		rasterizer.setDepthBiasEnable(vk::False);
		rasterizer.setDepthBiasConstantFactor(0.0f);
		rasterizer.setDepthBiasClamp(0.0f);
		rasterizer.setDepthBiasSlopeFactor(0.0f);

		vk::PipelineMultisampleStateCreateInfo multisampling{};
		multisampling.setSampleShadingEnable(vk::False);
		multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
		multisampling.setMinSampleShading(1.0f);
		multisampling.setPSampleMask(nullptr);
		multisampling.setAlphaToCoverageEnable(vk::False);
		multisampling.setAlphaToOneEnable(vk::False);

		vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
		colorBlendAttachment.setBlendEnable(vk::False);
		colorBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eOne);
		colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
		/*colorBlendAttachment.setBlendEnable(vk::True);*/
		/*colorBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);*/
		/*colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);*/
		colorBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd);
		colorBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
		colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
		colorBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd);

		vk::PipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.setLogicOpEnable(vk::False);
		colorBlending.setLogicOp(vk::LogicOp::eCopy);
		colorBlending.setAttachments(colorBlendAttachment);
		colorBlending.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.setSetLayouts(nullptr);
		pipelineLayoutInfo.setPushConstantRanges(nullptr);

		mPipelineLayout = mDevice.createPipelineLayout(pipelineLayoutInfo);
		if (!mPipelineLayout)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}

		vk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.setStages(shaderStages)
			.setPVertexInputState(&vertexInputInfo)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportState)
			.setPMultisampleState(&multisampling)
			.setPDepthStencilState(nullptr)
			.setPColorBlendState(&colorBlending)
			.setPRasterizationState(&rasterizer)
			.setPDynamicState(&dynamicState)
			.setLayout(mPipelineLayout)
			.setRenderPass(mRenderPass)
			.setSubpass(0)
			.setBasePipelineHandle(VK_NULL_HANDLE)
			.setBasePipelineIndex(-1);

		auto result = mDevice.createGraphicsPipeline({}, pipelineInfo);

		if (result.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to create graphics pipelines!");
		}
		mGraphicsPipeline = result.value;

		mDevice.destroyShaderModule(vertModule);
		mDevice.destroyShaderModule(fragModule);
	}

	void Renderer::vCreateRenderPass()
	{
		vk::AttachmentDescription colorAttachment{};
		colorAttachment.setFormat(mSwapchainImageFormat);
		colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
		colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
		colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
		colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
		colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
		colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		vk::AttachmentReference colorAttachmentRef{};
		colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::SubpassDescription subpass{};
		subpass.setColorAttachments(colorAttachmentRef);

		vk::RenderPassCreateInfo renderPassInfo{};
		renderPassInfo.setAttachments(colorAttachment);
		renderPassInfo.setSubpasses(subpass);

		vk::SubpassDependency dependency;
		dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setSrcAccessMask(vk::AccessFlags(0))
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

		renderPassInfo.setDependencies(dependency);

		mRenderPass = mDevice.createRenderPass(renderPassInfo);
	}

	void Renderer::vCreateFrameBuffers()
	{
		mSwapChainFrameBuffers.resize(mSwapchainImageViews.size());

		for (size_t i = 0; i < mSwapchainImageViews.size(); i++)
		{
			VkImageView attachments[] = { mSwapchainImageViews[i] };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = mRenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = mSwapchainExtent.width;
			framebufferInfo.height = mSwapchainExtent.height;
			framebufferInfo.layers = 1;

			mSwapChainFrameBuffers[i] = mDevice.createFramebuffer(framebufferInfo);
		}
	}

	void Renderer::vCreateCommandPool()
	{
		ru::QueueFamilyIndices queueFamilyIndices = ru::vFindQueueFamilies(mPhysicalDevice, mSurface);

		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

		mCommandPool = mDevice.createCommandPool(poolInfo);
	}

	void Renderer::vCreateCommandBuffers()
	{
		mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.setCommandPool(mCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount((u32)mCommandBuffers.size());
		mCommandBuffers = mDevice.allocateCommandBuffers(allocInfo);
	}

	void Renderer::vRecordCommandBuffer(const vk::CommandBuffer& commandBuffer, u32 imageIndex)
	{
		vk::CommandBufferBeginInfo beginInfo;

		commandBuffer.begin(beginInfo);

		vk::RenderPassBeginInfo renderPassInfo;
		renderPassInfo.setRenderPass(mRenderPass)
			.setFramebuffer(mSwapChainFrameBuffers[imageIndex])
			.setRenderArea(vk::Rect2D({ 0, 0 }, mSwapchainExtent));

		vk::ClearValue clearColor({ 0.f, 0.f, 0.f, 1.f });
		renderPassInfo.setClearValues(clearColor);

		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mGraphicsPipeline);
		commandBuffer.setViewport(0, vk::Viewport()
			.setX(0.f)
			.setY(0.f)
			.setWidth((f32)mSwapchainExtent.width)
			.setHeight((f32)mSwapchainExtent.height)
			.setMinDepth(0.f)
			.setMaxDepth(1.f));
		commandBuffer.setScissor(0, vk::Rect2D().setOffset({ 0, 0 }).setExtent(mSwapchainExtent));
		commandBuffer.draw(3, 1, 0, 0);
		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	void Renderer::vCreateSyncObjects()
	{
		mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		vk::SemaphoreCreateInfo semaphoreInfo;
		vk::FenceCreateInfo fenceCreateInfo;
		fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			mImageAvailableSemaphores[i] = mDevice.createSemaphore(semaphoreInfo);
			mRenderFinishedSemaphores[i] = mDevice.createSemaphore(semaphoreInfo);
			mInFlightFences[i] = mDevice.createFence(fenceCreateInfo);
		}
	}

	void Renderer::vRecreateSwapchain()
	{
		i32 width = 0, height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(mWindow->GetGlfwWindow(), &width, &height);
			glfwWaitEvents();
		}

		mDevice.waitIdle();

		vCleanupSwapchain();

		vCreateSwapchain();
		vCreateImageViews();
		vCreateFrameBuffers();
	}

	void Renderer::vCleanupSwapchain()
	{
		for (size_t i = 0; i < mSwapChainFrameBuffers.size(); i++)
		{
			mDevice.destroyFramebuffer(mSwapChainFrameBuffers[i]);
		}

		for (size_t i = 0; i < mSwapchainImageViews.size(); i++)
		{
			mDevice.destroyImageView(mSwapchainImageViews[i]);
		}

		mDevice.destroySwapchainKHR(mSwapchain);
	}

	void Renderer::DrawFrame()
	{
		vk::Result result = mDevice.waitForFences(mInFlightFences[mCurrentFrame], vk::True, UINT64_MAX);

		vk::ResultValue resultVal = mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame]);

		if (resultVal.result == vk::Result::eErrorOutOfDateKHR)
		{
			vRecreateSwapchain();
			return;
		}
		else if (resultVal.result != vk::Result::eSuccess && resultVal.result != vk::Result::eSuboptimalKHR)
		{
			Logger::Error("Failed to acquire swap chain image!");
		}

		mDevice.resetFences(mInFlightFences[mCurrentFrame]);

		u32 imageIndex = resultVal.value;

		mCommandBuffers[mCurrentFrame].reset(vk::CommandBufferResetFlags(0));

		vRecordCommandBuffer(mCommandBuffers[mCurrentFrame], imageIndex);

		vk::SubmitInfo submitInfo;
		vk::PipelineStageFlags waitStages[]{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.setWaitSemaphores(mImageAvailableSemaphores[mCurrentFrame])
			.setWaitDstStageMask(waitStages)
			.setCommandBuffers(mCommandBuffers[mCurrentFrame])
			.setSignalSemaphores(mRenderFinishedSemaphores[mCurrentFrame]);

		mGraphicsQueue.submit(submitInfo, mInFlightFences[mCurrentFrame]);

		vk::PresentInfoKHR presentInfo;
		presentInfo.setWaitSemaphores(mRenderFinishedSemaphores[mCurrentFrame]);
		presentInfo.setSwapchains(mSwapchain).setImageIndices(imageIndex);

		try
		{
			result = mPresentQueue.presentKHR(presentInfo);
		}
		catch (const vk::OutOfDateKHRError& e)
		{
			if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || mWindow->GetFramebufferResized())
			{
				vRecreateSwapchain();
				mWindow->SetFramebufferResized(false);
			}
			else if (result != vk::Result::eSuccess)
			{
				Logger::Error("failed to present swap chain image!");
			}
		}

		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::Clean()
	{
		CHECK_IN();

		mDevice.waitIdle();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			mDevice.destroySemaphore(mRenderFinishedSemaphores[i]);
			mDevice.destroySemaphore(mImageAvailableSemaphores[i]);
			mDevice.destroyFence(mInFlightFences[i]);
		}

		mDevice.destroyPipeline(mGraphicsPipeline);
		for (size_t i = 0; i < mSwapChainFrameBuffers.size(); i++)
		{
			mDevice.destroyFramebuffer(mSwapChainFrameBuffers[i]);
		}
		mDevice.destroyCommandPool(mCommandPool);
	}

} // namespace jet
