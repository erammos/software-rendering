#include "SpriteRenderer.h"
#include "SpriteComponent.h"
#include "TextureManager.h"

void Engine::Graphics::SpriteRenderer::init()
{
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ibo);

	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER,m_vbo );
	glBufferData(GL_ARRAY_BUFFER, TOTAL_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(SHADER_VERTEX_ATTRIB);
	glEnableVertexAttribArray(SHADER_COLOR_ATTRIB);
	glEnableVertexAttribArray(SHADER_TEX_ATTRIB);
	glVertexAttribPointer(SHADER_VERTEX_ATTRIB, 4, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (const GLvoid *)0);
	glVertexAttribPointer(SHADER_COLOR_ATTRIB, 4, GL_FLOAT, GL_FALSE, VERTEX_SIZE,(const GLvoid *)(offsetof(VertexData,VertexData::color)));
	glVertexAttribPointer(SHADER_TEX_ATTRIB, 4, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (const GLvoid *)(offsetof(VertexData, VertexData::tex)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLushort indices[TOTAL_INDICES_SIZE];
	int offset = 0;
	for (int i = 0; i < TOTAL_INDICES_SIZE; i += 6)
	{
		indices[i + 0] = offset + 0;
		indices[i + 1] = offset + 1;
		indices[i + 2] = offset + 2;

		indices[i + 3] = offset + 2;
		indices[i + 4] = offset + 3;
		indices[i + 5] = offset + 0;

		offset += 4;
	}
	
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, TOTAL_INDICES_SIZE * sizeof(GLushort), indices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	



}

Engine::Graphics::SpriteRenderer::SpriteRenderer()
{
	init();
}

Engine::Graphics::SpriteRenderer::~SpriteRenderer()
{
	
	glDeleteBuffers(1, &m_ibo);
	glDeleteBuffers(1, &m_vbo);
}


void Engine::Graphics::SpriteRenderer::DrawText(texture_font_t * font,
	char * text, Vec3f * color, Vec2f * pen)
{
	size_t i;
	float r = color->x, g = color->y, b = color->z, a = color->w;

	for (i = 0; i < strlen(text); ++i)
	{
		texture_glyph_t *glyph = texture_font_get_glyph(font, text + i);
		if (glyph != NULL)
		{
			float kerning = 0.0f;
			if (i > 0)
			{
				kerning = texture_glyph_get_kerning(glyph, text + i - 1);
			}
			pen->x += kerning;

			float x0 = (pen->x + glyph->offset_x);
			float y0 = (pen->y + glyph->offset_y);
			float x1 = (x0 + glyph->width);
			float y1 = (y0 - glyph->height);

			float s0 = glyph->s0;
			float t0 = glyph->t0;
			float s1 = glyph->s1;
			float t1 = glyph->t1;

			GLuint indices[6] = { 0,1,2, 0,2,3 };
			//vertex_t vertices[4] = 
			//{ { x0,y0,0,  s0,t0,  r,g,b,a },
			//{ x0,y1,0,  s0,t1,  r,g,b,a },
			//{ x1,y1,0,  s1,t1,  r,g,b,a },
			//{ x1,y0,0,  s1,t0,  r,g,b,a } };
			//vertex_buffer_push_back(buffer, vertices, 4, indices, 6);
			pen->x += glyph->advance_x;


			m_buffer->vertex = Vec3f(x0, y0, 0);

			m_buffer->color = Color(r, g, b, a);
			m_buffer->tex = Vec3f(s0, t0, 0);

			m_buffer++;
			m_buffer->vertex = Vec3f(x0, y1, 0);
			m_buffer->color = Color(r, g, b, a);
			m_buffer->tex = Vec3f(s0, t1, 0);

			m_buffer++;
			m_buffer->vertex = Vec3f(x1, y1, 0);
			m_buffer->color = Color(r, g, b, a);
			m_buffer->tex = Vec3f(s1, t1, 0);

			m_buffer++;
			m_buffer->vertex = Vec3f(x1, y0, 0);
			m_buffer->color = Color(r, g, b, a);
			m_buffer->tex = Vec3f(s1, t0, 0);

			m_buffer++;


			m_indexCount += 6;

		}
	}
}


void Engine::Graphics::SpriteRenderer::Begin()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	m_buffer = (VertexData *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	glActiveTexture(GL_TEXTURE0);
	TextureManager::Inst()->BindTexture(0);
	
}

void Engine::Graphics::SpriteRenderer::Draw(GraphicsComponent* renderable)
{
	SpriteComponent * sprite = (SpriteComponent *)renderable;

	Color color = sprite->GetColor();
	Rect rect = sprite->GetRect();
	

	VertexData * p = &rect.bl;
	for (int i = 0; i < 4; i++)
	{
 		m_buffer->vertex = p->vertex;
		m_buffer->color = color;
		m_buffer->tex = p->tex;
		p++;
		m_buffer++;

	}
	m_indexCount += 6;
}
void Engine::Graphics::SpriteRenderer::End()
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_SHORT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	m_indexCount = 0;
}

void Engine::Graphics::SpriteRenderer::SetShaderId(GLuint shader_id)
{
	this->shader_id = shader_id;

	int v[32];
	for (int i = 0; i < 31; i++)
	{
		v[i] = i;
	}
	glUniform1iv(glGetUniformLocation(shader_id, "tex"), 32, v);
	
}

void Engine::Graphics::SpriteRenderer::SetModelViewMatrix(Engine::Math::Mat4f& matrix)
{
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "model_matrix"), 1, GL_FALSE, matrix.values);

}

void Engine::Graphics::SpriteRenderer::SetProjectionMatrix(Engine::Math::Mat4f& matrix)
{
	glUniformMatrix4fv(glGetUniformLocation(shader_id, "proj_matrix"), 1, GL_FALSE, matrix.values);

}

	

