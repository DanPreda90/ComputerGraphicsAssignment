#include "Quad.h"

void initQuad(Quad& quad) {
	quad.programID = LoadShadersFromString(quad.vertex_shader, quad.fragment_shader);
	glGenVertexArrays(1, &quad.vao);
	glBindVertexArray(quad.vao);
	glEnableVertexAttribArray(0);
	// Create a vertex buffer object to store the vertex data		
	glGenBuffers(1, &quad.vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, quad.vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad.vertex_buffer_data), quad.vertex_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Create a vertex buffer object to store the color data
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &quad.colorBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, quad.colorBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad.color_buffer_data), quad.color_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &quad.indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad.indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad.index_buffer_data), quad.index_buffer_data, GL_STATIC_DRAW);
	glBindVertexArray(0);

	quad.MVPID = glGetUniformLocation(quad.programID, "MVP");
}
void renderQuad(Quad& quad, glm::mat4 vp) {
	glm::mat4 m = glm::translate(glm::mat4(1.0f), quad.position);
	m = glm::scale(m, quad.scale);
	
	glm::mat4 mvp = vp * m;
	glUseProgram(quad.programID);
	glBindVertexArray(quad.vao);
	glUniformMatrix4fv(quad.MVPID, 1, GL_FALSE, &mvp[0][0]);
	glDrawElements(
		GL_TRIANGLES,
		sizeof(quad.index_buffer_data),
		GL_UNSIGNED_INT,
		(void*)0
	);
	glBindVertexArray(0);
}