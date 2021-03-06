//**********************************************
//Singleton Texture Manager class
//Written by Ben English
//benjamin.english@oit.edu
//
//For use with OpenGL and the FreeImage library
//**********************************************

#include "TextureManager.h"

TextureManager* TextureManager::m_inst(0);

TextureManager* TextureManager::Inst()
{
	if(!m_inst)
		m_inst = new TextureManager();

	return m_inst;
}

TextureManager::TextureManager()
{
	// call this ONLY when linking with FreeImage as a static library
	#ifdef FREEIMAGE_LIB
		FreeImage_Initialise();
	#endif

}
bool TextureManager::CreateTextureAtlas()
{

	atlas_width += 2;
	atlas_height += 2;
	m_textureAtlas = texture_atlas_new(atlas_width, atlas_height, 4);

	for (auto const& file : textures)
	{
		std::string filename = file.first;

		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
		//pointer to the image, once loaded
		FIBITMAP *dib(0);
		//pointer to the image data
		BYTE* bits(0);

		fif = FreeImage_GetFileType(filename.c_str(), 0);
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(filename.c_str());

		if (fif == FIF_UNKNOWN)
			return false;

		if (FreeImage_FIFSupportsReading(fif))
			dib = FreeImage_Load(fif, filename.c_str());

		if (!dib)
			return false;

		//retrieve the image data
		bits = FreeImage_GetBits(dib);

		int pitch = FreeImage_GetPitch(dib);
		//get the image width and height
		int	width = FreeImage_GetWidth(dib);
		int height = FreeImage_GetHeight(dib);
		int k = FreeImage_GetBPP(dib);

		if (bits == 0 || (width == 0) || (height == 0))
			return false;

		ivec4 coords = texture_atlas_get_region(m_textureAtlas, width, height);

		textures[filename] = Vec4f(coords.x , coords.y , width, height);


		texture_atlas_set_region(m_textureAtlas, coords.x, coords.y, width, height, bits, pitch);


	
		FreeImage_Unload(dib);
	}

	glGenTextures(1, &m_textureAtlas->id);
	glBindTexture(GL_TEXTURE_2D, m_textureAtlas->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas_width, atlas_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, m_textureAtlas->data);
	

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, 0);

}

ftgl::texture_font_t* TextureManager::LoadFont(const char * filename)
{
	
	ftgl::texture_font_t* m_font;

	
	m_fontAtlas = texture_atlas_new(512, 512, 1);
	m_font = texture_font_new_from_file(m_fontAtlas, 20, filename);
	texture_font_load_glyphs(m_font, " !\"#$%&'()*+,-./0123456789:;<=>?"
		"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
		"`abcdefghijklmnopqrstuvwxyz{|}~");

	glGenTextures(1, &m_fontAtlas->id);
	glBindTexture(GL_TEXTURE_2D, m_fontAtlas->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_fontAtlas->width, m_fontAtlas->height,
		0, GL_RED, GL_UNSIGNED_BYTE, m_fontAtlas->data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return m_font;
}



//these should never be called
//TextureManager::TextureManager(const TextureManager& tm){}
//TextureManager& TextureManager::operator=(const TextureManager& tm){}
	
TextureManager::~TextureManager()
{
	// call this ONLY when linking with FreeImage as a static library
	#ifdef FREEIMAGE_LIB
		FreeImage_DeInitialise();
	#endif

	UnloadAllTextures();
	m_inst = 0;
}

int TextureManager::LoadTexture(const char* filename, int& width, int& height)
{
	int id = generateId();
	if (LoadTexture(filename, id,width, height))
	{
		return id;
	}
	return -1;
}
bool TextureManager::AddTexture(const char* filename )
{

	if (textures.find(filename) != textures.end())
		return false;

	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	//pointer to the image data
	BYTE* bits(0);
	//image width and height

	//OpenGL's image ID to map to
	GLuint gl_texID;

	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(filename, 0);
	//if still unknown, try to guess the file format from the file extension
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(filename);
	//if still unkown, return failure
	if (fif == FIF_UNKNOWN)
		return false;

	//check that the plugin has reading capabilities and load the file
	if (FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, filename);
	//if the image failed to load, return failure
	if (!dib)
		return false;

	int width = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);
	
	//if this somehow one of these failed (they shouldn't), return failure
	if ((width == 0) || (height == 0))
		return false;

	
	textures[filename] = Vec4f(-1, -1, width, height);
	atlas_width += width;
	atlas_height +=height;


	FreeImage_Unload(dib);
	return true;
}
bool TextureManager::LoadTexture(const char* filename,  const unsigned int texID, int& width, int& height, GLenum image_format, GLint internal_format, GLint level, GLint border)
{
	
	
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	//pointer to the image data
	BYTE* bits(0);
	//image width and height
	
	//OpenGL's image ID to map to
	GLuint gl_texID;

	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(filename, 0);
	//if still unknown, try to guess the file format from the file extension
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(filename);
	//if still unkown, return failure
	if (fif == FIF_UNKNOWN)
		return false;

	//check that the plugin has reading capabilities and load the file
	if (FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, filename);
	//if the image failed to load, return failure
	if (!dib)
		return false;
	//retrieve the image data
	bits = FreeImage_GetBits(dib);
	//get the image width and height
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	//if this somehow one of these failed (they shouldn't), return failure
	if ((bits == 0) || (width == 0) || (height == 0))
		return false;

	//if this texture ID is in use, unload the current texture
	if (m_texID.find(texID) != m_texID.end())
		glDeleteTextures(1, &(m_texID[texID]));

	//generate an OpenGL texture ID for this texture
	glGenTextures(1, &gl_texID);
	//store the texture ID mapping
	m_texID[texID] = gl_texID;
	//bind to the new texture ID
	glBindTexture(GL_TEXTURE_2D, gl_texID);
	//store the texture data for OpenGL use
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glTexImage2D(GL_TEXTURE_2D, level, internal_format, width, height,
		border, image_format, GL_UNSIGNED_BYTE, bits);
   // glGenerateMipmap(GL_TEXTURE_2D);

	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Free FreeImage's copy of the data
	FreeImage_Unload(dib);
	glBindTexture(GL_TEXTURE_2D, 0);
	//return success
	return true;
}



bool TextureManager::UnloadTexture(const unsigned int texID)
{
	bool result(true);
	//if this texture ID mapped, unload it's texture, and remove it from the map
	if(m_texID.find(texID) != m_texID.end())
	{
		glDeleteTextures(1, &(m_texID[texID]));
		m_texID.erase(texID);
	}
	//otherwise, unload failed
	else
	{
		result = false;
	}

	return result;
}

void TextureManager::BindTextures()
{
	if (m_textureAtlas!=NULL)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureAtlas->id);
	}
	if (m_fontAtlas!=NULL)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_fontAtlas->id);
	}
		

}
void TextureManager::UnBindTextures()
{

	glBindTexture(GL_TEXTURE_2D, 0);
	

	
}
void TextureManager::UnloadAllTextures()
{
	//start at the begginning of the texture map
	std::map<unsigned int, GLuint>::iterator i = m_texID.begin();

	//Unload the textures untill the end of the texture map is found
	while(i != m_texID.end())
		UnloadTexture(i->first);

	//clear the texture map
	m_texID.clear();
}