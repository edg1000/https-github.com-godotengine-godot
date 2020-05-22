/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

/** @file  FBXNoteAttribute.cpp
 *  @brief Assimp::FBX::NodeAttribute (and subclasses) implementation
 */

#ifndef ASSIMP_BUILD_NO_FBX_IMPORTER

#include "core/math/math_funcs.h"
#include "core/math/transform.h"
#include "FBXDocument.h"
#include "FBXDocumentUtil.h"
#include "FBXMeshGeometry.h"
#include "FBXParser.h"

#include <iostream>

namespace Assimp {
namespace FBX {

using namespace Util;

// ------------------------------------------------------------------------------------------------
Deformer::Deformer(uint64_t id, const Element &element, const Document &doc, const std::string &name) :
		Object(id, element, name) {
	const Scope &sc = GetRequiredScope(element);

	const std::string &classname = ParseTokenAsString(GetRequiredToken(element, 2));
	props = GetPropertyTable(doc, "Deformer.Fbx" + classname, element, sc, true);
}

// ------------------------------------------------------------------------------------------------
Deformer::~Deformer() {
}

Constraint::Constraint(uint64_t id, const Element &element, const Document &doc, const std::string &name) :
		Object(id, element, name) {
	const Scope &sc = GetRequiredScope(element);
	const std::string &classname = ParseTokenAsString(GetRequiredToken(element, 2));
	// used something.fbx as this is a cache name.
	props = GetPropertyTable(doc, "Something.Fbx" + classname, element, sc, true);
}

Constraint::~Constraint() {
}

// ------------------------------------------------------------------------------------------------
Cluster::Cluster(uint64_t id, const Element &element, const Document &doc, const std::string &name) :
		Deformer(id, element, doc, name), node(), valid_transformAssociateModel(false){
	const Scope &sc = GetRequiredScope(element);
	//    for( auto element : sc.Elements())
	//    {
	//        std::cout << "cluster element: " << element.first << std::endl;
	//    }
	//
	//    element: Indexes
	//    element: Transform
	//    element: TransformAssociateModel
	//    element: TransformLink
	//    element: UserData
	//    element: Version
	//    element: Weights

	const Element *const Indexes = sc["Indexes"];
	const Element *const Weights = sc["Weights"];

	const Element *const TransformAssociateModel = sc["TransformAssociateModel"];
	if(TransformAssociateModel != nullptr)
	{
		Transform t = ReadMatrix(*TransformAssociateModel);
		link_mode = SkinLinkMode_Additive;
        valid_transformAssociateModel = true;
	}
	else
	{
		link_mode = SkinLinkMode_Normalized;
		valid_transformAssociateModel = false;
	}

	const Element &Transform = GetRequiredElement(sc, "Transform", &element);
	const Element &TransformLink = GetRequiredElement(sc, "TransformLink", &element);

	// todo: check if we need this
	//const Element& TransformAssociateModel = GetRequiredElement(sc, "TransformAssociateModel", &element);

	transform = ReadMatrix(Transform);
	transformLink = ReadMatrix(TransformLink);

	// it is actually possible that there be Deformer's with no weights
	if (!!Indexes != !!Weights) {
		DOMError("either Indexes or Weights are missing from Cluster", &element);
	}

	if (Indexes) {
		ParseVectorDataArray(indices, *Indexes);
		ParseVectorDataArray(weights, *Weights);
	}

	if (indices.size() != weights.size()) {
		DOMError("sizes of index and weight array don't match up", &element);
	}

	// read assigned node
	const std::vector<const Connection *> &conns = doc.GetConnectionsByDestinationSequenced(ID(), "Model");
	for (const Connection *con : conns) {
		const Model *const mod = ProcessSimpleConnection<Model>(*con, false, "Model -> Cluster", element);
		if (mod) {
			node = mod;
			break;
		}
	}

	if (!node) {
		DOMError("failed to read target Node for Cluster", &element);
	}
}

// ------------------------------------------------------------------------------------------------
Cluster::~Cluster() {
}

// ------------------------------------------------------------------------------------------------
Skin::Skin(uint64_t id, const Element &element, const Document &doc, const std::string &name) :
		Deformer(id, element, doc, name), accuracy(0.0f) {
	const Scope &sc = GetRequiredScope(element);

	// keep this it is used for debugging and any FBX format changes
	// for (auto element : sc.Elements()) {
	// 	std::cout << "skin element: " << element.first << std::endl;
	// }

	const Element *const Link_DeformAcuracy = sc["Link_DeformAcuracy"];
	if (Link_DeformAcuracy) {
		accuracy = ParseTokenAsFloat(GetRequiredToken(*Link_DeformAcuracy, 0));
	}

	const Element *const SkinType = sc["SkinningType"];

	if(SkinType)
	{
		std::string skin_type = ParseTokenAsString(GetRequiredToken(*SkinType, 0));

		if(skin_type == "Linear")
		{
			skinType = Skin_Linear;
		}
		else if(skin_type == "Rigid")
		{
			skinType = Skin_Rigid;
		}
		else if(skin_type == "DualQuaternion")
		{
			skinType = Skin_DualQuaternion;
		}
		else if(skin_type == "Blend")
		{
			skinType = Skin_Blend;
		}
		else
		{
			print_error("[doc:skin] could not find valid skin type: " + String(skin_type.c_str()));
		}
	}

	// resolve assigned clusters
	const std::vector<const Connection *> &conns = doc.GetConnectionsByDestinationSequenced(ID(), "Deformer");

	//

	clusters.reserve(conns.size());
	for (const Connection *con : conns) {

		const Cluster *const cluster = ProcessSimpleConnection<Cluster>(*con, false, "Cluster -> Skin", element);
		if (cluster) {
			clusters.push_back(cluster);
			continue;
		}
	}
}

// ------------------------------------------------------------------------------------------------
Skin::~Skin() {
}
// ------------------------------------------------------------------------------------------------
BlendShape::BlendShape(uint64_t id, const Element &element, const Document &doc, const std::string &name) :
		Deformer(id, element, doc, name) {
	const std::vector<const Connection *> &conns = doc.GetConnectionsByDestinationSequenced(ID(), "Deformer");
	blendShapeChannels.reserve(conns.size());
	for (const Connection *con : conns) {
		const BlendShapeChannel *const bspc = ProcessSimpleConnection<BlendShapeChannel>(*con, false, "BlendShapeChannel -> BlendShape", element);
		if (bspc) {
			blendShapeChannels.push_back(bspc);
			continue;
		}
	}
}
// ------------------------------------------------------------------------------------------------
BlendShape::~BlendShape() {
}
// ------------------------------------------------------------------------------------------------
BlendShapeChannel::BlendShapeChannel(uint64_t id, const Element &element, const Document &doc, const std::string &name) :
		Deformer(id, element, doc, name) {
	const Scope &sc = GetRequiredScope(element);
	const Element *const DeformPercent = sc["DeformPercent"];
	if (DeformPercent) {
		percent = ParseTokenAsFloat(GetRequiredToken(*DeformPercent, 0));
	}
	const Element *const FullWeights = sc["FullWeights"];
	if (FullWeights) {
		ParseVectorDataArray(fullWeights, *FullWeights);
	}
	const std::vector<const Connection *> &conns = doc.GetConnectionsByDestinationSequenced(ID(), "Geometry");
	shapeGeometries.reserve(conns.size());
	for (const Connection *con : conns) {
		const ShapeGeometry *const sg = ProcessSimpleConnection<ShapeGeometry>(*con, false, "Shape -> BlendShapeChannel", element);
		if (sg) {
			shapeGeometries.push_back(sg);
			continue;
		}
	}
}
// ------------------------------------------------------------------------------------------------
BlendShapeChannel::~BlendShapeChannel() {
}
// ------------------------------------------------------------------------------------------------
} // namespace FBX
} // namespace Assimp
#endif
