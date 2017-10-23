#include "shadercache.h"

#include <fstream>

ShaderCache::ShaderCache( Renderer& renderer )
    : m_pRenderer( &renderer ),
      m_VertexShaders(),
      m_FragmentShaders()
{
}

ShaderCache::~ShaderCache()
{
	destroy();
}

void ShaderCache::destroy()
{
	cleanupMap( m_VertexShaders );
	cleanupMap( m_FragmentShaders );
}

Shader& ShaderCache::getVertexShader( const std::string& name )
{
	return getShader( name, VK_SHADER_STAGE_VERTEX_BIT, m_VertexShaders );
}

Shader& ShaderCache::getFragmentShader( const std::string& name )
{
	return getShader( name, VK_SHADER_STAGE_FRAGMENT_BIT, m_FragmentShaders );
}

void ShaderCache::readFile( const std::string& filename, std::vector<char>& code )
{
	std::ifstream file( filename, std::ios::binary | std::ios::ate );

	if( !file.is_open() )
	{
		log_error( "Cannot open file: " + filename );
		return;
	}

	size_t fileSize = file.tellg();

	code.resize( fileSize );

	file.seekg( 0 );
	file.read( code.data(), fileSize );
	file.close();
}

void ShaderCache::cleanupMap( map_type& map )
{
	for( auto& pair : map )
	{
		delete pair.second;
	}
	map.clear();
}

Shader& ShaderCache::getShader( const std::string& name,
                                VkShaderStageFlagBits stage,
                                map_type& map )
{
	auto iter = map.find( name );
	if( iter != map.end() )
	{
		return *( iter->second );
	}
	else
	{
		log_info( "Caching shader: " + name );

		std::vector<char> code;
		readFile( name + ".spv", code );

		Shader* shader = new Shader( *m_pRenderer, code, "main", stage ); // TODO: variable entry func?
		map[ name ] = shader;

		return *shader;
	}
}
