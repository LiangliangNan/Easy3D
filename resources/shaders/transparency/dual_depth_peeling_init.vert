#version 150
precision highp float;

in	vec4 vtx_position;

uniform mat4 MVP;
uniform mat4 MANIP = mat4(1.0);
uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

void main(void)
{
     vec4 new_position = MANIP * vtx_position;

     if (clippingPlaneEnabled) {
          gl_ClipDistance[0] = dot(new_position, clippingPlane0);
          if (planeClippingDiscard && gl_ClipDistance[0] < 0)
          return;
          if (crossSectionEnabled) {
               gl_ClipDistance[1] = dot(new_position, clippingPlane1);
               if (planeClippingDiscard && gl_ClipDistance[1] < 0)
               return;
          }
     }

     gl_Position = MVP * new_position;
}
