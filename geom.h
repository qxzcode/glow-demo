#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace geom {

	using floatVec = std::vector<GLfloat>;
	using vec2 = glm::vec2;
	using vec3 = glm::vec3;
	using vec4 = glm::vec4;
	using mat4 = glm::mat4;

	template <class T, class LT>
	inline std::vector<T>& operator+=(std::vector<T>& lhs, std::initializer_list<LT> l) {
		for (LT e : l)
			lhs.push_back(static_cast<T>(e));
		return lhs;
	}
	inline floatVec& operator+=(floatVec& lhs, vec3 v) {
		return lhs += {v.x, v.y, v.z};
	}
	inline floatVec& operator+=(floatVec& lhs, vec2 v) {
		return lhs += {v.x, v.y};
	}

	template<class... argT>
	inline floatVec _make(floatVec&(*func)(floatVec&,argT...), argT... args) {
		floatVec buf;
		func(buf, args...);
		return buf;
	}

	inline floatVec& makeTri(floatVec& buf, vec3 color, vec3 v0, vec2 t0, vec3 v1, vec2 t1, vec3 v2, vec2 t2) {
		vec3 norm = glm::cross(v0-v1, v0-v2);
		if (norm!=vec3(0,0,0)) {
			norm = glm::normalize(norm);
		}
		buf += v0; buf += color; buf += norm; buf += t0;
		buf += v1; buf += color; buf += norm; buf += t1;
		buf += v2; buf += color; buf += norm; buf += t2;
		return buf;
	}
	inline floatVec makeTri(vec3 color, vec3 v0, vec3 v1, vec3 v2) {return _make(makeTri, color, v0,vec2{}, v1,vec2{}, v2,vec2{});}

	inline floatVec& makeQuad(floatVec& buf, vec3 color, vec3 v0, vec3 v1, vec3 v2, vec3 v3) {
		makeTri(buf, color, v0,{0,0}, v1,{1,0}, v2,{1,1});
		makeTri(buf, color, v2,{1,1}, v3,{0,1}, v0,{0,0});
		return buf;
	}
	inline floatVec makeQuad(vec3 color, vec3 v0, vec3 v1, vec3 v2, vec3 v3) {return _make(makeQuad, color, v0, v1, v2, v3);}

}