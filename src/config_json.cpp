#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// #define DEBUG_PRINTS
#include "common.h"
#include "config_json.h"
#include "utils.h"

namespace config_ns
{

using libmap_t = std::unordered_map<std::string, Json::Value>;

static bool build_library_map(const Json::Value& root, libmap_t& lib)
{
	// library is optional
	if (!root.isMember("library")) {
		DBG("build_library_map: no library section");
		return true;
	}

	const Json::Value library = root["library"];
	if (!library.isObject()) {
		ERR("build_library_map: library is not an object");
		return false;
	}

	if (!library.isMember("objects")) {
		DBG("build_library_map: library has no objects");
		return true;
	}

	const Json::Value objects = library["objects"];
	if (!objects.isArray()) {
		ERR("build_library_map: library.objects is not an array");
		return false;
	}

	DBG("build_library_map: num objects: " << objects.size());
	for (unsigned int i = 0; i < objects.size(); i++) {
		const Json::Value obj = objects[i];

		if (!obj.isObject()) {
			ERR("build_library_map: library.objects[" << i << "] is not an object");
			return false;
		}
		else if (!obj.isMember("type")) {
			ERR("build_library_map: library.objects[" << i << "] missing type");
			return false;
		}
		else if (!obj["type"].isString()) {
			ERR("build_library_map: library.objects[" << i << "] type is not a string");
			return false;
		}

		if (!obj.isMember("name")) {
			ERR("build_library_map: library.objects[" << i << "] missing name");
			return false;
		}
		else if (!obj["name"].isString()) {
			ERR("build_library_map: library.objects[" << i << "] name is not a string");
			return false;
		}

		const std::string name = obj["name"].asString();
		if (name.empty()) {
			ERR("build_library_map: library.objects[" << i << "] has empty name");
			return false;
		}

		auto [it, inserted] = lib.emplace(name, obj);
		if (!inserted) {
			ERR("build_library_map: duplicate library object name: " << name);
			return false;
		}

		DBG("build_library_map: added: " << name << " (type: " << obj["type"].asString() << ")");
	}

	return true;
}

static bool expand_scene_objects(const Json::Value& objects, const libmap_t& lib, std::vector<Json::Value>& out, const std::string& scene_name)
{
	if (!objects.isArray()) {
		ERR("expand_scene_objects: scene.objects is not an array (scene: " << scene_name << ")");
		return false;
	}

	DBG("expand_scene_objects: scene: " << scene_name << ", num objects: " << objects.size());

	for (unsigned int i = 0; i < objects.size(); i++) {
		const Json::Value obj = objects[i];
		if (!obj.isObject()) {
			ERR("expand_scene_objects: objects[" << i << "] is not an object (scene: " << scene_name << ")");
			return false;
		}
		else if (!obj.isMember("type")) {
			ERR("expand_scene_objects: objects[" << i << "] missing type (scene: " << scene_name << ")");
			return false;
		}

		const std::string type = obj["type"].asString();
		if (type.empty()) {
			ERR("expand_scene_objects: objects[" << i << "] type is empty (scene: " << scene_name << ")");
			return false;
		}

		if (type != "include") {
			out.push_back(obj);
			continue;
		}

		// include expansion
		if (!obj.isMember("names")) {
			ERR("expand_scene_objects: include missing names[] (scene: " << scene_name << ", idx: " << i << ")");
			return false;
		}

		const Json::Value names = obj["names"];
		if (!names.isArray() || names.size() < 1) {
			ERR("expand_scene_objects: include names[] is not a non-empty array (scene: " << scene_name << ", idx: " << i << ")");
			return false;
		}

		DBG("expand_scene_objects: include: num names: " << names.size());

		for (unsigned int j = 0; j < names.size(); j++) {
			const Json::Value n = names[j];
			if (!n.isString()) {
				ERR("expand_scene_objects: include names[" << j << "] is not a string (scene: " << scene_name << ")");
				return false;
			}

			const std::string name = n.asString();
			auto it = lib.find(name);
			if (it == lib.end()) {
				ERR("expand_scene_objects: include name not found in library: " << name << " (scene: " << scene_name << ")");
				return false;
			}

			// push deep copy of the referenced object
			out.push_back(it->second);

			DBG("expand_scene_objects: included: " << name << " (type: " << it->second["type"].asString() << ")");
		}
	}

	DBG("expand_scene_objects: scene: " << scene_name << ", expanded objects: " << out.size());
	return true;
}

static bool validate_unique_names(const AST& ast, const std::string& scene_name)
{
	std::unordered_set<std::string> polys;
	std::unordered_set<std::string> elems;

	for (const auto& p : ast.polygons) {
		auto [it, inserted] = polys.insert(p.name);
		if (!inserted) {
			ERR("validate_unique_names: duplicate polygon name: " << p.name << " (scene: " << scene_name << ")");
			return false;
		}
	}

	for (const auto& e : ast.elements) {
		auto [it, inserted] = elems.insert(e.name);
		if (!inserted) {
			ERR("validate_unique_names: duplicate element name: " << e.name << " (scene: " << scene_name << ")");
			return false;
		}
	}

	return true;
}

bool parse_json_attrib(unit attribs[NUM_ATTRIBUTES], const Json::Value& attrib, const unsigned int idx [[maybe_unused]])
{
	DBG("parse_attrib: " << idx << ", num attribs: " << attrib.size());
	if (!attrib.isArray() || attrib.size() != NUM_ATTRIBUTES) {
		ERR("parse_attrib: attrib is not an array or too few values");
	}

	for (unsigned int i = 0; i < attrib.size(); i++) {
		DBG("parse_attrib: " << idx << ", " << i << ", attrib: " << i << ", val: " << attrib[i]);
		if (!attrib[i].isNumeric()) {
			ERR("parse_attrib: attrib[" << i << "] is not numeric");
			return false;
		}
		attribs[i] = attrib[i].asFloat();
	}

	return true;
}

bool parse_json_vector(unit coords[NUM_COORDS], const Json::Value& point, const unsigned int idx [[maybe_unused]])
{
	DBG("parse_vector: " << idx << ", num points: " << point.size());
	if (!point.isArray() || point.size() != NUM_COORDS) {
		ERR("parse_vector: point is not an array or too few values");
		return false;
	}

	for (unsigned int i = 0; i < point.size(); i++) {
		DBG("parse_vector: " << idx << ", " << i << ", coord: " << i << ", val: " << point[i]);
		if (!point[i].isNumeric()) {
			ERR("parse_vector: point[" << i << "] is not numeric");
			return false;
		}
		coords[i] = point[i].asFloat();
	}

	return true;
}

bool parse_json_pair(int points[2], const Json::Value& point, const unsigned int idx [[maybe_unused]])
{
	DBG("parse_pair: " << idx << ", num points: " << point.size());
	if (!point.isArray() || point.size() != 2) {
		ERR("parse_pair: point is not an array or too few values");
		return false;
	}

	if (!point[0].isNumeric() || !point[1].isNumeric()) {
		ERR("parse_pair: point[0/1] is not numeric");
		return false;
	}

	points[0] = point[0].asInt();
	points[1] = point[1].asInt();

	return true;
}

bool parse_json_polygon(AST& ast, const Json::Value& polygon, const unsigned int idx [[maybe_unused]])
{
	unit coords[NUM_COORDS];
	polygon_def pd {};
	bool rc;

	if (!polygon.isMember("name")) {
		ERR("parse_polygon: polygon[" << idx << "] missing name");
		return false;
	}
	else if (!polygon["name"].isString()) {
		ERR("parse_polygon: polygon[" << idx << "] name is not a string");
		return false;
	}

	DBG("parse_polygon: " << idx << ", name: " << polygon["name"]);
	pd.name = polygon["name"].asString();
	if (pd.name.empty()) {
		ERR("parse_polygon: polygon[" << idx << "] name is empty");
		return false;
	}

	if (!polygon.isMember("color_idx")) {
		ERR("parse_polygon: polygon[" << idx << "] missing color_idx");
		return false;
	}
	else if (!polygon["color_idx"].isNumeric()) {
		ERR("parse_polygon: polygon[" << idx << "] color_idx is not numeric");
		return false;
	}
	DBG("parse_polygon: " << idx << ", color_idx: " << polygon["color_idx"]);
	pd.color_index = polygon["color_idx"].asInt();

	if (!polygon.isMember("force")) {
		ERR("parse_polygon: polygon[" << idx << "] missing force");
		return false;
	}
	else if (!polygon["force"].isBool()) {
		ERR("parse_polygon: polygon[" << idx << "] force is not a boolean");
		return false;
	}
	DBG("parse_polygon: " << idx << ", force: " << polygon["force"]);
	pd.force = polygon["force"].asBool();

	if (!polygon.isMember("objects")) {
		ERR("parse_polygon: objects[" << idx << "] missing objects");
		return false;
	}
	else if (!polygon["objects"].isArray() || polygon["objects"].size() < 1) {
		ERR("parse_polygon: objects[" << idx << "] objects is not an array");
		return false;
	}
	const Json::Value objects = polygon["objects"];
	DBG("parse_polygon: " << idx << ", num objects: " << objects.size());

	for (unsigned int i = 0; i < objects.size(); i++) {
		const Json::Value object = objects[i];
		if (!object.isObject()) {
			ERR("parse_polygon: objects[" << i << "] is not an object");
			return false;
		}

		if (!object.isMember("type")) {
			ERR("parse_polygon: objects[" << i << "] missing type");
			return false;
		}
		else if (!object["type"].isString()) {
			ERR("parse_polygon: objects[" << idx << "] name is not a string");
			return false;
		}

		const Json::Value type = object["type"];
		DBG("parse_polygon: " << idx << ", " << i << ", type: " << type.asString());
		if (type.empty()) {
			ERR("parse_polygon: objects[" << idx << "] type is empty");
			return false;
		}

		if (type == "vector") {
			if (!object.isMember("points")) {
				ERR("parse_polygon: objects[" << i << "] missing points");
				return false;
			}
			else if (!object["points"].isArray() || object["points"].size() < MIN_VECTORS) {
				ERR("parse_polygon: " << idx << ", " << i << " too few vectors: " << polygon["points"].size()
				                      << " required at least: " << MIN_VECTORS);
				return false;
			}

			const Json::Value& points = object["points"];
			DBG("parse_polygon: " << idx << ", " << i << ", num points: " << points.size());

			for (unsigned int j = 0; j < points.size(); j++) {
				const Json::Value& point = points[j];
				rc = parse_json_vector(coords, point, j);
				if (!rc) {
					return false;
				}
				pd.points.push_back(coords);
			}
			if (polygon.isMember("normal")) {
				const Json::Value& normal = polygon["normal"];
				DBG("parse_polygon: " << idx << ", " << i << ", num coords: " << points.size());
				rc = parse_json_vector(coords, normal, i);
				if (!rc) {
					return false;
				}

				vector_3 v(coords);
				pd.normal_cfg = v;
			}
		}
		else {
			WARN("parse_polygon: unsupported object type" << type.asString());
		}
	}

	ast.polygons.push_back(std::move(pd));
	return true;
}

bool parse_json_element(AST& ast, const Json::Value& element, const unsigned int idx [[maybe_unused]])
{
	unit atts[NUM_ATTRIBUTES] = {0, 0, 0, 0, 0, 0, 1};
	element_def ed {};
	bool rc;

	if (!element.isMember("name")) {
		ERR("parse_element: element[" << idx << "] missing name");
		return false;
	}
	else if (!element["name"].isString()) {
		ERR("parse_element: element[" << idx << "] name is not a string");
		return false;
	}
	DBG("parse_element: " << idx << ", name: " << element["name"]);
	ed.name = element["name"].asString();
	if (ed.name.empty()) {
		ERR("parse_element: element[" << idx << "] name is empty");
		return false;
	}

	if (element.isMember("parent")) {
		if (!element["parent"].isString()) {
			ERR("parse_element: element[" << idx << "] parent is not a string");
			return false;
		}
		DBG("parse_element: " << idx << ", parent: " << element["parent"]);
		ed.parent = element["parent"].asString();
	}

	if (!element.isMember("active")) {
		ERR("parse_element: element[" << idx << "] missing active");
		return false;
	}
	else if (!element["active"].isBool()) {
		ERR("parse_element: element[" << idx << "] active is not a boolean");
		return false;
	}
	DBG("parse_element: " << idx << ", active: " << element["active"]);
	ed.active = element["active"].asBool();

	if (!element.isMember("objects")) {
		ERR("parse_element: element[" << idx << "] missing objects");
		return false;
	}
	else if (!element["objects"].isArray() || element["objects"].size() < 1) {
		ERR("parse_element: element[" << idx << "] objects is not an array");
		return false;
	}
	const Json::Value objects = element["objects"];
	DBG("parse_element: " << idx << ", num objects: " << objects.size());
	for (unsigned int i = 0; i < objects.size(); i++) {
		const Json::Value object = objects[i];
		if (!object.isObject()) {
			ERR("parse_element: objects[" << i << "] is not an object");
			return false;
		}

		if (!object.isMember("type")) {
			ERR("parse_element: objects[" << i << "] missing type");
			return false;
		}
		else if (!object["type"].isString()) {
			ERR("parse_element: objects[" << idx << "] name is not a string");
			return false;
		}

		const Json::Value type = object["type"];
		if (type.empty()) {
			ERR("parse_element: objects[" << idx << "] type is empty");
			return false;
		}

		DBG("parse_element: " << idx << ", " << i << ", type: " << type.asString());
		if (type == "attribute") {
			if (object.isMember("attrib")) {
				const Json::Value attrib = object["attrib"];
				DBG("parse ini attribs");
				rc = parse_json_attrib(atts, attrib, i);
				if (!rc) {
					return false;
				}
				ed.ini_att = atts;
				ed.ini_att.print();
			}
			else if (object.isMember("motion")) {
				DBG("parse motion attribs");
				const Json::Value motion = object["motion"];
				rc = parse_json_attrib(atts, motion, i);
				if (!rc) {
					return false;
				}
				ed.run_att = atts;
				ed.run_att.value().print();
			}
			else {
				WARN("parse_element: unsupported attribute type" << type.asString());
			}
		}
		else {
			WARN("parse_element: unsupported object type" << type.asString());
		}
	}

	if (element.isMember("polygons")) {
		if (!element["polygons"].isArray() || element["polygons"].size() < 1) {
			ERR("parse_element: element[" << idx << "] polygons is not an array");
			return false;
		}

		const Json::Value polygons = element["polygons"];
		std::unordered_set<std::string> seen;
		DBG("parse_element: " << idx << ", num polygons: " << polygons.size());
		for (unsigned int i = 0; i < polygons.size(); i++) {
			if (!polygons[i].isString()) {
				ERR("parse_element: polygons[" << i << "] polygon name is not a string");
				return false;
			}
			else if (polygons[i].empty()) {
				ERR("parse_element: polygons[" << i << "] polygon name is empty");
				return false;
			}

			const std::string poly_name = polygons[i].asString();
			DBG("parse_element: " << idx << ", polygon: " << i << ", name: " << poly_name);

			auto [it, inserted] = seen.insert(poly_name);
			if (!inserted) {
				ERR("parse_element: " << idx
				                      << ", element '" << ed.name
				                      << "' has duplicate polygon reference: " << poly_name);
				return false;
			}

			ed.polygons.emplace_back(poly_name);
		}
	}

	ast.elements.push_back(std::move(ed));
	return true;
}

bool parse_json_environment(AST& ast, const Json::Value& object, const unsigned int idx [[maybe_unused]])
{
	unit coords[NUM_COORDS];
	int points[2];
	Json::Value val;
	bool rc;

	DBG("parse_env: environment:");
	if (object.isMember("color_idx")) {
		DBG("parse_env: color_idx:");
		if (!object["color_idx"].isNumeric()) {
			ERR("parse_env: object[" << idx << "] color_idx is not numeric");
			return false;
		}
		val = object["color_idx"];
		ast.env.color_idx = val.asInt();
	}

	if (object.isMember("light_type")) {
		DBG("parse_env: light_type:");
		if (!object["light_type"].isNumeric()) {
			ERR("parse_env: object[" << idx << "] light_type is not numeric");
			return false;
		}
		val = object["light_type"];
		ast.env.light_type = val.asInt();
	}

	if (object.isMember("light_direction")) {
		DBG("parse_env: light_direction:");
		val = object["light_direction"];
		rc = parse_json_vector(coords, val, idx);
		if (!rc) {
			return false;
		}

		vector_3 v(coords);
		ast.env.light_direction = v;
	}

	if (object.isMember("light_position")) {
		DBG("parse_env: light_position:");
		val = object["light_position"];
		rc = parse_json_vector(coords, val, idx);
		if (!rc) {
			return false;
		}

		vector_3 v(coords);
		ast.env.light_position = v;
	}

	if (object.isMember("camera_position")) {
		DBG("parse_env: camera_position:");
		val = object["camera_position"];
		rc = parse_json_vector(coords, val, idx);
		if (!rc) {
			return false;
		}

		vector_3 v(coords);
		ast.env.camera_position = v;
	}

	if (object.isMember("focal_len")) {
		DBG("parse_env: focal_len:");
		if (!object["focal_len"].isNumeric()) {
			ERR("parse_env: object[" << idx << "] focal_len is not numeric");
			return false;
		}
		val = object["focal_len"];
		ast.env.focal_len = val.asFloat();
	}

	if (object.isMember("near_eps")) {
		val = object["near_eps"];
		if (!object["near_eps"].isNumeric()) {
			ERR("parse_env: object[" << idx << "] near_eps is not numeric");
			return false;
		}
		ast.env.near_eps = val.asFloat();
	}

	if (object.isMember("min_pos")) {
		DBG("parse_env: min_pos:");
		val = object["min_pos"];
		rc = parse_json_pair(points, val, idx);
		if (!rc) {
			return false;
		}

		ast.env.min_pos = config_ns::point_def {points[0], points[1]};
	}

	if (object.isMember("max_pos")) {
		DBG("parse_env: max_pos:");
		val = object["max_pos"];
		rc = parse_json_pair(points, val, idx);
		if (!rc) {
			return false;
		}

		ast.env.max_pos = config_ns::point_def {points[0], points[1]};
	}

	if (object.isMember("loops")) {
		DBG("parse_env: loops:");
		if (!object["loops"].isNumeric()) {
			ERR("parse_env: object[" << idx << "] loops is not numeric");
			return false;
		}
		val = object["loops"];
		ast.run.loops = val.asInt();
	}

	if (object.isMember("loop_delay")) {
		DBG("parse_env: loop_delay:");
		if (!object["loop_delay"].isNumeric()) {
			ERR("parse_env: object[" << idx << "] loop_delay is not numeric");
			return false;
		}
		val = object["loop_delay"];
		ast.run.loop_delay = val.asInt();
	}

	return true;
}

bool parse_json_objects(AST& ast, const Json::Value& scene, const libmap_t& lib, const unsigned int idx [[maybe_unused]])
{
	bool rc;

	const std::string scene_name = scene.isMember("name") ? scene["name"].asString() : std::string("<unnamed>");

	const Json::Value objects = scene["objects"];
	std::vector<Json::Value> expanded;
	expanded.reserve(objects.isArray() ? objects.size() : 0);

	rc = expand_scene_objects(objects, lib, expanded, scene_name);
	if (!rc) {
		return false;
	}

	DBG("parse_object: scene: " << scene_name << ", num expanded objects: " << expanded.size());

	for (unsigned int i = 0; i < expanded.size(); i++) {
		const Json::Value object = expanded[i];
		const Json::Value type = object["type"];
		DBG("parse_object: " << idx << ", " << i << " , type: " << type.asString());

		if (type == "polygon") {
			rc = parse_json_polygon(ast, object, i);
			if (!rc) {
				return false;
			}
		}
		else if (type == "element") {
			rc = parse_json_element(ast, object, i);
			if (!rc) {
				return false;
			}
		}
		else if (type == "environment") {
			rc = parse_json_environment(ast, object, i);
			if (!rc) {
				return false;
			}
		}
		else {
			ERR("parse_object: unsupported object type " << type.asString() << " (scene: " << scene_name << ")");
			return false;
		}
	}

	DBG("parse_object: scene: " << scene_name << ", num polygons: " << (int)ast.polygons.size());
	DBG("parse_object: scene: " << scene_name << ", num elements: " << (int)ast.elements.size());

	// enforce invariants early; avoids ambiguous resolution later
	rc = validate_unique_names(ast, scene_name);
	if (!rc) {
		return false;
	}

	return true;
}

static bool require_min_version(const Json::Value& root, int maj_req, int min_req, int pat_req)
{
	if (!root.isMember("version") || !root["version"].isString()) {
		ERR("parse_json: missing or non-string version");
		return false;
	}

	const std::string v = root["version"].asString(); // e.g. "0.0.2"
	int maj = 0, min = 0, pat = 0;

	// very small parser; reject anything unexpected
	if (sscanf(v.c_str(), "%d.%d.%d", &maj, &min, &pat) != 3) {
		ERR("parse_json: bad version format: " << v);
		return false;
	}

	auto geq = [&](int A, int B, int C, int X, int Y, int Z) {
		if (A != X) {
			return A > X;
		}
		if (B != Y) {
			return B > Y;
		}
		return C >= Z;
	};

	if (!geq(maj, min, pat, maj_req, min_req, pat_req)) {
		ERR("parse_json: config version too old: "
		    << v << " required >= " << maj_req << "." << min_req << "." << pat_req);
		return false;
	}

	return true;
}

AST parse_json(const std::string& filename, const std::string& conf_name)
{
	AST ast;
	bool rc;
	std::ifstream ifs;
	Json::Value root;
	Json::CharReaderBuilder builder;
	JSONCPP_STRING errs;
	bool found = false;
	libmap_t lib;

	ifs.open(filename.c_str(), std::ios::in);
	if (!ifs) {
		sys_error("parse_json: file not found:", filename.c_str());
	}

	ifs.unsetf(std::ios::skipws);

	builder["collectComments"] = false;
	rc = parseFromStream(builder, ifs, &root, &errs);
	ifs.close();
	if (!rc) {
		sys_error("parse_json: ", errs.c_str());
	}

	rc = require_min_version(root, 0, 0, 2);
	if (!rc) {
		sys_error("parse_json: unsupported config version in: ", filename.c_str());
	}

	// require scenes array (avoid silent null Value behavior)
	if (!root.isMember("scenes") || !root["scenes"].isArray()) {
		sys_error("parse_json: missing or invalid 'scenes' array in: ", filename.c_str());
	}

	rc = build_library_map(root, lib);
	if (!rc) {
		sys_error("parse_json: failed to build library map in: ", filename.c_str());
	}

	const Json::Value scenes = root["scenes"];
	DBG("parse_json: conf_name: " << conf_name);
	DBG("parse_json: num scenes: " << scenes.size());

	for (unsigned int i = 0; i < scenes.size(); i++) {
		const Json::Value scene = scenes[i];

		if (!scene.isObject() || !scene.isMember("name") || !scene["name"].isString()) {
			sys_error("parse_json: scene missing string 'name' in: ", filename.c_str());
		}
		const std::string name = scene["name"].asString();
		if (name.empty()) {
			sys_error("parse_json: scene 'name' is empty: ", filename.c_str());
		}

		DBG("parse_json: scene: " << i << ", " << name);

		if (name == conf_name) {
			found = true;
			rc = parse_json_objects(ast, scene, lib, i);
			if (!rc) {
				sys_error("parse_json: scene parse failed: ", conf_name.c_str());
			}
			break; // stop at first match
		}
	}

	if (!found) {
		sys_error("parse_json: scene not found: ", conf_name.c_str());
	}

	return ast;
}

} // namespace config_ns
