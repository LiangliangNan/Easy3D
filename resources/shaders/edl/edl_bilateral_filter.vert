#version 150

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtktextureObjectVS.glsl

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

in	vec3	vertexMC;
in	vec2	tcoordMC;

out	vec2	tcoordVC;

void main()
{
	tcoordVC = tcoordMC;
	gl_Position = vec4(vertexMC, 1.0f);
}
