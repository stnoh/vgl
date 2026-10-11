/******************************************************************************
Helper for raw file (.raw) I/O
This is temporary code. I have a plan to re-write or dispose this code soon.
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef RAW_FILE_IO
#define RAW_FILE_IO

#include <vector>
#include <glm/glm.hpp>

namespace vgl {

// read raw file
void ReadRawFile(const char* filepath, glm::ivec3& size, std::vector<glm::uint8>& data);

}

#endif
