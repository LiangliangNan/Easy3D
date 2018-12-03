#include "primitives.h"


namespace OpenGL {


	void draw_quad(GLuint positionAttrib, int x, int y, int w, int h, int vpw, int vph, float depth)
	{
		// vertex positions in NDC (Normalized Device Coordinates)
		// I assume viewportX = 0 and viewportY = 0.  Otherwise use the following equation to 
		// convert from screen coordinates to NDC.
		//Xndc = 2.0f * (x - viewportX) / vpw - 1.0f;			
		//Yndc = 2.0f * (y - viewportY) / vph - 1.0f;
		const float minX = 2.0f * x / vpw - 1.0f;
		const float minY = 2.0f * y / vph - 1.0f;
		const float maxX = 2.0f * (x + w) / vpw - 1.0f;
		const float maxY = 2.0f * (y + h) / vph - 1.0f;

		const float positions[] = {
			minX, minY, depth,
			maxX, minY, depth,
			minX, maxY, depth,
			maxX, maxY, depth
		};

		glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, false, 3 * sizeof(float), positions);
		glEnableVertexAttribArray(positionAttrib);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(positionAttrib);
	}


	void draw_quad_wire(GLuint positionAttrib, int x, int y, int w, int h, int vpw, int vph, float depth)
	{
		// vertex positions in NDC (Normalized Device Coordinates)
		// I assume viewportX = 0 and viewportY = 0.  Otherwise use the following equation to 
		// convert from screen coordinates to NDC.
		//Xndc = 2.0f * (x - viewportX) / vpw - 1.0f;			
		//Yndc = 2.0f * (y - viewportY) / vph - 1.0f;
		const float minX = 2.0f * x / vpw - 1.0f;
		const float minY = 2.0f * y / vph - 1.0f;
		const float maxX = 2.0f * (x + w) / vpw - 1.0f;
		const float maxY = 2.0f * (y + h) / vph - 1.0f;

		const float positions[] = {
			minX, minY, depth,
			maxX, minY, depth,
			maxX, maxY, depth,
			minX, maxY, depth
		};

		glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, false, 3 * sizeof(float), positions);
		glEnableVertexAttribArray(positionAttrib);

		glDrawArrays(GL_LINE_LOOP, 0, 4);

		glDisableVertexAttribArray(positionAttrib);
	}


	void draw_quad(GLuint positionAttrib, GLuint texcoordAttrib, int x, int y, int w, int h, int vpw, int vph, float depth)
	{
		// vertex positions in NDC (Normalized Device Coordinates)
		// I assume viewportX = 0 and viewportY = 0.  Otherwise use the following equation to 
		// convert from screen coordinates to NDC.
		//Xndc = 2.0f * (x - viewportX) / vpw - 1.0f;			
		//Yndc = 2.0f * (y - viewportY) / vph - 1.0f;
		const float minX = 2.0f * x / vpw - 1.0f;
		const float minY = 2.0f * y / vph - 1.0f;
		const float maxX = 2.0f * (x + w) / vpw - 1.0f;
		const float maxY = 2.0f * (y + h) / vph - 1.0f;

		const float maxYTexCoord = (h == 0) ? 0.0f : 1.0f;

		const float positions[] = {
			minX, minY, depth, 0, 0,
			maxX, minY, depth, 1.0, 0,
			minX, maxY, depth, 0, maxYTexCoord,
			maxX, maxY, depth, 1.0, maxYTexCoord
		};

		glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), positions);
		glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), positions + 3);

		glEnableVertexAttribArray(positionAttrib);
		glEnableVertexAttribArray(texcoordAttrib);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(positionAttrib);
		glDisableVertexAttribArray(texcoordAttrib);
	}


	void draw_full_screen_quad(GLuint positionAttrib, float depth)
	{
		// vertex positions in NDC (Normalized Device Coordinates)
		static const float positions[] = {
			-1.0f, -1.0f, depth,
			1.0f, -1.0f, depth,
			-1.0f, 1.0f, depth,
			1.0f, 1.0f, depth
		};

		glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, false, 3* sizeof(float), positions);
		glEnableVertexAttribArray(positionAttrib);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(positionAttrib);
	}


	void draw_full_screen_quad(GLuint positionAttrib, GLuint texcoordAttrib, float depth)
	{
		// vertex positions in NDC (Normalized Device Coordinates), then tex-coords
		const float positions[] =	{
			-1.0f, -1.0f, depth, 0.0f, 0.0f,
			1.0f, -1.0f, depth, 1.0f, 0.0f,
			-1.0f, 1.0f, depth, 0.0f, 1.0f,
			1.0f, 1.0f, depth, 1.0f, 1.0f	
		};

		glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), positions);
		glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), positions + 3);

		glEnableVertexAttribArray(positionAttrib);
		glEnableVertexAttribArray(texcoordAttrib);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableVertexAttribArray(positionAttrib);
		glDisableVertexAttribArray(texcoordAttrib);
	}


	void draw_cube(GLuint positionAttrib)
	{
		const float positions[] = {
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
		};

		const GLushort indices[] = {
			0, 1, 2, 2, 1, 3,   // f
			4, 6, 5, 5, 6, 7,   // b
			4, 0, 6, 6, 0, 2,   // l
			1, 5, 3, 3, 5, 7,   // r
			2, 3, 6, 6, 3, 7,   // t
			4, 5, 0, 0, 5, 1,   // b
		};

		glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, false, 3 * sizeof(float), positions);
		glEnableVertexAttribArray(positionAttrib);

		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLushort), GL_UNSIGNED_SHORT, indices);

		glDisableVertexAttribArray(positionAttrib);

		//checkGlError("drawCube");
	}

	void draw_cube_wire(GLuint positionAttrib)
	{
		const float positions[] = {
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,
		};

		const GLushort indices[] = {
			0, 1, 1, 2, 2, 3, 3, 0,    // f
			4, 5, 5, 6, 6, 7, 7, 4, // b
			0, 4, 1, 5, 2, 6, 3, 7
		};

		glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, false, 3 * sizeof(float), positions);
		glEnableVertexAttribArray(positionAttrib);

		glDrawElements(GL_LINES, sizeof(indices) / sizeof(GLushort), GL_UNSIGNED_SHORT, indices);

		glDisableVertexAttribArray(positionAttrib);
	}

	void draw_point(GLuint positionAttrib, const vec3 &pos)
	{
		glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, false, 3 * sizeof(float), pos);
		glEnableVertexAttribArray(positionAttrib);

		glDrawArrays(GL_POINTS, 0, 1);

		glDisableVertexAttribArray(positionAttrib);
	}

	// ----------------------------------------------------
}