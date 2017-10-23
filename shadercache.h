#ifndef SHADERCACHE_H
#define SHADERCACHE_H

#include "common.h"
#include "shader.h"

#include <vulkan/vulkan.h>
#include <unordered_map>
#include <string>

class ShaderCache
{
private:
	typedef std::unordered_map<std::string, Shader*> map_type;

public:
	ShaderCache( Renderer& renderer );
	~ShaderCache();

	void destroy();

	Shader& getVertexShader( const std::string& name );
	Shader& getFragmentShader( const std::string& name );

private:
	static void readFile( const std::string& filename, std::vector<char>& code );

	void    cleanupMap( map_type& map );

	Shader& getShader( const std::string& name, VkShaderStageFlagBits stage, map_type& map );

private:
	Renderer* m_pRenderer;

	map_type  m_VertexShaders;
	map_type  m_FragmentShaders;
};

#endif // SHADERCACHE_H
