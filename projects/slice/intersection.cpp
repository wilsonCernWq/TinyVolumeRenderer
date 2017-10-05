#include "intersection.hpp"
#ifdef USE_GLM
# include <glm/glm.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtc/type_ptr.hpp>
#else
# error "GLM is required here"
#endif
#include <cstdio>
#include <cmath>
#include <algorithm>

//---------------------------------------------------------------------------------------
// Box intersection
//
/////////////////////////////////////////
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3
/////////////////////////////////////////

static const float box_position[] =
{
   1, 1, 1,
  -1, 1, 1,
  -1,-1, 1,
   1,-1, 1,
   1,-1,-1,
   1, 1,-1,
  -1, 1,-1,
  -1,-1,-1
};

static const float box_texcoord[] =
{
  1, 1, 1,
  0, 1, 1,
  0, 0, 1,
  1, 0, 1,
  1, 0, 0,
  1, 1, 0,
  0, 1, 0,
  0, 0, 0
};

struct Intersect {
  float angle;
  glm::vec3 position;
  glm::vec3 texcoord;
};

static glm::vec3  ixMeanPos = glm::vec3(0.f, 0.f, 0.f);
static glm::vec3  ixMeanTex = glm::vec3(0.f, 0.f, 0.f);
static std::vector<Intersect> ixPts;
static float plane;
//---------------------------------------------------------------------------------------

void IntersectComputeBox(float clipCoordVertices[24], float& cameraZMin, float& cameraZMax)
{
  const glm::mat4& mv = GetMVMatrix();
  const glm::mat4& p  = GetProjection();
  float min = 1000.f, max = -1000.f;
  for (int i = 0; i < 8; ++i) {
    glm::vec4 vertex(box_position[3 * i + 0],
		     box_position[3 * i + 1],
		     box_position[3 * i + 2], 1.f);
    vertex = mv * vertex;
    float z = vertex.z / vertex.w;
    vertex = p  * vertex;
    clipCoordVertices[3 * i + 0] = vertex.x / vertex.w;
    clipCoordVertices[3 * i + 1] = vertex.y / vertex.w;
    clipCoordVertices[3 * i + 2] = vertex.z / vertex.w;
    min = std::min(min, z);
    max = std::max(max, z);      
  }
  cameraZMin = min;
  cameraZMax = max;
}

void IntersectReset(float z) {
  ixMeanPos = glm::vec3(0.f, 0.f, 0.f);
  ixMeanTex = glm::vec3(0.f, 0.f, 0.f);
  ixPts.clear();
  glm::vec4 x = GetProjection() * glm::vec4(0,0,z,1.f);
  plane = x.z / x.w;
};

void IntersectPlane(const float box[24], const int a, const int b)
{
  const glm::ivec2 segment(a,b);
  glm::vec3 vA (box[3 * segment.x + 0],
		box[3 * segment.x + 1],
		box[3 * segment.x + 2]);
  glm::vec3 vB (box[3 * segment.y + 0],
		box[3 * segment.y + 1],
		box[3 * segment.y + 2]);
  glm::vec3 tA (box_texcoord[3 * segment.x + 0],
		box_texcoord[3 * segment.x + 1],
		box_texcoord[3 * segment.x + 2]);
  glm::vec3 tB (box_texcoord[3 * segment.y + 0],
		box_texcoord[3 * segment.y + 1],
		box_texcoord[3 * segment.y + 2]); 
  if ((vA.z <= plane && vB.z >= plane) || (vB.z <= plane && vA.z >= plane))
  {
    // get intersection
    if (std::abs(vB.z - vA.z) < 0.0001f) { return; }
    const float ratio = (plane - vA.z) / (vB.z - vA.z);
    const glm::vec3 p = ratio * (vB - vA) + vA;
    const glm::vec3 t = ratio * (tB - tA) + tA;
    ixPts.push_back({0.f, p, t});
    // get center
    ixMeanPos += (p - ixMeanPos) / static_cast<float>(ixPts.size());
    ixMeanTex += (t - ixMeanTex) / static_cast<float>(ixPts.size());
  }
}

bool IntersectSortFunc(const Intersect& a, const Intersect& b) {
  return (a.angle < b.angle);
}

void IntersectSort()
{
  if (ixPts.empty()) { return; }
  for (auto& p : ixPts) {
    const glm::vec3 v = glm::normalize(p.position - ixMeanPos);
    if (std::abs(v.y) < 0.001f) {
      p.angle = v.x > 0 ? 0.f : M_PI;
    }
    else {
      const float tangent = v.y / v.x;
      p.angle = v.x > 0 ? atan(tangent) : atan(tangent) + M_PI;
    }
  }
  std::sort(ixPts.begin(), ixPts.end(), IntersectSortFunc);
}

void IntersectFetch(std::vector<float>& pos, std::vector<float>& tex)
{
  if (ixPts.empty()) { return; }
  pos.clear();
  tex.clear();
  pos.push_back(ixMeanPos.x);
  pos.push_back(ixMeanPos.y);
  pos.push_back(ixMeanPos.z);
  tex.push_back(ixMeanTex.x);
  tex.push_back(ixMeanTex.y);
  tex.push_back(ixMeanTex.z);
  for (auto& p : ixPts) {
    pos.push_back(p.position.x);
    pos.push_back(p.position.y);
    pos.push_back(p.position.z);
    tex.push_back(p.texcoord.x);
    tex.push_back(p.texcoord.y);
    tex.push_back(p.texcoord.z);
  };
  pos.push_back(ixPts[0].position.x);
  pos.push_back(ixPts[0].position.y);
  pos.push_back(ixPts[0].position.z);
  tex.push_back(ixPts[0].texcoord.x);
  tex.push_back(ixPts[0].texcoord.y);
  tex.push_back(ixPts[0].texcoord.z);
}
