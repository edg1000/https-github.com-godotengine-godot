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

/** @file  FBXAnimation.cpp
 *  @brief Assimp::FBX::AnimationCurve, Assimp::FBX::AnimationCurveNode,
 *         Assimp::FBX::AnimationLayer, Assimp::FBX::AnimationStack
 */

#ifndef ASSIMP_BUILD_NO_FBX_IMPORTER

#include "FBXCommon.h"
#include "FBXDocument.h"
#include "FBXDocumentUtil.h"
#include "FBXParser.h"
#include <iostream>

namespace Assimp {
namespace FBX {

using namespace Util;

// ------------------------------------------------------------------------------------------------
AnimationCurve::AnimationCurve(uint64_t id, const Element &element, const std::string &name, const Document & /*doc*/) :
		Object(id, element, name) {
	const Scope &sc = GetRequiredScope(element);
	const Element &KeyTime = GetRequiredElement(sc, "KeyTime");
	const Element &KeyValueFloat = GetRequiredElement(sc, "KeyValueFloat");

	// note preserved keys and values for legacy FBXConverter.cpp
	// we can remove this once the animation system is written
	// and clean up this code so we do not have copies everywhere.
	ParseVectorDataArray(keys, KeyTime);
	ParseVectorDataArray(values, KeyValueFloat);

	if (keys.size() != values.size()) {
		DOMError("the number of key times does not match the number of keyframe values", &KeyTime);
	}

	// put the two lists into the map, underlying container is really just a dictionary
	// these will always match, if not an error will throw and the file will not import
	// this is useful because we then can report something and fix this later if it becomes an issue
	// at this point we do not need a different count of these elements so this makes the
	// most sense to do.
	for (size_t x = 0; x < keys.size(); x++) {
		keyvalues[keys[x]] = values[x];
	}

	const Element *KeyAttrDataFloat = sc["KeyAttrDataFloat"];
	if (KeyAttrDataFloat) {
		ParseVectorDataArray(attributes, *KeyAttrDataFloat);
	}

	const Element *KeyAttrFlags = sc["KeyAttrFlags"];
	if (KeyAttrFlags) {
		ParseVectorDataArray(flags, *KeyAttrFlags);
	}
}

// ------------------------------------------------------------------------------------------------
AnimationCurve::~AnimationCurve() {
	// empty
}

// ------------------------------------------------------------------------------------------------
AnimationCurveNode::AnimationCurveNode(uint64_t id, const Element &element, const std::string &name,
		const Document &doc, const char *const *target_prop_whitelist /*= NULL*/,
		size_t whitelist_size /*= 0*/) :
		Object(id, element, name), target(), doc(doc) {
	const Scope &sc = GetRequiredScope(element);

	// find target node
	const char *whitelist[] = { "Model", "NodeAttribute", "Deformer" };
	const std::vector<const Connection *> &conns = doc.GetConnectionsBySourceSequenced(ID(), whitelist, 3);

	for (const Connection *con : conns) {

		// link should go for a property
		if (!con->PropertyName().length()) {
			continue;
		}

		if (target_prop_whitelist) {
			const char *const s = con->PropertyName().c_str();
			bool ok = false;
			for (size_t i = 0; i < whitelist_size; ++i) {
				if (!strcmp(s, target_prop_whitelist[i])) {
					ok = true;
					break;
				}
			}

			if (!ok) {
				throw std::range_error("AnimationCurveNode target property is not in whitelist");
			}
		}

		const Object *const ob = con->DestinationObject();
		if (!ob) {
			DOMWarning("failed to read destination object for AnimationCurveNode->Model link, ignoring", &element);
			continue;
		}

		// XXX support constraints as DOM class
		//ai_assert(dynamic_cast<const Model*>(ob) || dynamic_cast<const NodeAttribute*>(ob));
		target = ob;
		if (!target) {
			continue;
		}

		prop = con->PropertyName();
		break;
	}

	if (!target) {
		DOMWarning("failed to resolve target Model/NodeAttribute/Constraint for AnimationCurveNode", &element);
	}

	props = GetPropertyTable(doc, "AnimationCurveNode.FbxAnimCurveNode", element, sc, false);
}

// ------------------------------------------------------------------------------------------------
AnimationCurveNode::~AnimationCurveNode() {
	// empty
}

// ------------------------------------------------------------------------------------------------
const std::map<std::string, const AnimationCurve *> &AnimationCurveNode::Curves() const {
	if (curves.empty()) {
		// resolve attached animation curves
		const std::vector<const Connection *> &conns = doc.GetConnectionsByDestinationSequenced(ID(), "AnimationCurve");

		//std::cout << "================== connection processing ======================" << std::endl;
		for (const Connection *con : conns) {

			// // link should go for a property
			// if (!con->PropertyName().length()) {
			// 	continue;
			// }

			const Object *const ob = con->SourceObject();
			if (!ob) {
				DOMWarning("failed to read source object for AnimationCurve->AnimationCurveNode link, ignoring", &element);
				continue;
			}

			const AnimationCurve *const anim = dynamic_cast<const AnimationCurve *>(ob);
			if (!anim) {
				DOMWarning("source object for ->AnimationCurveNode link is not an AnimationCurve", &element);
				continue;
			}
			//std::cout << "property found " << con->PropertyName() << std::endl;
			curves[con->PropertyName()] = anim;
		}
	}

	return curves;
}

// ------------------------------------------------------------------------------------------------
AnimationLayer::AnimationLayer(uint64_t id, const Element &element, const std::string &name, const Document &doc) :
		Object(id, element, name), doc(doc) {
	const Scope &sc = GetRequiredScope(element);

	// note: the props table here bears little importance and is usually absent
	props = GetPropertyTable(doc, "AnimationLayer.FbxAnimLayer", element, sc, true);
}

// ------------------------------------------------------------------------------------------------
AnimationLayer::~AnimationLayer() {
	// empty
}

// ------------------------------------------------------------------------------------------------
AnimationCurveNodeList AnimationLayer::Nodes(const char *const *target_prop_whitelist /*= NULL*/,
		size_t whitelist_size /*= 0*/) const {
	AnimationCurveNodeList nodes;

	// resolve attached animation nodes
	const std::vector<const Connection *> &conns = doc.GetConnectionsByDestinationSequenced(ID(), "AnimationCurveNode");
	nodes.reserve(conns.size());

	for (const Connection *con : conns) {

		// link should not go to a property
		if (con->PropertyName().length()) {
			continue;
		}

		const Object *const ob = con->SourceObject();
		if (!ob) {
			DOMWarning("failed to read source object for AnimationCurveNode->AnimationLayer link, ignoring", &element);
			continue;
		}

		const AnimationCurveNode *const anim = dynamic_cast<const AnimationCurveNode *>(ob);
		if (!anim) {
			DOMWarning("source object for ->AnimationLayer link is not an AnimationCurveNode", &element);
			continue;
		}

		if (target_prop_whitelist) {
			const char *s = anim->TargetProperty().c_str();
			bool ok = false;
			for (size_t i = 0; i < whitelist_size; ++i) {
				if (!strcmp(s, target_prop_whitelist[i])) {
					ok = true;
					break;
				}
			}
			if (!ok) {
				continue;
			}
		}
		nodes.push_back(anim);
	}

	return nodes; // pray for NRVO
}

// ------------------------------------------------------------------------------------------------
AnimationStack::AnimationStack(uint64_t id, const Element &element, const std::string &name, const Document &doc) :
		Object(id, element, name) {
	const Scope &sc = GetRequiredScope(element);

	// note: we don't currently use any of these properties so we shouldn't bother if it is missing
	props = GetPropertyTable(doc, "AnimationStack.FbxAnimStack", element, sc, true);

	// resolve attached animation layers
	const std::vector<const Connection *> &conns = doc.GetConnectionsByDestinationSequenced(ID(), "AnimationLayer");
	layers.reserve(conns.size());

	for (const Connection *con : conns) {

		// link should not go to a property
		if (con->PropertyName().length()) {
			continue;
		}

		const Object *const ob = con->SourceObject();
		if (!ob) {
			DOMWarning("failed to read source object for AnimationLayer->AnimationStack link, ignoring", &element);
			continue;
		}

		const AnimationLayer *const anim = dynamic_cast<const AnimationLayer *>(ob);
		if (!anim) {
			DOMWarning("source object for ->AnimationStack link is not an AnimationLayer", &element);
			continue;
		}
		layers.push_back(anim);
	}
}

// ------------------------------------------------------------------------------------------------
AnimationStack::~AnimationStack() {
	// empty
}

} // namespace FBX
} // namespace Assimp

#endif // ASSIMP_BUILD_NO_FBX_IMPORTER
