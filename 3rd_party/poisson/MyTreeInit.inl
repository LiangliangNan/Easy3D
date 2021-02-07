template< class Real >
template< class Data >
int Octree< Real >::init(
	std::size_t num,
	const float* pts, 
	const float* nms, 
	const float* cls, 
	const XForm4x4< Real >& xForm,
	LocalDepth maxDepth, 
	bool useConfidence, 
	std::vector<PointSample>& samples, 
	std::vector<ProjectiveData<Data, Real>>* sampleData ) 
{
	XForm3x3< Real > normalXForm;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++) 
			normalXForm(i, j) = xForm(i, j);
	normalXForm = normalXForm.transpose().inverse();

	// Add the point data
	int outOfBoundPoints = 0, zeroLengthNormals = 0, undefinedNormals = 0, pointCount = 0;
	{
		std::vector< int > nodeToIndexMap;
		Point3D< Real > p, n;
		Data _d;
		for (std::size_t i=0; i<num; ++i) {
			p = Point3D< Real >(pts[i * 3], pts[i * 3 + 1], pts[i * 3 + 2]);
			n = Point3D< Real >(nms[i * 3], nms[i * 3 + 1], nms[i * 3 + 2]);
			p = xForm * p;
			n = normalXForm * n;

			Real len = (Real)Length(n);
			if (!_InBounds(p)) { outOfBoundPoints++; continue; }
			if (!len) { zeroLengthNormals++; continue; }
			if (len != len) { undefinedNormals++; continue; }
			n /= len;
			Point3D< Real > center = Point3D< Real >(Real(0.5), Real(0.5), Real(0.5));
			Real width = Real(1.0);
			TreeOctNode* temp = _spaceRoot;
			LocalDepth depth = _localDepth(temp);
			while (depth < maxDepth)
			{
				if (!temp->children) temp->initChildren(_NodeInitializer);
				int cIndex = TreeOctNode::CornerIndex(center, p);
				temp = temp->children + cIndex;
				width /= 2;
				if (cIndex & 1) center[0] += width / 2;
				else           center[0] -= width / 2;
				if (cIndex & 2) center[1] += width / 2;
				else           center[1] -= width / 2;
				if (cIndex & 4) center[2] += width / 2;
				else           center[2] -= width / 2;
				depth++;
			}
			Real weight = (Real)(useConfidence ? len : 1.);
			int nodeIndex = temp->nodeData.nodeIndex;
			if (nodeIndex >= nodeToIndexMap.size()) nodeToIndexMap.resize(nodeIndex + 1, -1);
			int idx = nodeToIndexMap[nodeIndex];
			if (idx == -1)
			{
				idx = (int)samples.size();
				nodeToIndexMap[nodeIndex] = idx;
				samples.resize(idx + 1), samples[idx].node = temp;
				if (sampleData) sampleData->resize(idx + 1);
			}
			samples[idx].sample += ProjectiveData< OrientedPoint3D< Real >, Real >(OrientedPoint3D< Real >(p * weight, n * weight), weight);

			// ------------ Liangliang -----------
			if (sampleData) {
				_d = Point3D< Real >(cls[i * 3] * 255, cls[i * 3 + 1] * 255, cls[i * 3 + 2] * 255); // the color range in Misha's code is [0, 255] 
				(*sampleData)[idx] += ProjectiveData< Data, Real >(_d * weight, weight);
			}
			// ------------ Liangliang -----------

			pointCount++;
		}
	}
	if (outOfBoundPoints) 
		fprintf(stderr, "[WARNING] Found out-of-bound points: %d\n", outOfBoundPoints);
	if (zeroLengthNormals)
		fprintf(stderr, "[WARNING] Found zero-length normals: %d\n", zeroLengthNormals);
	if (undefinedNormals)
		fprintf(stderr, "[WARNING] Found undefined normals: %d\n", undefinedNormals);

	memoryUsage();
	return pointCount;
}