#ifndef JET_VERTEX_H
#define JET_VERTEX_H

#include "../def.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace jet
{
struct Vertex
{
  glm::vec2 position;
  glm::vec3 color;

  static vk::VertexInputBindingDescription getBindingDescription()
  {
    vk::VertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = vk::VertexInputRate::eVertex;

    return bindingDescription;
  }

  static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
  {
    std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions = {};

    // Position
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    // Color
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
  }
};

const std::vector<Vertex> Vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},  {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},  {{-0.2f, -0.2f}, {1.0f, 1.0f, 1.0f}}, {{0.2f, -0.2f}, {1.0f, 1.0f, 1.0f}},
    {{0.2f, 0.2f}, {1.0f, 1.0f, 1.0f}},   {{-0.2f, 0.2f}, {1.0f, 1.0f, 1.0f}},
};

const std::vector<u16> Indices = {0, 1, 2, 3, 0, 0xFFFF, 4, 5, 6, 7, 4};
} // namespace jet

#endif // JET_VERTEX_H
