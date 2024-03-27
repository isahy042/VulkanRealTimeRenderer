
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // For glm::make_mat4
#include <glm/gtx/quaternion.hpp>

# include <vulkan/vulkan.h>

# include <iostream>
# include <stdexcept>
#include <fstream>

# include <vector>
# include <optional>
# include <set>
# include <array>

# include <cstring>
# include <cstdlib>

#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#include <chrono>

# include "s72parser.h"



// using directive
using namespace std;

uint32_t WIDTH = 800;
uint32_t HEIGHT = 600;

const int TEXTURE_COUNT = 7;
const int MAX_LIGHT = 10;
const int MAX_SUN_LIGHT = 5;

bool adjustDimension = true;

const string currRepo = ""; // for some reason this needs to be added manually :(

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// in place of frame buffers
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// defines how many frames should be processed concurrently
const int MAX_FRAMES_IN_FLIGHT = 2;

// structs

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities; // number of images/weight/height
	std::vector<VkSurfaceFormatKHR> formats; // pixel format and color space
	std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
	Vec3f pos;
	Vec3f normal;
	Vec4f tangent;
	Vec2f texCoord;
	Vec4f color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, tangent);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[4].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};

// structures for each type of light.
struct LightObject {
	alignas(16) float model[16]; 
	alignas(16) float data[16];
};

struct UniformBufferObject {
	// std430 in the layout qualifier
	alignas(16) float model[16]; //column major
	alignas(16) float view[16];
	alignas(16) float proj[16];
	alignas(16) float cameraTrans[16];

};

// skipped message callback in setup > validation layers (so no setupDebugMessenger implemented.)
class HelloTriangleApplication {

public:
	Scene scene = Scene();
	// arguments initilized by args 
	string s72filepath = "s72-main/examples/lights-Mix.s72";//sg-Articulation.s72";//

	string PreferredCamera = "";
	bool isCulling = true;
	bool isHeadless = false;
	string eventsFile = "example.events";

	void run() {
		loadModel();

		initWindow();
		initVulkan();

		if (isHeadless) mainLoopHeadless();
		else mainLoop();
		cleanup();
	}

private:
	GLFWwindow* window;
	VkInstance instance;

	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;
	
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	vector<VkPipelineLayout>pipelineLayout;
	vector<VkPipeline>graphicsPipeline;

	std::vector<VkFramebuffer> swapChainFramebuffers; // hold frame buffers

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	/*
	At a high level, rendering a frame in Vulkan consists of a common set of steps:
	1. Wait for the previous frame to finish
	2. Acquire an image from the swap chain
	3. Record a command buffer which draws the scene onto that image
	4. Submit the recorded command buffer
	5. Present the swap chain image
	*/

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	uint32_t currentFrame = 0;

	bool framebufferResized = false;

	vector<VkBuffer> vertexBuffer;
	vector<VkDeviceMemory> vertexBufferMemory;
	vector<VkBuffer> indexBuffer;
	vector<VkDeviceMemory> indexBufferMemory;

	vector<vector<VkBuffer>> uniformBuffers;
	vector<vector<VkDeviceMemory>> uniformBuffersMemory;
	vector<vector<void*>> uniformBuffersMapped;

	// light buffers
	vector < vector<vector<VkBuffer>>> sunlightBuffers;
	vector < vector<vector<VkDeviceMemory>>> sunlightBuffersMemory;
	vector < vector<vector<void*>>> sunlightBuffersMapped;

	vector < vector<vector<VkBuffer>>> spherelightBuffers;
	vector < vector<vector<VkDeviceMemory>>> spherelightBuffersMemory;
	vector < vector<vector<void*>>> spherelightBuffersMapped;

	vector < vector<vector<VkBuffer>>> spotlightBuffers;
	vector < vector<vector<VkDeviceMemory>>> spotlightBuffersMemory;
	vector < vector<vector<void*>>> spotlightBuffersMapped;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	vector<VkDescriptorSet> descriptorSets;

	// texture
	vector<vector<VkImage>> textureImage;
	vector<vector<VkDeviceMemory>> textureImageMemory;
	vector<vector<VkImageView>> textureImageView;
	vector<vector<VkSampler>> textureSampler;

	//depth
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	// vertices
	vector<vector<Vertex>> vertices;
	vector<vector<uint32_t>> indices;

	int totalObjects = 0;
	int totalMaterials = 0;

	int keyboardInput = 0;

	int totalFrames = 2;
	int currentFrameIndex = 0;

	string unusedTextureFile = "unused.png";

