/******************************************************************************
Helper for raw file (.raw) I/O
This is temporary code. I have a plan to re-write or dispose this code soon.
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <vgl/rawFileIO.h>
#include <fstream>
#include <sstream>

namespace vgl {

///////////////////////////////////////////////////////////////////////////////
// public method : read ply file
///////////////////////////////////////////////////////////////////////////////
void ReadRawFile(const char* filepath, 
	glm::ivec3& size, std::vector<glm::uint8>& data)
{
	// read header file (.raw.ini)
	{
		std::string filepath_ini = std::string(filepath) + ".ini";
		std::ifstream iFile(filepath_ini);

		std::vector<std::string> tokens;
		std::string line;
		while (std::getline(iFile, line, ':')) {
			std::istringstream iss(line);

			std::string token;
			while (iss >> token) {
				tokens.push_back(token);
			}
		}

		iFile.close();

		for (int i = 0; i < tokens.size(); i += 2) {
			if (0 == strcmp(tokens[i].c_str(), "dimx")) {
				size.x = atoi(tokens[i + 1].c_str());
			}
			else if (0 == strcmp(tokens[i].c_str(), "dimy")) {
				size.y = atoi(tokens[i + 1].c_str());
			}
			else if (0 == strcmp(tokens[i].c_str(), "dimz")) {
				size.z = atoi(tokens[i + 1].c_str());
			}
		}
	}

	// read content file (.raw)
	{
		// set file reader and flags
		std::ifstream iFile(filepath, std::ios::binary);
		iFile.unsetf(std::ios::skipws);

		// resize before reading
		int sz = size.x * size.y * size.z;
		data.resize(sz);
		iFile.read((char*)data.data(), sz * sizeof(glm::uint8));
	}
}

}
