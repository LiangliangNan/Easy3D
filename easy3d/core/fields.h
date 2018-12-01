#pragma once

#include "drawable.h"

namespace easy3d {

	class ScalarField {
	public:
		ScalarField() {}
		~ScalarField() {}
	};


	// scalar field defined on faces
	class FaceScalarField : public ScalarField, public FacesDrawable {
	public:
		FaceScalarField() {}
		~FaceScalarField() {}
	};


	// scalar field defined on edges
	class EdgeScalarField : public ScalarField, public LinesDrawable {
	public:
		EdgeScalarField() {}
		~EdgeScalarField() {}
	};


	class VertexScalarField : ScalarField, public PointsDrawable {
	public:
		VertexScalarField() {}
		~VertexScalarField() {}
	};


	class VectorField : public LinesDrawable {
	public:
		VectorField();
		~VectorField();
	};

}