	void loadModel() {
		float aspect = scene.parseJson(s72filepath, PreferredCamera);
		if (adjustDimension && !isHeadless) {
			float w = static_cast<float>(WIDTH);
			float h = static_cast<float>(HEIGHT);
			if ((h * aspect > w) && (h * aspect <= 16000)) {
				w = floor(h * aspect);
				HEIGHT = static_cast<uint32_t>(h);
			}
			else {
				h = floor(w / aspect);
				WIDTH = static_cast<uint32_t>(w);
			}
		}
		// create objects
		scene.InstantiateObjects();

		int c = 0;
		for (int obj = 0; obj < scene.objects.size(); obj++) {
			vector<Vertex> v;
			vector<uint32_t> ind;

			int matind = scene.s72map[scene.objects[obj].mesh.material].second;
			if (scene.materials[matind]->getType() == "simple") {
				for (int i = 0; i < scene.objects[obj].mesh.count; i++) {
					v.push_back({scene.objects[obj].position[i], scene.objects[obj].normal[i], {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, scene.objects[obj].color[i] });
					ind.push_back(i);
				}
			}
			else {

				for (int i = 0; i < scene.objects[obj].mesh.count; i++) {
					v.push_back({ scene.objects[obj].position[i], scene.objects[obj].normal[i], scene.objects[obj].tangent[i], scene.objects[obj].texcoord[i], scene.objects[obj].color[i]});
					ind.push_back(i);
				}
			}
			vertices.push_back(v);
			indices.push_back(ind);


		}
		totalObjects = scene.objects.size();
		totalMaterials = scene.materials.size();
		totalFrames = scene.totalFrames;
		cout << "\n MODEL LOADED. " << totalFrames << " total frames with #" << totalObjects << " objects, #" << totalMaterials << " materials. \n";
	 
		// testing this:
		// TODO: not hardcode in the specular map for the environment cube.
		//makeLambertianCubeMap("ocean-map.png");
		//makeLambertianCubeMap("env-cube1.png");

		//float roughness = 0.f;
		//for (int r = 0; r < 6; r++) {
		//	makePBRCubeMap("env-cube1.png", roughness, r);
		//	roughness += 0.2f;
		//}

		//makePBRLUT();
	}


	// keyboard call back
	void executeKeyboardEvents() {
		if (glfwGetKey(window, GLFW_KEY_A)) scene.cameraMovement.x -= 0.05f;
		else if (glfwGetKey(window, GLFW_KEY_D)) scene.cameraMovement.x += 0.05f;
		else if (glfwGetKey(window, GLFW_KEY_W)) scene.cameraMovement.y += 0.05f;
		else if (glfwGetKey(window, GLFW_KEY_S)) scene.cameraMovement.y -= 0.05f;
		else if (glfwGetKey(window, GLFW_KEY_Z)) scene.cameraMovement.z -= 0.05f;
		else if (glfwGetKey(window, GLFW_KEY_X)) scene.cameraMovement.z += 0.1f;

		else if (glfwGetKey(window, GLFW_KEY_RIGHT)) scene.cameraRot.y += 0.01f;
		else if (glfwGetKey(window, GLFW_KEY_LEFT)) scene.cameraRot.y -= 0.01f;
		else if (glfwGetKey(window, GLFW_KEY_DOWN)) scene.cameraRot.x += 0.01f;
		else if (glfwGetKey(window, GLFW_KEY_UP)) scene.cameraRot.x -= 0.01f;

		else if (glfwGetKey(window, GLFW_KEY_R)) scene.cameraMovement = Vec3f(0.f);

		else if (glfwGetKey(window, GLFW_KEY_C) && keyboardInput == 0) {  
			scene.updateFrustum = !scene.updateFrustum; 
			cout << "\n nolonger upating frustum." << scene.updateFrustum; 
			keyboardInput = 1;
		} // one press
		else if (!glfwGetKey(window, GLFW_KEY_C)){ keyboardInput = 0; // no key is pressed now. 
		}
	}

	// picking physical device 
	// TODO: implement device rating and picking the highest scored one, as denoted in setup > physical device and queues
	/** checks if device is suitable for the operations we want to perform */
	bool isDeviceSuitable(VkPhysicalDevice device) {

		//VkPhysicalDeviceProperties deviceProperties;
		//VkPhysicalDeviceFeatures deviceFeatures;
		//vkGetPhysicalDeviceProperties(device, &deviceProperties);

		//// The support for optional features like texture compression, 64 bit floats and multi viewport 
		//// rendering (useful for VR) can be queried using vkGetPhysicalDeviceFeatures:
		//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		//return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

		QueueFamilyIndices indices = findQueueFamilies(device);
		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	void pickPhysicalDevice() {

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		// If there are 0 devices with Vulkan support then there is no point going further.
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	// window?
	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	// creating logical device
	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

	}

	// queues
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {

		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			// look for a queue family that has the capability of presenting to our window surface
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	// instance and validation layers

	/** check if all of the layers in validationLayers exist in the availableLayers list.
	This program should now should not ever return a VK_ERROR_LAYER_NOT_PRESENT error */
	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	void createInstance() {
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		createInfo.enabledLayerCount = 0;

		//  include the validation layer names if they are enabled
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}
		//VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}


	}

	// swap chain 
	/** check if all required extensions are available. */
	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		// query support presentation modes
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		// verify that swap chain support is adequate - 
		// at least one supported omage foramat and presentation mode for our window surface
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	// swap chain - finding the most ideal settings
	/* surface format - color depth*/
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	/* presentation mode - 4 total in vulkan*/
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {

		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR; // better on mobile devices, where energy usage is more important
	}

	/* swap extent - resolution of images in swap chain
	resolution in screen coordinates can sometimes < resolution in pixels
	meaning we cant just use the WIDTH, HEIGHT, but instead glfwGetFramebufferSize*/
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
			
			return actualExtent;
		}
	}

