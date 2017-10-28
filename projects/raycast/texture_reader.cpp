#include "texture_reader.hpp"
#include "volume_reader.hpp"
#include "comm.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLuint loadBMP_custom(const char * imagepath)
{
  fprintf(stdout, "[BMP] Reading image %s\n", imagepath);
  // Data read from the header of the BMP file
  unsigned char header[54];
  unsigned int  dataPos;
  unsigned int  imageSize;
  unsigned int  width, height;
  // Actual RGB data
  unsigned char*data;
  // Open the file
  FILE * file = fopen(imagepath,"rb");
  if (!file){
    fprintf(stdout, "%s could not be opened. Are you in the right directory ? "
	   "Don't forget to read the FAQ !\n", imagepath);
    getchar();
    return 0;
  }
  // Read the header, i.e. the 54 first bytes
  // If less than 54 bytes are read, problem
  if ( fread(header, 1, 54, file)!=54 ){ 
    fprintf(stdout, "Not a correct BMP file\n");
    fclose(file);
    return 0;
  }
  // A BMP files always begins with "BM"
  if ( header[0]!='B' || header[1]!='M' ){
    fprintf(stdout, "Not a correct BMP file\n");
    fclose(file);
    return 0;
  }
  // Make sure this is a 24bpp file
  if ( *(int*)&(header[0x1E])!=0  ) {
    fprintf(stdout, "Not a correct BMP file\n");
    fclose(file);
    return 0;
  }
  if ( *(int*)&(header[0x1C])!=24 ) {
    fprintf(stdout, "Not a correct BMP file\n");
    fclose(file);
    return 0;
  }
  // Read the information about the image
  dataPos    = *(int*)&(header[0x0A]);
  imageSize  = *(int*)&(header[0x22]);
  width      = *(int*)&(header[0x12]);
  height     = *(int*)&(header[0x16]);
  // Some BMP files are misformatted, guess missing information
  // 3 : one byte for each Red, Green and Blue component
  if (imageSize==0)    imageSize=width*height*3; 
  if (dataPos==0)      dataPos=54; // The BMP header is done that way
  // Create a buffer
  data = new unsigned char [imageSize];
  // Read the actual data from the file into the buffer
  fread(data,1,imageSize,file);
  // Everything is in memory now, the file can be closed.
  fclose (file);
  // Create one OpenGL texture
  GLuint textureID;
  glGenTextures(1, &textureID);       
  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_2D, textureID);
  // Give the image to OpenGL
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
  // OpenGL has now copied the data. Free our own version
  delete [] data;
  // Poor filtering, or ...
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
  // ... nice trilinear filtering ...
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  // ... which requires mipmaps. Generate them automatically.
  glGenerateMipmap(GL_TEXTURE_2D);
  // Return the ID of the texture we just created
  return textureID;
}

GLuint loadRAW_custom(const char * volumepath, int& depth)
{
  // read data from disk
  fprintf(stdout, "[texture] start reading volume %s\n", volumepath);  
  int data_type, data_size, data_dim[3];
  void* data_ptr_void = NULL;
  ReadVolume(volumepath, data_type, data_size,
	     data_dim[0], data_dim[1], data_dim[2], data_ptr_void);
  char* data_ptr = (char*)data_ptr_void;
  depth = std::max(data_dim[0], std::max(data_dim[1], data_dim[2]));
  fprintf(stdout, "[texture] done reading data\n");  
  // calculate texture type
  GLenum internal_type;
  fprintf(stdout, "[texture] receving data_type: %i\n", data_type);  
  switch (data_type) {
  case (UCHAR):
    internal_type = GL_UNSIGNED_BYTE;
    break;
  case (CHAR):
    internal_type = GL_BYTE;
    break;
  case (UINT8):
    internal_type = GL_UNSIGNED_BYTE;
    break;
  case (UINT16):
    internal_type = GL_UNSIGNED_SHORT;
    break;
  case (UINT32):
    internal_type = GL_UNSIGNED_INT;
    break;
  case (UINT64):
    fprintf(stderr, "Error: Cannot handle this type UINT64 %i\n", data_type);
    exit(-1);
  case (INT8):
    internal_type = GL_BYTE;
    break;
  case (INT16):
    internal_type = GL_SHORT;
    break;
  case (INT32):
    internal_type = GL_INT;
    break;
  case (INT64):
    fprintf(stderr, "Error: Cannot handle this type INT64 %i\n", data_type);
    exit(-1);
  case (FLOAT16):
    internal_type = GL_HALF_FLOAT;
    break;    
  case (FLOAT32):
    internal_type = GL_FLOAT;
    break;
  case (DOUBLE64):
    fprintf(stderr, "Error: Cannot handle this type DOUBLE64 %i\n", data_type);
    exit(-1);
  default:
    fprintf(stderr, "Error: Unrecognized type %i\n", data_type);
    exit(-1);
  }
  // Create one OpenGL texture
  check_error_gl("before texture");
  GLuint textureID;
  glGenTextures(1, &textureID);   
  glBindTexture(GL_TEXTURE_3D, textureID);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, data_dim[0], data_dim[1], data_dim[2],
	       0, GL_RED, internal_type, data_ptr);
  check_error_gl("generate texture");
  // OpenGL has now copied the data. Free our own version
  delete [] data_ptr;  
  // ... nice trilinear filtering ...    
  // glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  check_error_gl("filter texture");
  // Return the ID of the texture we just created
  return textureID;
}

GLuint loadTFN_custom()
{
  // Create one OpenGL texture
  check_error_gl("before texture");
  GLuint textureID;
  glGenTextures(1, &textureID);   
  glBindTexture(GL_TEXTURE_2D, textureID);
  check_error_gl("generate texture");
  // ... nice trilinear filtering ...    
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // Return the ID of the texture we just created
  return textureID;  
}

void updateTFN_custom(const GLuint textureID, const GLubyte* palette,
		      const int width, const int height)
{
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
	       GL_UNSIGNED_BYTE, static_cast<const void*>(palette));
}
