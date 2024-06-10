#ifndef JET_RENDERER_H
#define JET_RENDERER_H

#include "def.h"
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace jet
{
	struct UniformBufferObject
	{
		m4 model;
		m4 view;
		m4 proj;
	};

	class Renderer
	{
		const std::vector<const char*> mValidationLayers = { "VK_LAYER_KHRONOS_validation" };

		const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
		static constexpr bool mEnableValidationLayers = false;
#else
		static constexpr bool mEnableValidationLayers = true;
#endif

		vk::Instance mInstance;
		vk::DebugUtilsMessengerEXT mDebugMessenger;
		vk::PhysicalDevice mPhysicalDevice;
		vk::Device mDevice;
		vk::Queue mGraphicsQueue;
		vk::Queue mPresentQueue;
		vk::SurfaceKHR mSurface;
		vk::SwapchainKHR mSwapchain;
		std::vector<vk::Image> mSwapchainImages;
		std::vector<vk::ImageView> mSwapchainImageViews;
		vk::Format mSwapchainImageFormat;
		vk::Extent2D mSwapchainExtent;
		vk::Pipeline mGraphicsPipeline;
		vk::DescriptorSetLayout mDescriptorSetLayout;
		vk::PipelineLayout mPipelineLayout;
		vk::RenderPass mRenderPass;
		vk::CommandPool mCommandPool;

		std::vector<vk::CommandBuffer> mCommandBuffers;
		std::vector<vk::Semaphore> mImageAvailableSemaphores;
		std::vector<vk::Semaphore> mRenderFinishedSemaphores;
		std::vector<vk::Fence> mInFlightFences;

		std::vector<vk::Framebuffer> mSwapChainFrameBuffers;

		class Window* mWindow;

		u32 mCurrentFrame = 0;

		friend class Engine;

		vk::Buffer mVertexBuffer;
		vk::DeviceMemory mVertexBufferMemory;
		vk::Buffer mIndexBuffer;
		vk::DeviceMemory mIndexBufferMemory;

		std::vector<vk::Buffer> mUniformBuffers;
		std::vector<vk::DeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;

		vk::DescriptorPool mDescriptorPool;
		std::vector<vk::DescriptorSet> mDescriptorSets;

	public:
		const vk::PhysicalDevice& GetPhysicalDevice() const;
		const vk::Device& GetDevice() const;
		const vk::SurfaceKHR& GetSurfaceKHR() const;
		const Window& GetWindow() const;

	private:
		void Init(class Window* window);

		void vCreateInstance();
		void vGetExtensions();
		void vSetValidationLayers();
		bool vCheckValidationLayerSupport();
		std::vector<const char*> vGetRequiredExtensions();
		static VKAPI_ATTR VkBool32 VKAPI_CALL vDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		void vSetupDebugMessenger();
		void vCreateSurface();
		void vCreateLogicalDevice();
		void vCreateSwapchain();
		void vCreateImageViews();
		void vCreateRenderPass();
		void vCreateDescriptorSetLayout();
		void vCreateGraphicsPipeline();
		void vCreateFrameBuffers();
		void vCreateCommandPool();
		void vCreateVertexBuffer();
		void vCreateIndexBuffer();
		void vCreateUniformBuffers();
		void vCreateDescriptorPool();
		void vCreateDescriptorSets();
		void vCreateCommandBuffers();
		void vCreateSyncObjects();

		void vRecreateSwapchain();
		void vCleanupSwapchain();

		void vRecordCommandBuffer(const vk::CommandBuffer& CommandBuffer, u32 imageIndex);
		void vUpdateUniformBuffer(u32 currentImage);

		void DrawFrame();

		void Clean();
	};

} // namespace jet

#endif // JET_RENDERER_H