	/* creating the swap chain!*/
	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);


		//cout << "\nactual extent" << extent.width << extent.height;

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}
		
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		
		if ((HEIGHT > extent.height || WIDTH > extent.width) && (isHeadless)){
			extent.height = HEIGHT;
			extent.width = WIDTH;
		}
		// if extent is too big, scale it.
		// TODO: VUID-VkSwapchainCreateInfoKHR-pNext-07781
		VkSwapchainPresentScalingCreateInfoEXT scalingCreateInfo{};

		scalingCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_EXT;
		//scalingCreateInfo.pNext = &createInfo;
		scalingCreateInfo.scalingBehavior = VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT; 
		scalingCreateInfo.presentGravityX = 4;
		scalingCreateInfo.presentGravityY = 4;

		createInfo.pNext = &scalingCreateInfo;
		//cout << "\npnext set! " << scalingCreateInfo.scalingBehavior;


		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // unless stereoscopic 3D application
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT; //VK_IMAGE_USAGE_TRANSFER_DST_BIT - use a memory operation to transfer the rendered image to a swap chain image

		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;

	}

	// image views 
	void createImageViews() {

		swapChainImageViews.resize(swapChainImages.size());

		for (uint32_t i = 0; i < swapChainImages.size(); i++) {
			swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		}

	}

	// graphics pipeline!
	static std::vector<char> readFile(const std::string& filename) {

		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			std::cerr << "Failed to open file: " << filename << std::endl;
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	VkShaderModule createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}


	void createGraphicsPipeline(int materialIndex) {

		auto vertShaderCode = readFile("shaders/"+ scene.materials[materialIndex]->getVertshader());
		auto fragShaderCode = readFile("shaders/"+ scene.materials[materialIndex]->getFragshader());

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		//rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		// multisampling
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		
		// render pass
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		// color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout[materialIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}


		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout[materialIndex];
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline[materialIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	void createRenderPass() {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	// frame buffer
	void createFramebuffers() {
		// resizing the container to hold all of the framebuffers
		swapChainFramebuffers.resize(swapChainImageViews.size());

		// iterate through the image views and create framebuffers from them:
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			std::array<VkImageView, 2> attachments = {
				swapChainImageViews[i], depthImageView
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	// command pool - manage the memory that is used to store the buffers and command buffers are allocated from them
	void createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	// command buffer
	void createCommandBuffer() {

		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	// writes the commands we want to execute into a command buffer
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		for (int mat=0; mat<totalMaterials; mat++){
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline[mat]);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)swapChainExtent.width;
			viewport.height = (float)swapChainExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = swapChainExtent;
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			for (int obj : scene.materials[mat]->getObjList()) {
				if (!scene.objects[obj].inFrame && isCulling) continue;
				VkBuffer vertexBuffers[] = { vertexBuffer[obj] };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(commandBuffer, indexBuffer[obj], 0, VK_INDEX_TYPE_UINT32);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout[mat], 0, 1, &descriptorSets[(currentFrame * totalObjects) + obj], 0, nullptr);
				vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices[obj].size()), 1, 0, 0, 0);
			}
		}
		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	// sync object - ordering
	void createSyncObjects() {

		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	void updateUniformBuffer(uint32_t currentImage, int objIndex) {
		//static auto startTime = std::chrono::high_resolution_clock::now();

		//auto currentTime = std::chrono::high_resolution_clock::now();
		//float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};

		std::memcpy(ubo.model, makeUboMatrix(transpose44(scene.objects[objIndex].transformMatrix)), 16 * sizeof(float));
		std::memcpy(ubo.view, makeUboMatrix((*scene.currCam).viewMatrix), 16 * sizeof(float));
		std::memcpy(ubo.proj, makeUboMatrix((*scene.currCam).projectionMatrix), 16 * sizeof(float));
		ubo.proj[5] *= -1;

		std::memcpy(ubo.cameraTrans, makeUboMatrix((*scene.currCam).transformMatrix), 16 * sizeof(float));
		 // view44((*scene.currCam).transformMatrix, "\n");

		memcpy(uniformBuffersMapped[objIndex][currentImage], &ubo, sizeof(ubo));
	}

	void updateLightBuffer(uint32_t currentImage, int objIndex) {
		// for now, put all light in all light buffers.
		int sphereind = 0;
		int spotind = 0;
		int sunind = 0;

		// copying all lights
		for (int i = 0; i < scene.lights.size(); i++) {

			shared_ptr<Light> light = scene.lights[i];

			// copy data to light object
			LightObject l{};
			std::memcpy(l.model, makeUboMatrix(transpose44(light->getTransformationMatrix())), 16 * sizeof(float));
			std::memcpy(l.data, makeUboMatrix(light->getDataMatrix()), 16 * sizeof(float));
			
			if (light->getType() == 0 && sphereind < MAX_LIGHT) { //sphere
				/*cout << "\n showing sphere light at " << i;
				view44(light->getTransformationMatrix(), "\n trans");
				view44(light->getDataMatrix(), "\n data");*/

				memcpy(spherelightBuffersMapped[objIndex][currentImage][sphereind], &l, sizeof(l));
				sphereind++;
			}
			else if (light->getType() == 1 && spotind < MAX_LIGHT) { //spot
				/*cout << "\n showing spot light at " << i;
				view44(light->getTransformationMatrix(), "\n trans");
				view44(light->getDataMatrix(), "\n data");*/

				memcpy(spotlightBuffersMapped[objIndex][currentImage][spotind], &l, sizeof(l));
				spotind++;
			}
			else if (light->getType() == 2 && sunind < MAX_SUN_LIGHT) { // sun
				/*cout << "\n showing sun light at " << i;
				view44(light->getTransformationMatrix(), "\n trans");
				view44(light->getDataMatrix(), "\n data");*/

				memcpy(sunlightBuffersMapped[objIndex][currentImage][sunind], &l, sizeof(l));
				sunind++;
			}
			else {
				cerr << "\n unsupported light type encountered when updating light buffer.";
			}
		}

		// tell the shader when to stop.
		float* notUsedLight = makeUboMatrix(Vec44f(Vec4f(0.f)));
		LightObject l{};
		std::memcpy(l.model, notUsedLight, 16 * sizeof(float));
		std::memcpy(l.data, notUsedLight, 16 * sizeof(float));

		if (sphereind < MAX_LIGHT) { //sphere
			memcpy(spherelightBuffersMapped[objIndex][currentImage][sphereind], &l, sizeof(l));
		}
		if (spotind < MAX_LIGHT) { //spot
			memcpy(spotlightBuffersMapped[objIndex][currentImage][spotind], &l, sizeof(l));
		}
		if (sunind < MAX_SUN_LIGHT) { // sun
			memcpy(sunlightBuffersMapped[objIndex][currentImage][sunind], &l, sizeof(l));
		}

	}

	void drawFrame() {

		// wait for prev frame
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		// doing this for multiple vertex buffers!
		auto startTime = std::chrono::high_resolution_clock::now();
		
		// scene processing
		scene.updateSceneTransformMatrix(currentFrameIndex);
		scene.cull();
		for (int obj = 0; obj < totalObjects; obj++) {
			if (isCulling && !scene.objects[obj].inFrame) continue;
			updateUniformBuffer(currentFrame, obj);
			updateLightBuffer(currentFrame, obj);
		}

		// manually reset the fence to the unsignaled state
		// Only reset the fence if we are submitting work
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

			vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
			recordCommandBuffer(commandBuffers[currentFrame], imageIndex);
		
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

			VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;


			if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
				throw std::runtime_error("failed to submit draw command buffer!");
			}
		
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { swapChain };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = &imageIndex;

			result = vkQueuePresentKHR(presentQueue, &presentInfo);

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
				framebufferResized = false;
				recreateSwapChain();
			}
			else if (result != VK_SUCCESS) {
				throw std::runtime_error("failed to present swap chain image!");
			}
			//recreateSwapChain();

		
		currentFrameIndex = (currentFrameIndex + 1) % totalFrames;
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		auto currentTime = std::chrono::high_resolution_clock::now();
		auto time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		//cout << "\n frame time: " << time;
	}

	void saveImg(string filename) {
		VkDeviceSize imageSize = WIDTH * HEIGHT * 4;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			VkCommandBuffer commandBuffer = beginSingleTimeCommands();

			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				WIDTH,
				HEIGHT,
				1
			};

			vkCmdCopyImageToBuffer(commandBuffer, swapChainImages[currentFrame], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);
			endSingleTimeCommands(commandBuffer);

			void* mappedMemory;
			vkMapMemory(device, stagingBufferMemory, 0, WIDTH * HEIGHT * 3, 0, &mappedMemory);

			ofstream file(filename, std::ios::out | std::ios::binary | std::ios::trunc);
			if (!file.is_open()) {
				std::cerr << "Failed to open file: " << filename << std::endl;
				return;
			}

			// Write PPM header
			file << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";

			// Access and print buffer contents (assuming buffer contains uint32_t data)
			unsigned char* data = reinterpret_cast<unsigned char*>(mappedMemory);
			for (size_t i = 0; i < WIDTH * HEIGHT * 4; ++i) {
				if (i % 4 != 0) file << data[i];
			}

			file.close();

			// Unmap buffer memory
			vkUnmapMemory(device, stagingBufferMemory);

			// Destroy the buffer
			vkDestroyBuffer(device, stagingBuffer, nullptr);

			// Free the memory
			vkFreeMemory(device, stagingBufferMemory , nullptr);


	}



	// swap chain recreation
	void recreateSwapChain() {

		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);

		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);

		cleanupSwapChain();

		createSwapChain();
		createImageViews();
		createDepthResources();
		createFramebuffers();
	}

	void cleanupSwapChain() {
		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);

		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	// vertex buffer
	void createVertexBuffer(int objIndex) {
		VkDeviceSize bufferSize = sizeof(vertices[0][0]) * vertices[objIndex].size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices[objIndex].data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer[objIndex], vertexBufferMemory[objIndex]);

		copyBuffer(stagingBuffer, vertexBuffer[objIndex], bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// just memory. called by create vertex buffer
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	VkCommandBuffer beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	// index buffer
	void createIndexBuffer(int objIndex) {
		VkDeviceSize bufferSize = sizeof(indices[0][0]) * indices[objIndex].size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices[objIndex].data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer[objIndex], indexBufferMemory[objIndex]);

		copyBuffer(stagingBuffer, indexBuffer[objIndex], bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// descriptor pool
	void createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		// environment
		VkDescriptorSetLayoutBinding samplerLayoutBinding1{};
		samplerLayoutBinding1.binding = 1;
		samplerLayoutBinding1.descriptorCount = 1;
		samplerLayoutBinding1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding1.pImmutableSamplers = nullptr;
		samplerLayoutBinding1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// albedo 
		VkDescriptorSetLayoutBinding samplerLayoutBinding2{};
		samplerLayoutBinding2.binding = 2;
		samplerLayoutBinding2.descriptorCount = 1;
		samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding2.pImmutableSamplers = nullptr;
		samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// roughness
		VkDescriptorSetLayoutBinding samplerLayoutBinding3{};
		samplerLayoutBinding3.binding = 3;
		samplerLayoutBinding3.descriptorCount = 1;
		samplerLayoutBinding3.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding3.pImmutableSamplers = nullptr;
		samplerLayoutBinding3.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// metalness
		VkDescriptorSetLayoutBinding samplerLayoutBinding4{};
		samplerLayoutBinding4.binding = 4;
		samplerLayoutBinding4.descriptorCount = 1;
		samplerLayoutBinding4.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding4.pImmutableSamplers = nullptr;
		samplerLayoutBinding4.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// normal
		VkDescriptorSetLayoutBinding samplerLayoutBinding5{};
		samplerLayoutBinding5.binding = 5;
		samplerLayoutBinding5.descriptorCount = 1;
		samplerLayoutBinding5.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding5.pImmutableSamplers = nullptr;
		samplerLayoutBinding5.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// dsplacemet
		VkDescriptorSetLayoutBinding samplerLayoutBinding6{};
		samplerLayoutBinding6.binding = 6;
		samplerLayoutBinding6.descriptorCount = 1;
		samplerLayoutBinding6.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding6.pImmutableSamplers = nullptr;
		samplerLayoutBinding6.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding7{};
		samplerLayoutBinding7.binding = 7;
		samplerLayoutBinding7.descriptorCount = 1;
		samplerLayoutBinding7.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding7.pImmutableSamplers = nullptr;
		samplerLayoutBinding7.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding lightLayoutBinding{};
		lightLayoutBinding.binding = 8;
		lightLayoutBinding.descriptorCount = MAX_LIGHT;
		lightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightLayoutBinding.pImmutableSamplers = nullptr;
		lightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding lightLayoutBinding1{};
		lightLayoutBinding1.binding = 9;
		lightLayoutBinding1.descriptorCount = MAX_LIGHT;
		lightLayoutBinding1.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightLayoutBinding1.pImmutableSamplers = nullptr;
		lightLayoutBinding1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding lightLayoutBinding2{};
		lightLayoutBinding2.binding = 10;
		lightLayoutBinding2.descriptorCount = MAX_SUN_LIGHT;
		lightLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightLayoutBinding2.pImmutableSamplers = nullptr;
		lightLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// TODO: displacement, normal.
		std::array<VkDescriptorSetLayoutBinding, 11> bindings = 
		{ uboLayoutBinding, samplerLayoutBinding1,samplerLayoutBinding2,
			samplerLayoutBinding3 ,samplerLayoutBinding4 ,samplerLayoutBinding5,
			samplerLayoutBinding6, samplerLayoutBinding7, lightLayoutBinding,
			lightLayoutBinding1,lightLayoutBinding2 };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();


		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void createDescriptorPool() {

		std::array<VkDescriptorPoolSize, 11> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects);
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects);
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects);
		poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[4].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects);
		poolSizes[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[5].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects);
		poolSizes[6].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[6].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects);
		poolSizes[7].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[7].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects);
		poolSizes[8].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[8].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects * MAX_LIGHT);
		poolSizes[9].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[9].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects * MAX_LIGHT);
		poolSizes[10].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[10].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects * MAX_SUN_LIGHT);


		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects);

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void createDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * totalObjects, descriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * totalObjects);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT * totalObjects);

		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		// when 1D array, store so that all objects of one frame are together.
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			for (size_t obj = 0; obj < totalObjects; obj++) {

				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniformBuffers[obj][i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UniformBufferObject);

				int materialIndex = scene.objects[obj].mesh.material; // node of currently used material
				if (scene.s72map[materialIndex].first != 5) cerr << "\n incorrect object instead of material encountered at descriptor set creation.";
				materialIndex = scene.s72map[materialIndex].second; // map material to index of materials only

				// environment
				VkDescriptorImageInfo imageInfo1{};
				imageInfo1.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo1.imageView = textureImageView[materialIndex][0];
				imageInfo1.sampler = textureSampler[materialIndex][0];

				// albedo
				VkDescriptorImageInfo imageInfo2{};
				imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo2.imageView = textureImageView[materialIndex][1];
				imageInfo2.sampler = textureSampler[materialIndex][1];

				// roughness
				VkDescriptorImageInfo imageInfo3{};
				imageInfo3.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo3.imageView = textureImageView[materialIndex][2];
				imageInfo3.sampler = textureSampler[materialIndex][2];

				//metalness
				VkDescriptorImageInfo imageInfo4{};
				imageInfo4.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo4.imageView = textureImageView[materialIndex][3];
				imageInfo4.sampler = textureSampler[materialIndex][3];

				//normal
				VkDescriptorImageInfo imageInfo5{};
				imageInfo5.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo5.imageView = textureImageView[materialIndex][4];
				imageInfo5.sampler = textureSampler[materialIndex][4];

				//displacement
				VkDescriptorImageInfo imageInfo6{};
				imageInfo6.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo6.imageView = textureImageView[materialIndex][5];
				imageInfo6.sampler = textureSampler[materialIndex][5];

				//displacement
				VkDescriptorImageInfo imageInfo7{};
				imageInfo7.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo7.imageView = textureImageView[materialIndex][6];
				imageInfo7.sampler = textureSampler[materialIndex][6];

				std::array<VkWriteDescriptorSet, 11> descriptorWrites{};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = descriptorSets[(i * totalObjects) + obj];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;
				
				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = descriptorSets[(i * totalObjects) + obj];
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &imageInfo1;

				descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[2].dstSet = descriptorSets[(i * totalObjects) + obj];
				descriptorWrites[2].dstBinding = 2;
				descriptorWrites[2].dstArrayElement = 0;
				descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[2].descriptorCount = 1;
				descriptorWrites[2].pImageInfo = &imageInfo2;

				descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[3].dstSet = descriptorSets[(i * totalObjects) + obj];
				descriptorWrites[3].dstBinding = 3;
				descriptorWrites[3].dstArrayElement = 0;
				descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[3].descriptorCount = 1;
				descriptorWrites[3].pImageInfo = &imageInfo3;

				descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[4].dstSet = descriptorSets[(i * totalObjects) + obj];
				descriptorWrites[4].dstBinding = 4;
				descriptorWrites[4].dstArrayElement = 0;
				descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[4].descriptorCount = 1;
				descriptorWrites[4].pImageInfo = &imageInfo4;

				descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[5].dstSet = descriptorSets[(i * totalObjects) + obj];
				descriptorWrites[5].dstBinding = 5;
				descriptorWrites[5].dstArrayElement = 0;
				descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[5].descriptorCount = 1;
				descriptorWrites[5].pImageInfo = &imageInfo5;

				descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[6].dstSet = descriptorSets[(i * totalObjects) + obj];
				descriptorWrites[6].dstBinding = 6;
				descriptorWrites[6].dstArrayElement = 0;
				descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[6].descriptorCount = 1;
				descriptorWrites[6].pImageInfo = &imageInfo6;

				descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[7].dstSet = descriptorSets[(i * totalObjects) + obj];
				descriptorWrites[7].dstBinding = 7;
				descriptorWrites[7].dstArrayElement = 0;
				descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[7].descriptorCount = 1;
				descriptorWrites[7].pImageInfo = &imageInfo7;

				// filling in descriptor set for lights. three, each for one type of light

				std::array<VkDescriptorBufferInfo, MAX_LIGHT> bufferInfoSphere;
				std::array<VkDescriptorBufferInfo, MAX_LIGHT> bufferInfoSpot;
				std::array<VkDescriptorBufferInfo, MAX_SUN_LIGHT> bufferInfoSun;
				

				for (size_t j = 0; j < MAX_LIGHT; j++) {
					VkDescriptorBufferInfo bi1{};
					bi1.buffer = { spherelightBuffers[obj][i][j] };
					bi1.offset = 0;
					bi1.range = sizeof(LightObject);

					VkDescriptorBufferInfo bi2{};
					bi2.buffer = { spotlightBuffers[obj][i][j] };
					bi2.offset = 0;
					bi2.range = sizeof(LightObject);

					bufferInfoSphere[j] = bi1;
					bufferInfoSpot[j] = bi2;
				}

				for (size_t j = 0; j < MAX_SUN_LIGHT; j++) {
					VkDescriptorBufferInfo bi1{};
					bi1.buffer = { sunlightBuffers[obj][i][j] };
					bi1.offset = 0;
					bi1.range = sizeof(LightObject);

					bufferInfoSun[j] = bi1;
				}

				descriptorWrites[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[8].dstSet = descriptorSets[(i * totalObjects) + obj];
				descriptorWrites[8].dstBinding = 8;
				descriptorWrites[8].dstArrayElement = 0;
				descriptorWrites[8].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[8].descriptorCount = MAX_LIGHT;
				descriptorWrites[8].pBufferInfo = bufferInfoSphere.data();

				descriptorWrites[9].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[9].dstSet = descriptorSets[(i * totalObjects) + obj];
				descriptorWrites[9].dstBinding = 9;
				descriptorWrites[9].dstArrayElement = 0;
				descriptorWrites[9].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[9].descriptorCount = MAX_LIGHT;
				descriptorWrites[9].pBufferInfo = bufferInfoSpot.data();

				descriptorWrites[10].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[10].dstSet = descriptorSets[(i * totalObjects) + obj];
				descriptorWrites[10].dstBinding = 10;
				descriptorWrites[10].dstArrayElement = 0;
				descriptorWrites[10].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[10].descriptorCount = MAX_SUN_LIGHT;
				descriptorWrites[10].pBufferInfo = bufferInfoSun.data();

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
		}
	}

	void createUniformBuffers(int objIndex) {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers[objIndex].resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory[objIndex].resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped[objIndex].resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				uniformBuffers[objIndex][i], uniformBuffersMemory[objIndex][i]);

			vkMapMemory(device, uniformBuffersMemory[objIndex][i], 0, bufferSize, 0, &uniformBuffersMapped[objIndex][i]);
		}
	}

	void createLightBuffers(int objIndex) {
		VkDeviceSize bufferSize = sizeof(LightObject);
		sunlightBuffers[objIndex].resize(MAX_FRAMES_IN_FLIGHT);
		sunlightBuffersMemory[objIndex].resize(MAX_FRAMES_IN_FLIGHT);
		sunlightBuffersMapped[objIndex].resize(MAX_FRAMES_IN_FLIGHT);

		spotlightBuffers[objIndex].resize(MAX_FRAMES_IN_FLIGHT);
		spotlightBuffersMemory[objIndex].resize(MAX_FRAMES_IN_FLIGHT);
		spotlightBuffersMapped[objIndex].resize(MAX_FRAMES_IN_FLIGHT);

		spherelightBuffers[objIndex].resize(MAX_FRAMES_IN_FLIGHT);
		spherelightBuffersMemory[objIndex].resize(MAX_FRAMES_IN_FLIGHT);
		spherelightBuffersMapped[objIndex].resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			spotlightBuffers[objIndex][i].resize(MAX_LIGHT);
			spotlightBuffersMemory[objIndex][i].resize(MAX_LIGHT);
			spotlightBuffersMapped[objIndex][i].resize(MAX_LIGHT);

			spherelightBuffers[objIndex][i].resize(MAX_LIGHT);
			spherelightBuffersMemory[objIndex][i].resize(MAX_LIGHT);
			spherelightBuffersMapped[objIndex][i].resize(MAX_LIGHT);

			sunlightBuffers[objIndex][i].resize(MAX_SUN_LIGHT);
			sunlightBuffersMemory[objIndex][i].resize(MAX_SUN_LIGHT);
			sunlightBuffersMapped[objIndex][i].resize(MAX_SUN_LIGHT);

			// initializing buffer for spot and sphere light
			for (size_t j = 0; j < MAX_LIGHT; j++) {

				createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					spotlightBuffers[objIndex][i][j], spotlightBuffersMemory[objIndex][i][j]);
				createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					spherelightBuffers[objIndex][i][j], spherelightBuffersMemory[objIndex][i][j]);

				vkMapMemory(device, spotlightBuffersMemory[objIndex][i][j], 0, bufferSize, 0, &spotlightBuffersMapped[objIndex][i][j]);
				vkMapMemory(device, spherelightBuffersMemory[objIndex][i][j], 0, bufferSize, 0, &spherelightBuffersMapped[objIndex][i][j]);
			}
			// initializing buffer for sun light
			for (size_t j = 0; j < MAX_SUN_LIGHT; j++) {
				createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					sunlightBuffers[objIndex][i][j], sunlightBuffersMemory[objIndex][i][j]);

				vkMapMemory(device, sunlightBuffersMemory[objIndex][i][j], 0, bufferSize, 0, &sunlightBuffersMapped[objIndex][i][j]);
			}

		}
	}

	//// texture 

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer);
	}

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endSingleTimeCommands(commandBuffer);
	}

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image view!");
		}

		return imageView;
	}

	void createCubeMapTextures(int materialIndex) {
		string type = scene.materials[materialIndex]->getType();
		textureImage[materialIndex].resize(TEXTURE_COUNT);
		textureImageMemory[materialIndex].resize(TEXTURE_COUNT);
		textureImageView[materialIndex].resize(TEXTURE_COUNT);
		textureSampler[materialIndex].resize(TEXTURE_COUNT);

		if (type == "env") { // create environmet cube map.
			//TODO: take both color AND filename. right now its only file name
			createCubeMap(scene.envMat->getBaseColor(), materialIndex, 0);
			createCubeMap(unusedTextureFile, materialIndex, 1);
			createCubeMap(unusedTextureFile, materialIndex, 2);
			createCubeMap(unusedTextureFile, materialIndex, 3);
		}
		else if (type == "mirror") // nothing for now.
		{
			createCubeMap(scene.envMat->getBaseColor(), materialIndex, 0);
			createCubeMap(unusedTextureFile, materialIndex, 1);
			createCubeMap(unusedTextureFile, materialIndex, 2);
			createCubeMap(unusedTextureFile, materialIndex, 3);
		}
		else if (type == "lamb") // 
		{
			createCubeMap("lambertian-map-"+ scene.envMat->getBaseColor(), materialIndex, 0);
			createCubeMap(scene.materials[materialIndex]->getBaseColor(), materialIndex, 1);
			createCubeMap(unusedTextureFile, materialIndex, 2);
			createCubeMap(unusedTextureFile, materialIndex, 3);
		}
		else if (type == "pbr") //pbr
		{
			createCubeMap(scene.materials[materialIndex]->getBaseColor(), materialIndex, 0);
			createCubeMap(scene.materials[materialIndex]->getRoughness(), materialIndex, 1);
			createCubeMap(scene.materials[materialIndex]->getMetalness(), materialIndex, 2);
			createCubeMap(scene.envMat->getBaseColor(), materialIndex, 3, true);
		}

		// displacement and normal map.
		createCubeMap(scene.materials[materialIndex]->getNormal(), materialIndex, 4);
		createCubeMap(scene.materials[materialIndex]->getDisplacement(), materialIndex, 5);
		create2DTexture("pbr-map.png", materialIndex, 6);
	}

	void createCubeMap(string filename, int matIndex, int category, bool mipmap = false) {
		createCubeMapTexture(filename, matIndex, category, mipmap);
		createCubeMapTextureView(matIndex, category, mipmap);
		createCubeMapSampler(matIndex, category, mipmap);
	}

	void createCubeMapTexture(string filename, int matIndex, int category, bool mipmap = false) {

		int texWidth, texHeight, texChannels;
		VkDeviceSize imageSize, layerSize;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		// the file name may also be a Vec3f of colors. In this case, generate 6x1 map.
		// TODO:make this more rigid!!!
		if (!mipmap && (filename.substr(0, 1) == "[" || filename.substr(filename.size() - 1, 1) == "]")) {
			Vec3f color = tovec3f(filename);

			texWidth = 1;
			texHeight = 6;
			texChannels = 4;
			imageSize = texWidth * texHeight * 4;
			layerSize = imageSize / 6;

			unsigned char pixels[24];
			for (int i = 0; i < 6; i++) {
				pixels[(i * 4)] = (int)(color.x*255.f);
				pixels[(i * 4)+1] = (int)(color.y * 255.f);
				pixels[(i * 4)+2] = (int)(color.z * 255.f);
				pixels[(i * 4)+3] = 129;
			}

			createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			// map memory
			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);

			memcpy(data, pixels, static_cast<size_t>(imageSize));
			vkUnmapMemory(device, stagingBufferMemory);

			//TODO: free this.uc
			//stbi_image_free(pixels);

			createCubeMapImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage[matIndex][category], textureImageMemory[matIndex][category], mipmap);

			transitionCubeMapImageLayout(textureImage[matIndex][category], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipmap);
			copyBufferToCubeMapImage(stagingBuffer, textureImage[matIndex][category], static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), mipmap);
			transitionCubeMapImageLayout(textureImage[matIndex][category], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipmap);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);
		}
		else if (!mipmap) {
			string sn = "s72-main/examples/texture/" + filename;
			const char* charArray = sn.c_str();
			stbi_uc* pixels = stbi_load(charArray, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

			VkDeviceSize imageSize = texWidth * texHeight * 4;
			VkDeviceSize layerSize = imageSize / 6;

			if (!pixels) {
				throw std::runtime_error("failed to load texture image!");
			}

			createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			// map memory
			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);

			memcpy(data, pixels, static_cast<size_t>(imageSize));
			vkUnmapMemory(device, stagingBufferMemory);

			stbi_image_free(pixels);

			createCubeMapImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage[matIndex][category], textureImageMemory[matIndex][category], mipmap);

			transitionCubeMapImageLayout(textureImage[matIndex][category], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipmap);
			copyBufferToCubeMapImage(stagingBuffer, textureImage[matIndex][category], static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), mipmap);
			transitionCubeMapImageLayout(textureImage[matIndex][category], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipmap);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);
		}
		else { // mipmap

			// read in the 6 mipmaps
			stbi_uc* mipmaps[6];

			for (int l = 0; l < 6; l++) {
				string sn = "s72-main/examples/texture/pbr-map-" + to_string(l) + "-"+ filename;
				const char* charArray = sn.c_str();
				mipmaps[l] = stbi_load(charArray, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

				if (!mipmaps[l]) {
					throw std::runtime_error("failed to load texture image for mipmap: " + sn);
				}
			}
			
			texWidth = 32;
			texHeight = 32 * 6;
			VkDeviceSize totalMipmapSize = 0;
			VkDeviceSize mipmapLevelSize = texWidth * texHeight * 4;
			for (int l = 0; l < 6; l++) {
				totalMipmapSize += mipmapLevelSize;
				mipmapLevelSize = mipmapLevelSize / 4;
			}

			cout << "\n total mipmap size: " << totalMipmapSize;
			createBuffer(totalMipmapSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			// map memory
			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, totalMipmapSize, 0, &data);

			//https://satellitnorden.wordpress.com/2018/03/13/vulkan-adventures-part-4-the-mipmap-menace-mipmapping-tutorial/

			texWidth = 32;
			texHeight = 32 * 6;
			mipmapLevelSize = texWidth * texHeight * 4;
			VkDeviceSize currentOffset = 0;
			for (int l = 0; l < 6; l++) {
				memcpy(static_cast<byte*>(data) + currentOffset, mipmaps[l],
					static_cast<size_t>(mipmapLevelSize));
				currentOffset += mipmapLevelSize;
				mipmapLevelSize = mipmapLevelSize / 4;
			}

			vkUnmapMemory(device, stagingBufferMemory);

			for (int l = 0; l < 6; l++) {
				stbi_image_free(mipmaps[l]);
			}

			createCubeMapImage(32, 32*6, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage[matIndex][category], textureImageMemory[matIndex][category], mipmap);

			transitionCubeMapImageLayout(textureImage[matIndex][category], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipmap);

			copyBufferToCubeMapImage(stagingBuffer, textureImage[matIndex][category], static_cast<uint32_t>(32), static_cast<uint32_t>(32*6), mipmap);

			transitionCubeMapImageLayout(textureImage[matIndex][category], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipmap);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);
			
		}

		
	}
	
	void copyBufferToCubeMapImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, bool mipmap) {
		if (!mipmap) {
			VkCommandBuffer commandBuffer = beginSingleTimeCommands();

			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 6;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				width,
				height / 6,
				1
			};

			vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			endSingleTimeCommands(commandBuffer);
		}
		else {
			vector<VkBufferImageCopy> bufferImageCopies;
			bufferImageCopies.resize(6);

			VkDeviceSize currentOffset = 0;
			VkDeviceSize imgsize = 32 * 32 * 4 * 6;
			for (int i = 0; i < 6; i++)
			{
			VkCommandBuffer commandBuffer = beginSingleTimeCommands();

				VkBufferImageCopy bufferImageCopy;

				bufferImageCopy.bufferOffset = currentOffset;
				bufferImageCopy.bufferRowLength = 0;
				bufferImageCopy.bufferImageHeight = 0;
				bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferImageCopy.imageSubresource.mipLevel = i;
				bufferImageCopy.imageSubresource.baseArrayLayer = 0;
				bufferImageCopy.imageSubresource.layerCount = 6;
				bufferImageCopy.imageOffset = { 0, 0, 0 };
				bufferImageCopy.imageExtent = { ((unsigned int)32) >> i, ((unsigned int)32) >> i, 1 };

				bufferImageCopies[i] = bufferImageCopy;
				
				currentOffset += imgsize;
				imgsize = imgsize / 4;

				vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopies[i]);

				endSingleTimeCommands(commandBuffer);
			}

			
		}

		
	}
	void createCubeMapTextureView(int matIndex, int category, bool mipmap = false)
	{
		textureImageView[matIndex][category] = createCubeMapImageView(textureImage[matIndex][category], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipmap);
	}

	void createCubeMapSampler(int matIndex, int category, bool mipmap) {
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE; // bypassing validation
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		if (mipmap) samplerInfo.maxLod = 6.f;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler[matIndex][category]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	VkImageView createCubeMapImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, bool isMipMap) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		if (isMipMap) viewInfo.subresourceRange.levelCount = 6;
		else viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 6;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image view!");
		}

		return imageView;
	}

	void createCubeMapImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, bool isMipMap) {

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height/6;
		imageInfo.extent.depth = 1;

		if (isMipMap) imageInfo.mipLevels = 6;
		else imageInfo.mipLevels = 1;

		imageInfo.arrayLayers = 6;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}


	void transitionCubeMapImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, bool isMipMap) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		if (isMipMap) barrier.subresourceRange.levelCount = 6;
		else barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 6;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer);
	}

	// image texture
	void create2DTexture(string filename, int matIndex, int category) {
		createTextureImage(filename, matIndex, category);

		createTextureImageView(filename, matIndex, category);

		createTextureSampler(matIndex, category);

	}

	void createTextureSampler(int matIndex, int category) {
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE; // bypassing validation
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler[matIndex][category]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	// texture 
	void createTextureImage(string filename, int matIndex, int category) {
		int texWidth, texHeight, texChannels;

		string sn = "s72-main/examples/texture/" + filename;
		const char* charArray = sn.c_str();
		stbi_uc* pixels = stbi_load(charArray, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		VkDeviceSize imageSize = texWidth * texHeight * 4;
		VkDeviceSize layerSize = imageSize;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image: " + filename);
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		// map memory
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);
		stbi_image_free(pixels);

		createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage[matIndex][category], textureImageMemory[matIndex][category]);
		transitionImageLayout(textureImage[matIndex][category], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(stagingBuffer, textureImage[matIndex][category], static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		transitionImageLayout(textureImage[matIndex][category], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
	
	void createTextureImageView(string filename, int matIndex, int category)
	{
		textureImageView[matIndex][category] = createImageView(textureImage[matIndex][category], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	}
	
	// depth
	void createDepthResources() {
		VkFormat depthFormat = findDepthFormat();

		createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}

	VkFormat findDepthFormat() {
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool hasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	// called in run function
	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		if (isHeadless) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

		//glfwSetKeyCallback(window, key_callback);
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void initVulkan() {

		createInstance();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();

		createDescriptorSetLayout();

		createCommandPool();
		createDepthResources();
		createFramebuffers();

		// materials
		pipelineLayout.resize(totalMaterials);
		graphicsPipeline.resize(totalMaterials);

		textureImage.resize(totalMaterials);
		textureImageMemory.resize(totalMaterials);
		textureImageView.resize(totalMaterials);
		textureSampler.resize(totalMaterials);

		// add more sampler
		for (int i = 0; i < totalMaterials; i++) {
			cout << "\nCreating pipeline and sampler for material " << i;
			createGraphicsPipeline(i);
			createCubeMapTextures(i);
			// normal, displacement, albedo (lambertian, pbr), roughness (pbr), metalness (pbr)
			
		}

		vertexBuffer.resize(totalObjects);
		vertexBufferMemory.resize(totalObjects);
		indexBuffer.resize(totalObjects);
		indexBufferMemory.resize(totalObjects);
		uniformBuffers.resize(totalObjects);
		uniformBuffersMemory.resize(totalObjects);
		uniformBuffersMapped.resize(totalObjects);

		sunlightBuffers.resize(totalObjects);
		sunlightBuffersMemory.resize(totalObjects);
		sunlightBuffersMapped.resize(totalObjects);

		spotlightBuffers.resize(totalObjects);
		spotlightBuffersMemory.resize(totalObjects);
		spotlightBuffersMapped.resize(totalObjects);

		spherelightBuffers.resize(totalObjects);
		spherelightBuffersMemory.resize(totalObjects);
		spherelightBuffersMapped.resize(totalObjects);

		cout << "\nCreating descriptor pool";
		createDescriptorPool();

		cout << "\nCreating buffers";

		for (int obj = 0; obj < totalObjects; obj++) {
			createVertexBuffer(obj);
			createIndexBuffer(obj);
			createUniformBuffers(obj);
			createLightBuffers(obj);
		}

		cout << "\nCreating descriptor sets.";

		createDescriptorSets();

		cout << "\nAlmost done..";

		createCommandBuffer();
		createSyncObjects();

		cout << "\nVulkan Successfully Initialized.";
	}

	void mainLoop() {
		

		while (!glfwWindowShouldClose(window)) {

			executeKeyboardEvents();		
			glfwPollEvents();
			drawFrame();

		}
		

		vkDeviceWaitIdle(device);
	}

	void mainLoopHeadless() {

		// Open the file
		ifstream file(FOLDER + eventsFile);

		// Check if the file is opened successfully
		if (!file.is_open()) {
			std::cerr << "\nFailed to open the events file.\n" << std::endl;
			return;
		}

		string line;
		float frame = 0;
		int p1 = 0;
		int p2 = 0;
		string action = "";
		try {
			while (std::getline(file, line)) {
				//std::cout << line << std::endl;
				p1 = line.find(' ');
				frame = stof(line.substr(0, p1));
				action = line.substr(p1+1, 4);
				if (action == "MARK") cout << "\n" << line.substr(p1+5);
				else if (action == "AVAI") {
					glfwPollEvents();
					currentFrameIndex = static_cast<uint32_t>(static_cast<int>(floor((frame / 1000000.f) * FPSi)) % totalFrames);
					drawFrame();
				}
				else if (action == "SAVE") {
					p1 += 6;
					string fileName = line.substr(p1);
					saveImg(fileName);
				}
				else if (action == "PLAY") {
					p1 += 6;
					p2 = line.find(' ', p1);
					frame = stof(line.substr(p1, p2-p1));
					if (stoi(line.substr(p2 + 1, 1)) == 0) {
						FPS = 0;
						FPSi = 0.f;
					}else if(stoi(line.substr(p2 + 1, 1)) == 1){
						FPS = 60;
						FPSi = 60.f;
					}
					else {
						cout << "\nInvalid PLAY mode encountered in events file.\n";
					}

				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "\nError occured while executing the events file.\n"<< std::endl;
		}
		// Close the file
		file.close();
		vkDeviceWaitIdle(device);

	}

	void cleanup() {
		cleanupSwapChain();

		for (size_t i = 0; i < totalMaterials; i++) {
			for (size_t j = 0; j < TEXTURE_COUNT; j++) {
				vkDestroySampler(device, textureSampler[i][j], nullptr);
				vkDestroyImageView(device, textureImageView[i][j], nullptr);

				vkDestroyImage(device, textureImage[i][j], nullptr);
				vkFreeMemory(device, textureImageMemory[i][j], nullptr);
			}
		}
		
		for (size_t obj = 0; obj < totalObjects; obj++) {
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

				vkDestroyBuffer(device, uniformBuffers[obj][i], nullptr);
				vkFreeMemory(device, uniformBuffersMemory[obj][i], nullptr);

				for (size_t j = 0; j < MAX_LIGHT; j++) {
					vkDestroyBuffer(device, spotlightBuffers[obj][i][j], nullptr);
					vkFreeMemory(device, spotlightBuffersMemory[obj][i][j], nullptr);
					vkDestroyBuffer(device, spherelightBuffers[obj][i][j], nullptr);
					vkFreeMemory(device, spherelightBuffersMemory[obj][i][j], nullptr);
				}
				for (size_t j = 0; j < MAX_SUN_LIGHT; j++) {
					vkDestroyBuffer(device, sunlightBuffers[obj][i][j], nullptr);
					vkFreeMemory(device, sunlightBuffersMemory[obj][i][j], nullptr);
				}

				
			}
		}

		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		for (size_t obj = 0; obj < totalObjects; obj++) {
			vkDestroyBuffer(device, vertexBuffer[obj], nullptr);
			vkFreeMemory(device, vertexBufferMemory[obj], nullptr);
	 
			vkDestroyBuffer(device, indexBuffer[obj], nullptr);
			vkFreeMemory(device, indexBufferMemory[obj], nullptr);
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(device, commandPool, nullptr);

		for (size_t i = 0; i < totalMaterials; i++) {
			vkDestroyPipeline(device, graphicsPipeline[i], nullptr);
			vkDestroyPipelineLayout(device, pipelineLayout[i], nullptr);
		}
		vkDestroyRenderPass(device, renderPass, nullptr);

		vkDestroyDevice(device, nullptr);
		glfwDestroyWindow(window);

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwTerminate();
	}

};

int main(int argc, char* argv[]) {
	HelloTriangleApplication app;

	// parsing arguments in the main function ..
	bool validArgs = false;
	
	for (int i = 1; i < argc; i++) { 
		string argument = argv[i];
		if (argument == "--scene") { // scene name
			try {
				app.s72filepath = "s72-main/examples/" + string(argv[i + 1]);
				cout << app.s72filepath;
			}
			catch (const std::exception& e) {
				std::cerr << e.what() << std::endl;
				return EXIT_FAILURE;
			}
			validArgs = true;
			i++;
		}
		else if (argument == "--camera") {// camera name
			try {
				app.PreferredCamera = string(argv[i + 1]);
			}
			catch (const std::exception& e) {
				std::cerr << "invalid argument." << std::endl;
				return EXIT_FAILURE;
			}
			i++;
		}
		else if (argument == "--culling") {// culling
			try {
				if (string(argv[i + 1]) == "none") app.isCulling = false; // default
				if (string(argv[i + 1]) == "frustum") app.isCulling = true;
			}
			catch (const std::exception& e) {
				std::cerr << "invalid argument." << std::endl;
				return EXIT_FAILURE;
			}
			i++;
		}
		else if (argument == "--headless") {// culling
			app.isHeadless = true;
			app.eventsFile = string(argv[i + 1]);
			i++;
		}
		else if (argument == "--drawing-size") {// drawing size
			try {
				adjustDimension = false;
				HEIGHT = min(16000, stoi((string(argv[i + 2]))));
				WIDTH = min(16000,stoi((string(argv[i + 1]))));
			}
			catch (const std::exception& e) {
				std::cerr << "invalid argument." << std::endl;
				return EXIT_FAILURE;
			}
			i += 2;
		}
		else if (argument == "--cube") { // cube
			try {
				string inFile = string(argv[i + 1]);
				string mode = string(argv[i + 2]);
				if (mode == "--lambertian") {
					makeLambertianCubeMap(inFile);
				}
				else if (mode == "--gxx") {
					float roughness = 0.f;
					for (int r = 0; r < 6; r++) {
						makePBRCubeMap(inFile, roughness, r);
						roughness += 0.2f;
					}
				}
				else {
					cout << "\n please enter valid cube map generation mode.";
				}
			}
			catch (const std::exception& e) {
				std::cerr << "invalid argument." << std::endl;
				return EXIT_FAILURE;
			}
			i += 2;
		}
		else {
			std::cerr << "invalid argument." << std::endl;
			return EXIT_FAILURE;
		}
	}

	//if (!validArgs) {
	//	std::cerr << "invalid argument." << std::endl;
	//	return EXIT_FAILURE;
	//}

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}