// #define DEBUG_PRINTS
#include "common.h"
#include "element.h"
#include "utils.h"

namespace element_ns
{

using namespace matrix_ns;
using namespace polygon_ns;
using polygon = polygon_ns::polygon;
using polylist_t = polygon_ns::polygon::polylist_t;

void element::init_from_def(const polylist_t& poly_list, element* root, const config_ns::element_def& def)
{
	_name = def.name;
	_active = def.active;
	_ini_att = def.ini_att;
	if (def.run_att.has_value()) {
		_run_att = def.run_att.value();
	}

	if (def.parent.length()) {
		element* parent = find(root, def.parent);
		if (!parent) {
			sys_error("element::read find error - _parent_name");
		}
		_parent = parent;
		_parent_name = def.parent;
		add_node(parent);
	}

	_polygons.clear();
	for (const auto& name : def.polygons) {
		polygon* poly = find(poly_list, name);
		if (!poly) {
			sys_error("element::read find error -  polygon");
		}
		_polygons.push_back(poly);
	}
}

bool element::read(const polylist_t& poly_list, element* root, std::ifstream& ifs)
{
	LINE line;
	int len;
	element* parent;
	polygon* poly;
	std::string s;
	bool rc, finish = false;

	while (!ifs.eof() && !finish) {
		while ((!read_word(ifs, line)) && (!ifs.eof()));
		if (ifs.eof()) {
			break;
		}

		switch (line[1]) {
		case 'n':
			// read element's name
			len = read_word(ifs, line);
			if (!len) {
				sys_error("element::read error _name");
			}

			_name = line;
			break;
		case 't':
			// read element's parent name
			len = read_word(ifs, line);
			if (len) {
				_parent_name = line;
				parent = find(root, _parent_name);
				if (!parent) {
					sys_error("element::read find error - _parent_name");
				}

				add_node(parent);
				_parent = parent;
			}
			break;
		case 'p':
			// add a polygon to element
			len = read_word(ifs, line);
			if (len) {
				s = line;
				poly = find(poly_list, s);
				if (!poly) {
					sys_error("element::read find error -  polygon");
				}
				_polygons.push_back(poly);
			}
			else {
				sys_error("element::read error polygon");
			}
			break;
		case 'f':
			// element active flag
			len = read_word(ifs, line);
			if (!len) {
				sys_error("element::read error flag");
			}

			_active = (bool)atoi(line);
			break;
		case 'a':
			// element's attribute
			rc = _ini_att.read(ifs);
			if (!rc) {
				sys_error("element::read error attrib");
			}
			break;
		default:
			finish = true;
			ifs.seekg(-4, std::ios::cur);
			break;
		}
	}

	return true;
}

void element::print() const
{
#ifdef DEBUG_PRINTS
	DBG(STR("element:", 1));
	DBG(STR("  name: ", 1) << _name);
	DBG(STR("  parent_name: ", 1) << _parent_name);
	DBG(STR("  active: ", 1) << _active);
	DBG(STR("   dirty: ", 1) << _dirty);
	_ini_att.print();

	if (!_polygons.empty()) {
		DBG("num polygons: " << (int)_polygons.size());
		DBG(STR("  polygons:", 1));
		for (const auto& poly : _polygons) {
			poly->print();
		}
	}
#endif // DEBUG_PRINTS
}

#ifdef DEBUG_PRINTS
static void prn(element_ns::element* e, void* data [[maybe_unused]])
{
	if (e) {
		e->print();
	}
}
#endif // DEBUG_PRINTS

void element::print_all()
{
#ifdef DEBUG_PRINTS
	this->print_tree(prn, NULL);
#endif // DEBUG_PRINTS
}

static bool match_element_name(element_ns::element* e, void* data)
{
	auto* key = static_cast<const std::string*>(data);
	return e && key && (e->get_name() == *key);
}

element* element::find(element* root, const std::string& s) const
{
	return root ? root->search_tree(match_element_name, (void*)&s) : nullptr;
}

void element::update(const attrib& att)
{
	_ini_att += att;
	_dirty = true;
}

void element::update()
{
	if (_run_att.has_value()) {
		_ini_att += _run_att.value();
		_dirty = true;
	}
}

void element::update(const matrix& p_trans, const matrix& p_rot, const bool p_dirty, frame_context& frame_ctx)
{
	if (!_active) {
		return;
	}

	_trans_mat.prep_trans_mat(_ini_att);
	_rot_mat.prep_rot_mat(_ini_att);

	_world_dirty = _dirty || p_dirty;

	if (_world_dirty) {
		_trans_mat = p_trans * _trans_mat;
		_rot_mat = p_rot * _rot_mat;

		if (_polygons.size()) {
			for (auto poly : _polygons) {
				if (poly) {
					poly->update(_trans_mat, _rot_mat, frame_ctx);
				}
			}
		}

		_dirty = false;
	}
}

void element::update(frame_context& frame_ctx)
{
	matrix m_trans, m_rot;
	bool parent_dirty;

	if (_parent) {
		m_trans = _parent->_trans_mat;
		m_rot = _parent->_rot_mat;
		parent_dirty = _parent->_world_dirty;
	}
	else {
		m_trans = matrix_ns::get_unit_mat();
		m_rot = matrix_ns::get_unit_mat();
		parent_dirty = false;
	}

	DBG("element::update: " << _name << "parent: " << _parent_name);
	DBG("transform:");
	m_trans.print();
	DBG("rotate:");
	m_rot.print();
	update(m_trans, m_rot, parent_dirty, frame_ctx);
}

static void upd(element_ns::element* e, void* data)
{
	if (!e || !data) {
		return;
	}

	auto* frame_ctx = static_cast<scene_ns::frame_context*>(data);
	e->update(*frame_ctx);
}

void element::update_all(frame_context& frame_ctx)
{
	this->update_tree(upd, &frame_ctx);
}

polygon* element::find(const polylist_t& poly_list, const std::string& s) const
{
	for (const auto poly : poly_list) {
		if (poly && poly->get_name() == s) {
			return poly;
		}
	}

	return nullptr;
}

} // namespace element_ns
