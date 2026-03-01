#include <algorithm>
#include <chrono>
#include <thread>

// #define DEBUG_PRINTS
#include "common.h"
#include "scene.h"
#ifdef USING_JSON
#include "config_json.h"
#else
#include "config_legacy.h"
#endif

namespace scene_ns
{

using namespace element_ns;
using namespace graphics_ns;
using namespace polygon_ns;
using element = element_ns::element;
using graphics = graphics_ns::graphics;
using polygon = polygon_ns::polygon;

bool scene_ns::my_scene::_keep_going = true;

my_scene::my_scene()
{
	frame_ctx.draw_vec = std::make_unique<drawvec_t>();
	frame_ctx.state = std::make_unique<scene_state>();
	frame_ctx.state->grfx.gfx = std::make_unique<graphics>("Software 3D Engine");
	set_defaults();
}

polygon* my_scene::add_polygon()
{
	polygons_owned.push_back(std::make_unique<polygon>());
	polygon* p = polygons_owned.back().get();
	poly_list.push_back(p);
	return p;
}

element* my_scene::add_element()
{
	elements_owned.push_back(std::make_unique<element>());
	return elements_owned.back().get();
}

void my_scene::parse(const std::string& filename, const std::string& conf_name)
{
	DBG("parse");

#ifdef USING_JSON
	ast = config_ns::parse_json(filename, conf_name);
#else
	ast = config_ns::parse_legacy(filename, conf_name);
#endif

#ifdef DEBUG_PRINTS
	DBG("parse: number of polygons: " << (int)ast.polygons.size());
	for (const auto& poly : ast.polygons) {
		DBG("parse: polygon: name: " << poly.name);
		DBG("parse: polygon: color idx: " << poly.color_index);
		DBG("parse: polygon: force: " << poly.force);
		for (const auto& vec : poly.points) {
			DBG("parse: polygon: vector:");
			vec.print();
		}
	}

	DBG("parse: number of elements: " << (int)ast.elements.size());
	for (const auto& elem : ast.elements) {
		DBG("parse: element: name: " << elem.name);
		DBG("parse: element: parent: " << elem.parent);
		DBG("parse: element: active: " << elem.active);
		DBG("parse: element: att");
		elem.ini_att.print();
		for (const auto& poly : elem.polygons) {
			DBG("parse: element: polygon: " << poly);
		}
	}
#endif
}

void my_scene::build()
{
	DBG("build");

	DBG("build: num polygon defs: " << (int)ast.polygons.size());
	for (const auto& def : ast.polygons) {
		polygon* poly = add_polygon();
		poly->init_from_def(frame_ctx, def);
		DBG("build: polygon:");
		poly->print();
	}

	DBG("build: num element defs: " << (int)ast.elements.size());
	for (const auto& def : ast.elements) {
		element* elem = add_element();
		if (!root) {
			root = elem;
		}
		elem->init_from_def(poly_list, root, def);
		DBG("build: element:");
		elem->print();
	}

	DBG("build: environment:");
	if (ast.env.color_idx.has_value()) {
		DBG("build: override: color_idx");
		frame_ctx.state->grfx.clear_color = frame_ctx.state->grfx.gfx->get_color_val(static_cast<graphics_ns::graphics::color_idx>(ast.env.color_idx.value()));
	}

	if (ast.env.light_type.has_value()) {
		DBG("build: override: light_type");
		frame_ctx.state->light.type = ast.env.light_type.value() ? light_type::point : light_type::directional;
	}

	if (ast.env.light_direction.has_value()) {
		DBG("build: override: light_direction");
		frame_ctx.state->light.direction = ast.env.light_direction.value();
	}

	if (ast.env.light_position.has_value()) {
		DBG("build: override: light_position");
		frame_ctx.state->light.position = ast.env.light_position.value();
	}

	if (ast.env.camera_position.has_value()) {
		DBG("build: override: camera_position");
		frame_ctx.state->camera.position = ast.env.camera_position.value();
	}

	if (ast.env.focal_len.has_value()) {
		DBG("build: override: focal_len");
		frame_ctx.state->proj.focal_len = ast.env.focal_len.value();
	}

	if (ast.env.near_eps.has_value()) {
		DBG("build: override: near_eps");
		frame_ctx.state->proj.near_eps = ast.env.near_eps.value();
	}

	bool vp_changed = false;
	if (ast.env.min_pos.has_value()) {
		vp_changed = true;
		DBG("build: override: 1: min_pos.x: " << DEC(frame_ctx.state->vp.min_pos.x, 4) << ", min_pos.y: " << DEC(frame_ctx.state->vp.min_pos.y, 4));
		frame_ctx.state->vp.min_pos.x = std::min(min_pos.x, ast.env.min_pos.value().x);
		frame_ctx.state->vp.min_pos.y = std::min(min_pos.y, ast.env.min_pos.value().y);
		DBG("build: override: 2: min_pos.x: " << DEC(frame_ctx.state->vp.min_pos.x, 4) << ", min_pos.y: " << DEC(frame_ctx.state->vp.min_pos.y, 4));
	}

	if (ast.env.max_pos.has_value()) {
		vp_changed = true;
		DBG("build: override: 1: max_pos.x: " << DEC(frame_ctx.state->vp.max_pos.x, 4) << ", max_pos.y: " << DEC(frame_ctx.state->vp.max_pos.y, 4));
		frame_ctx.state->vp.max_pos.x = std::min(max_pos.x, ast.env.max_pos.value().x);
		frame_ctx.state->vp.max_pos.y = std::min(max_pos.y, ast.env.max_pos.value().y);
		DBG("build: override: 2: max_pos.x: " << DEC(frame_ctx.state->vp.max_pos.x, 4) << ", max_pos.y: " << DEC(frame_ctx.state->vp.max_pos.y, 4));
	}

	if (vp_changed) {
		frame_ctx.state->vp.mid_pos.x = frame_ctx.state->vp.min_pos.x + (frame_ctx.state->vp.max_pos.x - frame_ctx.state->vp.min_pos.x) / 2;
		frame_ctx.state->vp.mid_pos.y = frame_ctx.state->vp.min_pos.y + (frame_ctx.state->vp.max_pos.y - frame_ctx.state->vp.min_pos.y) / 2;
		DBG("build: override: mid_pos.x: " << DEC(frame_ctx.state->vp.mid_pos.x, 4) << ", mid_pos.y: " << DEC(frame_ctx.state->vp.mid_pos.y, 4));
	}

	if (ast.run.loops.has_value()) {
		DBG("build: override: 1: loops: " << frame_ctx.state->run.loops);
		frame_ctx.state->run.loops = ast.run.loops.value();
		DBG("build: override: 2: loops: " << frame_ctx.state->run.loops);
	}

	if (ast.run.loop_delay.has_value()) {
		DBG("build: override: 1: loop_delay: " << frame_ctx.state->run.loop_delay);
		frame_ctx.state->run.loop_delay = ast.run.loop_delay.value();
		DBG("build: override: 2: loop_delay: " << frame_ctx.state->run.loop_delay);
	}
}

void my_scene::update()
{
	// draw list is per-frame (non-owning)
	frame_ctx.draw_vec->clear();

	// update tree + polygons; element/polygon code appends to draw_vec
	root->update_all(frame_ctx);

#ifdef DEBUG_PRINTS
	// DBG("print tree");
	// root->print_all();
#endif
}

void my_scene::sort()
{
	// clang-format off
	std::sort(frame_ctx.draw_vec->begin(), frame_ctx.draw_vec->end(),
		  [](polygon* a, polygon* b) {
			return a->get_depth() > b->get_depth();
		  });
	// clang-format on
}

void my_scene::draw()
{
	for (polygon* poly : *frame_ctx.draw_vec) {
		if (poly) {
			poly->draw(frame_ctx);
		}
	}
}

void my_scene::render()
{
	static int loops = frame_ctx.state->run.loops;

	graphics* gfx = frame_ctx.state->grfx.gfx.get();

	// Acquire backbuffer for this frame and clear it
	frame_ctx.state->grfx.fb = gfx->get_backbuffer();
	gfx->fill_buffer(frame_ctx.state->grfx.fb, frame_ctx.state->grfx.clear_color);

	if (!frame_ctx.draw_vec->empty()) {
		// DBG("sort");
		sort();
	}

	// DBG("draw");
	draw();

	// DBG("show");
	gfx->present();

	std::this_thread::sleep_for(std::chrono::milliseconds(frame_ctx.state->run.loop_delay));

	if (frame_ctx.state->run.loops > 0) {
		if (--loops < 1) {
			_keep_going = false;
		}
	}
}

bool my_scene::keep_going()
{
	return _keep_going;
}

void my_scene::set_defaults()
{
	DBG("set_default");

	// drawing vector
	frame_ctx.draw_vec->reserve(256);

	// graphics
	frame_ctx.state->grfx.clear_color = frame_ctx.state->grfx.gfx->get_color_val(graphics_ns::graphics::color_idx::black);
	DBG("clear color: " << HEX(frame_ctx.state->grfx.clear_color.c, 8));
	min_pos = frame_ctx.state->grfx.gfx->get_min_position();
	max_pos = frame_ctx.state->grfx.gfx->get_max_position();

	// light
	frame_ctx.state->light.type = light_type::directional;
	frame_ctx.state->light.direction = {LIGHT_DIR_X, LIGHT_DIR_Y, LIGHT_DIR_Z};
	frame_ctx.state->light.position = {LIGHT_POS_X, LIGHT_POS_Y, LIGHT_POS_Z};
	DBG("light type: " << (int)frame_ctx.state->light.type);
	DBG("light direction:");
	frame_ctx.state->light.direction.print();
	DBG("light position:");
	frame_ctx.state->light.position.print();

	// camera
	frame_ctx.state->camera.position = {CAM_POS_X, CAM_POS_Y, CAM_POS_Z};
	DBG("camera position:");
	frame_ctx.state->camera.position.print();

	// projection
	frame_ctx.state->proj.focal_len = FOCAL_LEN;
	frame_ctx.state->proj.near_eps = NEAR_EPS;
	DBG("projection: focal len: " << FLT(frame_ctx.state->proj.focal_len, 3) << ", near_eps: " << FLT(frame_ctx.state->proj.near_eps, 3));

	// screen
	frame_ctx.state->vp.min_pos = min_pos;
	frame_ctx.state->vp.max_pos = max_pos;
	frame_ctx.state->vp.mid_pos.x = min_pos.x + (max_pos.x - min_pos.x) / 2;
	frame_ctx.state->vp.mid_pos.y = min_pos.y + (max_pos.y - min_pos.y) / 2;

	DBG("min_pos: {" << DEC(frame_ctx.state->vp.min_pos.x, 4) << SEP << DEC(frame_ctx.state->vp.min_pos.y, 4) << "}");
	DBG("max_pos: {" << DEC(frame_ctx.state->vp.max_pos.x, 4) << SEP << DEC(frame_ctx.state->vp.max_pos.y, 4) << "}");
	DBG("mid_pos: {" << DEC(frame_ctx.state->vp.mid_pos.x, 4) << SEP << DEC(frame_ctx.state->vp.mid_pos.y, 4) << "}");

	frame_ctx.state->run.loops = DEF_LOOP_NUM;
	frame_ctx.state->run.loop_delay = DEF_LOOP_DELAY;
}

} // namespace scene_ns
