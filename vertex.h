#ifndef VERTEX_H
#define VERTEX_H

#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Vertex
{
public:
	glm::vec2 pos;
	glm::vec3 color;

public:
	struct AttributeDesc
	{
		VkFormat format;
		uint32_t offset;
	};

public:
	static const std::vector<AttributeDesc>& getAttributeDescriptions()
	{
		static const std::vector<AttributeDesc> desc = {
		    { VK_FORMAT_R32G32_SFLOAT,    offsetof( Vertex, pos ) },
		    { VK_FORMAT_R32G32B32_SFLOAT, offsetof( Vertex, color ) }
		};
		return desc;
	}
};

#endif // VERTEX_H
