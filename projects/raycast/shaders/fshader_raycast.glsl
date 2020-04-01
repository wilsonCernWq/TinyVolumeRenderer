#version 330 core
uniform sampler3D tex3d; // volume data
uniform sampler2D textf; // transfer function
uniform float samplingRate;
uniform float samplingStep;
uniform vec3 camera; // camera in world coordinate
in vec3 fRayDir;
layout(location = 0) out vec4 color;
void main()
{
  // initialization
  color = vec4(0.0);  
  // read the metadata
  vec3 dir = normalize(fRayDir);
  vec3 inv_dir = 1.0 / dir;
  // Check for intersection against the bounding box of the volume
  vec3 box_max = vec3( 1);
  vec3 box_min = vec3(-1);
  vec3 tmin_tmp = (box_min - camera) * inv_dir;
  vec3 tmax_tmp = (box_max - camera) * inv_dir;
  vec3 tmin = min(tmin_tmp, tmax_tmp);
  vec3 tmax = max(tmin_tmp, tmax_tmp);
  float tenter = max(0, max(tmin.x, max(tmin.y, tmin.z)));
  float texit = min(tmax.x, min(tmax.y, tmax.z));
  if (tenter > texit) {
    discard;
  }
  else {
    // raycasting
    vec3  dt_vec = samplingStep * abs(inv_dir);
    float dt = min(dt_vec.x, min(dt_vec.y, dt_vec.z));
    float scale = 1.0 / samplingRate;
    for (float t = tenter; t < texit; t += dt)
    {
      // compute position
      vec3 p = camera + t * dir;  
      // read the data value (normalized)
      float dataval = texture(tex3d, p * 0.5 + 0.5).r;
      // sample from transfer function
      vec4  datacol = texture(textf, vec2(dataval, 0.f));
      // compositing
      color.rgb += (1 - color.a) * datacol.a * datacol.rgb * scale;
      color.a   += (1 - color.a) * datacol.a * scale;
      if (color.a >= 0.99) { break; }
    }
  }
}
