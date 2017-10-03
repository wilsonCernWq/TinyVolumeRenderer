#include "volume_reader.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#ifdef USE_GLM
# include <glm/glm.hpp>
#else
# error "GLM is required here"
#endif
#include <fstream>
#include <string>
#include <cassert>
#include <algorithm>

using namespace rapidjson;
using vec3i = glm::ivec3;
using vec3f = glm::vec3;

namespace VolumeInfo {
  static std::string name;
  static int         type;
  static vec3i size;
  static vec3f spacing;
  static std::string fname;
};

int ConvertType(const std::string& type)
{
  if      (type.compare("uchar"   ) == 0) { return UCHAR;    }
  else if (type.compare("char"    ) == 0) { return CHAR;     }
  else if (type.compare("uint8"   ) == 0) { return UINT8;    }
  else if (type.compare("uint16"  ) == 0) { return UINT16;   }
  else if (type.compare("uint32"  ) == 0) { return UINT32;   }
  else if (type.compare("uint64"  ) == 0) { return UINT64;   }
  else if (type.compare("int8"    ) == 0) { return INT8;     }
  else if (type.compare("int16"   ) == 0) { return INT16;    }
  else if (type.compare("int32"   ) == 0) { return INT32;    }
  else if (type.compare("int64"   ) == 0) { return INT64;    }
  else if (type.compare("float32" ) == 0) { return FLOAT32;  }
  else if (type.compare("double64") == 0) { return DOUBLE64; }
  else {}
}

std::string ParsePath(const std::string& str)
{
  std::string cstr = str;
#if						\
  defined(WIN32)  ||				\
  defined(_WIN32) ||				\
  defined(__WIN32) &&				\
  !defined(__CYGWIN__)
  std::replace(cstr.begin(), cstr.end(), '/', '\\');
#else
  std::replace(cstr.begin(), cstr.end(), '\\', '/');
#endif
  return cstr;
}

std::string ParseURL(const std::string& str)
{
  std::string fname;
  std::string fpath = ParsePath(str);
  size_t p = fpath.find_last_of("/\\");
  if (p != std::string::npos) {
    auto dpath = fpath.substr(0, p + 1);
    fname = fpath.substr(p + 1, fpath.size()-dpath.size());
  }
  else {
    fname = fpath;
  }
  return fname;
}

vec3f ReadVec3(const Value& a)
{
  assert(a.IsArray());
  assert(a.Size() == 3);
  return vec3f(a[0].GetFloat(),a[1].GetFloat(),a[2].GetFloat());
}

bool ParseJSON(const std::string& fname)
{
  using namespace std;
  // read file
  ifstream in(fname.c_str());
  if (!in.is_open()) {
    fprintf(stderr, "Error: cannot open file %s\n", fname.c_str());
    return false;
  }
  string contents((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());  
  // parse into JSON
  Document d; d.Parse(contents.c_str());
  fprintf(stdout, "[json] successful loading %s\n", fname.c_str());
  // read JSON
  VolumeInfo::name    = d["name"].GetString();
  VolumeInfo::type    = ConvertType(d["type"].GetString());
  VolumeInfo::size    = ReadVec3(d["size"]);
  VolumeInfo::spacing = ReadVec3(d["spacing"]);
  VolumeInfo::fname   = ParseURL(d["url"].GetString());
  fprintf(stdout, "[json] volume information\n");
  fprintf(stdout, "[json]     name: %s\n", VolumeInfo::name.c_str());
  fprintf(stdout, "[json]     type: %i\n", VolumeInfo::type);
  fprintf(stdout, "[json]     size: %i, %i, %i\n",
	  VolumeInfo::size.x, VolumeInfo::size.y, VolumeInfo::size.z);
  fprintf(stdout, "[json]     spacing: %f, %f, %f\n",
	  VolumeInfo::spacing.x, VolumeInfo::spacing.y, VolumeInfo::spacing.z);
  fprintf(stdout, "[json]     data file: %s\n", VolumeInfo::fname.c_str());
  return true;
}

bool ParseRaw(void*& data_ptr, int& data_size)
{
  // open file
  std::ifstream in(VolumeInfo::fname.c_str(), std::ios::in | std::ios::binary);
  if (!in.is_open()) {
    fprintf(stderr, "Error: cannot open file %s\n", VolumeInfo::fname.c_str());
    return false;
  }
  if (data_ptr != NULL) {
    fprintf(stderr, "Error: Data buffer is not empty\n");
    return false;
  }
  // allocate data buffer
  const int unit_size = SizeOf(VolumeInfo::type);
  data_size = VolumeInfo::size.x * VolumeInfo::size.y * VolumeInfo::size.z * unit_size;
  data_ptr = new char[data_size];  
  in.read((char*)data_ptr, data_size);
  return true;
}

bool ReadVolume
(const char* fname, int& data_type, int& data_size,
 int& data_X, int& data_Y, int& data_Z, void*& data_ptr)
{
  if (!ParseJSON(fname)) { return false; };
  if (!ParseRaw(data_ptr, data_size)) { return false; };
  data_type = VolumeInfo::type;
  data_X = VolumeInfo::size.x;
  data_Y = VolumeInfo::size.y;
  data_Z = VolumeInfo::size.z;
  return true;
}
