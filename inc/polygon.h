#pragma once

#include <list>
#include <utility>
#include <vector>

#include "common.h"
#include "graphics.h"
#include "matrix.h"
#include "unit.h"
#include "vector_3.h"
#include "scene_types.h"
#include "config_document.h"

// clang-format off
namespace graphics_ns { class graphics; }
namespace matrix_ns { class matrix; }
namespace vector_3_ns { class vector_3; }
// clang-format on

namespace polygon_ns
{

// #define DEBUG_POLYGON

class polygon
{
	class drawing
	{
	public:

		// ---- types ----
		using point = graphics_ns::graphics::point;
		using scratchpad_t = std::vector<point>;

		// For scanline fill: list of X intersections per row (y)
		using xrow_t = std::vector<int>;
		using xlist_t = std::vector<xrow_t>;

		// ---- data ----

		// projected polygon (screen coords, before clipping)
		scratchpad_t _scratch_pad;

		// polygon after clipping to screen rect (screen coords)
		scratchpad_t _clip_poly;

		// x-intersections per scanline row (relative to _bbox_tl.y)
		xlist_t _xlist;

		bool _invalid {false};

		// current point (used by stroke routines)
		graphics_ns::graphics::addr_t _xy_addr {nullptr};
		point _xy_pos;

		// bounding box (of clipped polygon)
		point _bbox_tl;
		point _bbox_br;

		// color
		graphics_ns::graphics::ARGB _base_color {};
		graphics_ns::graphics::ARGB _draw_color {};

		// translate/rotate matrices
		matrix_ns::matrix _trans_mat;
		matrix_ns::matrix _rot_mat;

#ifdef DEBUG_POLYGON
		vector_3_ns::vector_3 _debug_normal;
		vector_3_ns::vector_3 _debug_fill;
		graphics_ns::graphics::color_idx _debug_color {graphics_ns::graphics::white};
		graphics_ns::graphics::color_idx _bbox_color {graphics_ns::graphics::green};
		graphics_ns::graphics::color_idx _vp_color {graphics_ns::graphics::blue};
		graphics_ns::graphics::color_idx _normal_color {graphics_ns::graphics::lime};
		graphics_ns::graphics::color_idx _fill_color {graphics_ns::graphics::red};
		point _normal_point;
		point _fill_point;
#endif

		drawing() = default;
		~drawing() = default;

		void set_base_color(const graphics_ns::graphics*, int);
		void set_color(const graphics_ns::graphics*, int);

		graphics_ns::graphics::addr_t offset(scene_ns::frame_context&, const point&) const;
		void moveto(scene_ns::frame_context&, const point&);

		graphics_ns::graphics::ARGB getdirect() const { return *_xy_addr; }
		void putdirect(const graphics_ns::graphics::ARGB& a) { *_xy_addr = a; }

		graphics_ns::graphics::ARGB getpixel(scene_ns::frame_context&, const point&);
		void putpixel(scene_ns::frame_context&, const point&);

		// Fast scanline bar (no per-pixel bounds checks)
		void bar(scene_ns::frame_context&, graphics_ns::val_t y, graphics_ns::val_t x_left, graphics_ns::val_t x_right);

		// Optional: keep for debug outline rendering. Not used for filling anymore.
		void clamp_to_vp(scene_ns::frame_context&, point&);
		int check_boundaries(scene_ns::frame_context&, point&);
		bool cs_clip_line(scene_ns::frame_context&, point&, point&);

		void stroke_to(scene_ns::frame_context&, point&);
		void line(scene_ns::frame_context&, point&, point&);
		void rect(scene_ns::frame_context&, point&, point&, bool fill = true);

		void clear_scratch_pad();
		void transform(scene_ns::frame_context&, const vector_3_ns::vector_3&, vector_3_ns::vector_3&, const int, const bool);
		void project(scene_ns::frame_context&, const vector_3_ns::vector_3&, point&);
		void make_color(unit);

		void draw(scene_ns::frame_context&);

	private:

		// Clip polygon to screen rect (Sutherland–Hodgman)
		scratchpad_t clip_polygon_to_screen(scene_ns::frame_context&, const scratchpad_t& in);

		// bbox from clipped poly
		void build_bbox_from(const scratchpad_t& poly);

		// build x intersections per y (top-left rule)
		void build_xlist_from(const scratchpad_t& poly);

		// fill from xlist (sort and fill pairs)
		void fill_from_xlist(scene_ns::frame_context&);
	}; // class polygon::drawing

	drawing* _draw_ctx {nullptr}; // raw pointer, non owning

	std::string _name;
	vector_3_ns::vector_3 _fill;          // polygon middle point - average of all vectors
	vector_3_ns::vector_3 _config_normal; // polygon's direction - from config
	vector_3_ns::vector_3 _normal;        // polygon's direction - calculated
	double _depth {ZERO};                 // objects location on the z axis
	bool _force {false};                  // display polygon regardless of direction

	bool is_consec();
	bool is_degenerate();
	bool is_planar();
	bool verify();
	vector_3_ns::vector_3 find_fill();
	vector_3_ns::vector_3 find_normal();

public:

	typedef std::vector<vector_3_ns::vector_3> vector_t;
	typedef std::list<polygon*> polylist_t;
	using frame_context = scene_ns::frame_context;

	polygon();
	~polygon();

	const std::string get_name() const { return _name; }
	double get_depth() const { return _depth; }

	void init_from_def(frame_context&, const config_ns::polygon_def&);
	bool read(const graphics_ns::graphics*, std::ifstream&);
	void print() const;
	void draw(scene_ns::frame_context&);
	void update(matrix_ns::matrix&, matrix_ns::matrix&, scene_ns::frame_context&);

private:

	vector_t _points; // owning
};

} // namespace polygon_ns
