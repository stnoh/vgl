#include <stdio.h>
#include <glm/glm.hpp>

namespace vgl {

void ShowMatrix4x4(glm::mat4 mat) {
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			printf("%+7.4f ", mat[i][j]); // [CAUTION] row and column for OpenGL
		}
		printf("\n");
	}
	printf("\n");
}

}
