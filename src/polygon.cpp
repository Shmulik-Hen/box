#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>

// #define DEBUG_PRINTS
#include "polygon.h"
#include "render_conventions.h"
#include "utils.h"

namespace polygon_ns
{

constexpr uint32_t LEFT_BOUNDARY_CROSSED = 0x00000001;
constexpr uint32_t RIGHT_BOUNDARY_CROSSED = 0x00000002;
constexpr uint32_t TOP_BOUNDARY_CROSSED = 0x00000004;
constexpr uint32_t BOTTOM_BOUNDARY_CROSSED = 0x00000008;

using namespace graphics_ns;
using namespace matrix_ns;
using namespace vector_3_ns;

using addr_t = graphics::addr_t;
using ARGB = graphics::ARGB;
using color_idx = graphics::color_idx;
using color_t = graphics_ns::color_t;
using frame_buffer = graphics::frame_buffer;
using graphics = graphics_ns::graphics;
using point = graphics::point;
using frame_context = scene_ns::frame_context;

using render_ns::front_facing;
using render_ns::lambert;
using vector_t = polygon_ns::polygon::vector_t;

// ---------------------------- polygon::drawing --------------------------------

void polygon::drawing::set_base_color(const graphics* gfx, int idx)
{
	_base_color = gfx->get_color_val(static_cast<color_idx>(idx));
}

void polygon::drawing::set_color(const graphics* gfx, int idx)
{
	_draw_color = gfx->get_color_val(static_cast<color_idx>(idx));
}

addr_t polygon::drawing::offset(frame_context& frame_ctx, const point& p) const
{
	// NOTE: offset assumes p is in "viewport-local" coords if vp.min_pos != {0,0}.
	// Your current convention has vp.min_pos == {0,0}.
	// clang-format off
	return reinterpret_cast<addr_t>(frame_ctx.state->grfx.fb.pixels +
					frame_ctx.state->vp.min_pos.x + p.x +
					(frame_ctx.state->vp.min_pos.y + p.y) *
					(frame_ctx.state->grfx.fb.pitch_pixels));
	// clang-format on
}

// Unchecked memory access - caller must verify viewport boundaries
void polygon::drawing::moveto(frame_context& frame_ctx, const point& p)
{
	_xy_pos = p;
	_xy_addr = offset(frame_ctx, p);
}

// Unchecked memory access - caller must verify viewport boundaries
ARGB polygon::drawing::getpixel(frame_context& frame_ctx, const point& p)
{
	moveto(frame_ctx, p);
	return *(offset(frame_ctx, p));
}

// Unchecked memory access - caller must verify viewport boundaries
void polygon::drawing::putpixel(frame_context& frame_ctx, const point& p)
{
	moveto(frame_ctx, p);
	*(offset(frame_ctx, p)) = _draw_color;
}

void polygon::drawing::clamp_to_vp(frame_context& frame_ctx, point& p)
{
	auto& vp = frame_ctx.state->vp;
	p.x = std::max(p.x, vp.min_pos.x);
	p.x = std::min(p.x, vp.max_pos.x);
	p.y = std::max(p.y, vp.min_pos.y);
	p.y = std::min(p.y, vp.max_pos.y);
	DBG("clamp_to_vp: p.x: " << DEC(p.x, 4) << ", p.y: " << DEC(p.y, 4));
}

// return a code according to which boundary is crossed
int polygon::drawing::check_boundaries(frame_context& frame_ctx, point& p)
{
	auto& vp = frame_ctx.state->vp;
	int code = 0;

	if (p.x < vp.min_pos.x) {
		code |= LEFT_BOUNDARY_CROSSED;
	}
	else if (p.x > vp.max_pos.x) {
		code |= RIGHT_BOUNDARY_CROSSED;
	}

	if (p.y < vp.min_pos.y) {
		code |= TOP_BOUNDARY_CROSSED;
	}
	else if (p.y > vp.max_pos.y) {
		code |= BOTTOM_BOUNDARY_CROSSED;
	}

	return code;
}

// Cohen–Sutherland clip line in place. Returns false if fully outside.
bool polygon::drawing::cs_clip_line(frame_context& frame_ctx, point& p1, point& p2)
{
	auto& vp = frame_ctx.state->vp;
	int c0 = check_boundaries(frame_ctx, p1);
	int c1 = check_boundaries(frame_ctx, p2);

	while (true) {
		if ((c0 | c1) == 0) {
			return true; // accept
		}
		if (c0 & c1) {
			return false; // reject
		}

		int out = c0 ? c0 : c1;
		float x = 0.0f, y = 0.0f;

		int x_gap = p2.x - p1.x;
		int y_gap = p2.y - p1.y;

		if (out & TOP_BOUNDARY_CROSSED) {
			if (y_gap == 0) {
				return false;
			}
			float t = float(vp.min_pos.y - p1.y) / float(y_gap);
			x = p1.x + t * float(x_gap);
			y = float(vp.min_pos.y);
		}
		else if (out & BOTTOM_BOUNDARY_CROSSED) {
			if (y_gap == 0) {
				return false;
			}
			float t = float(vp.max_pos.y - p1.y) / float(y_gap);
			x = p1.x + t * float(x_gap);
			y = float(vp.max_pos.y);
		}
		else if (out & LEFT_BOUNDARY_CROSSED) {
			if (x_gap == 0) {
				return false;
			}
			float t = float(vp.min_pos.x - p1.x) / float(x_gap);
			y = p1.y + t * float(y_gap);
			x = float(vp.min_pos.x);
		}
		else { // RIGHT
			if (x_gap == 0) {
				return false;
			}
			float t = float(vp.max_pos.x - p1.x) / float(x_gap);
			y = p1.y + t * float(y_gap);
			x = float(vp.max_pos.x);
		}

		if (out == c0) {
			p1.x = (int)std::lroundf(x);
			p1.y = (int)std::lroundf(y);
			c0 = check_boundaries(frame_ctx, p1);
		}
		else {
			p2.x = (int)std::lroundf(x);
			p2.y = (int)std::lroundf(y);
			c1 = check_boundaries(frame_ctx, p2);
		}
	}
}

// Unchecked memory access - caller must verify viewport boundaries
void polygon::drawing::stroke_to(frame_context& frame_ctx, point& p)
{
	int dx = p.x - _xy_pos.x;
	int dy = p.y - _xy_pos.y;

	int signx = (dx > 0) - (dx < 0);
	int signy = (dy > 0) - (dy < 0);

	int ax = std::abs(dx);
	int ay = std::abs(dy);

	int incy = signy * frame_ctx.state->grfx.fb.pitch_pixels;

	int err = ax - ay;

	while (true) {
		putdirect(_draw_color);

		if (_xy_pos.x == p.x && _xy_pos.y == p.y) {
			break;
		}

		int e2 = 2 * err;
		if (e2 > -ay) {
			err -= ay;
			_xy_pos.x += signx;
			_xy_addr += signx;
		}
		if (e2 < ax) {
			err += ax;
			_xy_pos.y += signy;
			_xy_addr += incy;
		}
	}
}

// Unchecked memory access - caller must verify viewport boundaries
void polygon::drawing::bar(scene_ns::frame_context& frame_ctx, val_t y, val_t x_left, val_t x_right)
{
	addr_t row = offset(frame_ctx, {0, y});
	std::fill(row + x_left, row + x_right + 1, _draw_color);
}

void polygon::drawing::line(frame_context& frame_ctx, point& start, point& end)
{
	point p1 = start;
	point p2 = end;
	if (cs_clip_line(frame_ctx, p1, p2)) {
		moveto(frame_ctx, p1);
		stroke_to(frame_ctx, p2);
	}
}

void polygon::drawing::rect(frame_context& frame_ctx, point& tl, point& br, bool fill)
{
	point tl_ = tl;
	point br_ = br;
	if (cs_clip_line(frame_ctx, tl_, br_)) {
		if (fill) {
			for (int y = tl_.y; y <= br_.y; y++) {
				bar(frame_ctx, y, tl_.x, br_.x);
			}
		}
		else {
			point tr_ {br_.x, tl_.y};
			point bl_ {tl_.x, br_.y};
			moveto(frame_ctx, tl_);
			stroke_to(frame_ctx, tr_);
			stroke_to(frame_ctx, br_);
			stroke_to(frame_ctx, bl_);
			stroke_to(frame_ctx, tl_);
		}
	}
}

polygon::drawing::scratchpad_t polygon::drawing::clip_polygon_to_screen(frame_context& frame_ctx, const scratchpad_t& in)
{
	auto& vp = frame_ctx.state->vp;
	const int L = vp.min_pos.x;
	const int R = vp.max_pos.x;
	const int T = vp.min_pos.y;
	const int B = vp.max_pos.y;

	if (in.size() < 3) {
		return {};
	}

	auto clip_edge = [](const scratchpad_t& poly,
	                    auto inside_fn,
	                    auto intersect_fn) -> scratchpad_t {
		scratchpad_t out;
		if (poly.empty()) {
			return out;
		}

		point S = poly.back();
		bool S_in = inside_fn(S);

		for (const point& E : poly) {
			bool E_in = inside_fn(E);

			if (E_in) {
				if (!S_in) {
					out.push_back(intersect_fn(S, E));
				}
				out.push_back(E);
			}
			else {
				if (S_in) {
					out.push_back(intersect_fn(S, E));
				}
			}

			S = E;
			S_in = E_in;
		}

		// Optional: remove consecutive duplicates introduced by clipping
		if (out.size() >= 2) {
			scratchpad_t cleaned;
			cleaned.reserve(out.size());
			cleaned.push_back(out[0]);
			for (size_t i = 1; i < out.size(); ++i) {
				if (out[i].x != out[i - 1].x || out[i].y != out[i - 1].y) {
					cleaned.push_back(out[i]);
				}
			}
			// also handle wrap-around dup
			if (cleaned.size() >= 2 &&
			    cleaned.front().x == cleaned.back().x &&
			    cleaned.front().y == cleaned.back().y) {
				cleaned.pop_back();
			}
			return cleaned;
		}

		return out;
	};

	auto intersect_x = [](const point& S, const point& E, int x_const) -> point {
		// Segment S->E intersects vertical line x=x_const.
		const int dx = E.x - S.x;
		const int dy = E.y - S.y;
		if (dx == 0) {
			// Parallel; return something stable
			return {x_const, S.y};
		}
		const double t = double(x_const - S.x) / double(dx);
		const double y = double(S.y) + t * double(dy);
		return {x_const, (int)std::lround(y)};
	};

	auto intersect_y = [](const point& S, const point& E, int y_const) -> point {
		// Segment S->E intersects horizontal line y=y_const.
		const int dx = E.x - S.x;
		const int dy = E.y - S.y;
		if (dy == 0) {
			return {S.x, y_const};
		}
		const double t = double(y_const - S.y) / double(dy);
		const double x = double(S.x) + t * double(dx);
		return {(int)std::lround(x), y_const};
	};

	scratchpad_t poly = in;

	// Left
	poly = clip_edge(
		poly,
		[L](const point& p) { return p.x >= L; },
		[intersect_x, L](const point& S, const point& E) { return intersect_x(S, E, L); });

	// Right
	poly = clip_edge(
		poly,
		[R](const point& p) { return p.x <= R; },
		[intersect_x, R](const point& S, const point& E) { return intersect_x(S, E, R); });

	// Top
	poly = clip_edge(
		poly,
		[T](const point& p) { return p.y >= T; },
		[intersect_y, T](const point& S, const point& E) { return intersect_y(S, E, T); });

	// Bottom
	poly = clip_edge(
		poly,
		[B](const point& p) { return p.y <= B; },
		[intersect_y, B](const point& S, const point& E) { return intersect_y(S, E, B); });

	// Final sanity: if we ended with <3 vertices, it’s fully clipped
	if (poly.size() < 3) {
		poly.clear();
	}

	return poly;
}

void polygon::drawing::build_bbox_from(const scratchpad_t& poly)
{
	// poly is already clipped to viewport, so bbox should be valid.
	point mn = poly[0];
	point mx = poly[0];

	for (size_t i = 1; i < poly.size(); ++i) {
		mn.x = std::min(mn.x, poly[i].x);
		mn.y = std::min(mn.y, poly[i].y);
		mx.x = std::max(mx.x, poly[i].x);
		mx.y = std::max(mx.y, poly[i].y);
	}

	_bbox_tl = mn;
	_bbox_br = mx;

	// Prepare xlist storage for [bbox_tl.y .. bbox_br.y]
	const int h = (_bbox_br.y - _bbox_tl.y + 1);
	_xlist.clear();
	if (h > 0) {
		_xlist.resize((size_t)h);
	}
}

void polygon::drawing::build_xlist_from(const scratchpad_t& poly)
{
	if (poly.size() < 3) {
		_xlist.clear();
		return;
	}

	const int y0 = _bbox_tl.y;
	const int y1 = _bbox_br.y;

	// clear rows
	for (auto& row : _xlist) {
		row.clear();
	}

	const size_t n = poly.size();
	for (size_t i = 0; i < n; ++i) {
		const point& a = poly[i];
		const point& b = poly[(i + 1) % n];

		// Skip horizontal edges (do not contribute to scanline intersections)
		if (a.y == b.y) {
			continue;
		}

		// Enforce top-inclusive, bottom-exclusive rule:
		// include scanlines y such that y in [y_min, y_max)
		const point& top = (a.y < b.y) ? a : b;
		const point& bottom = (a.y < b.y) ? b : a;

		int y_min = top.y;
		int y_max = bottom.y; // exclusive

		// Clamp to bbox vertically
		int ys = std::max(y_min, y0);
		int ye = std::min(y_max - 1, y1);
		if (ys > ye) {
			continue;
		}

		const int dy = (bottom.y - top.y); // > 0
		const int dx = (bottom.x - top.x);

		for (int y = ys; y <= ye; ++y) {
			// Intersection at scanline y:
			// x = x0 + (y - y0) * dx / dy
			const double t = double(y - top.y) / double(dy);
			const double xf = double(top.x) + t * double(dx);
			const int x = (int)std::lround(xf);

			const int row = y - y0;
			if ((unsigned)row < (unsigned)_xlist.size()) {
				_xlist[(size_t)row].push_back(x);
			}
		}
	}
}

void polygon::drawing::fill_from_xlist(scene_ns::frame_context& frame_ctx)
{
	auto& vp = frame_ctx.state->vp;
	const int y_base = _bbox_tl.y;

	for (int row = 0; row < (int)_xlist.size(); ++row) {
		const int y = y_base + row;
		auto& xs = _xlist[(size_t)row];

		if (xs.size() < 2) {
			continue;
		}

		std::sort(xs.begin(), xs.end());

		// Defensive: ensure even count
		if (xs.size() & 1U) {
			xs.pop_back();
		}

		for (size_t i = 0; i + 1 < xs.size(); i += 2) {
			int xl = xs[i];
			int xr = xs[i + 1];
			if (xl > xr) {
				std::swap(xl, xr);
			}

			// Clamp to viewport
			xl = std::max(xl, vp.min_pos.x);
			xr = std::min(xr, vp.max_pos.x);

			if (xl <= xr) {
				bar(frame_ctx, y, xl, xr);
			}
		}
	}
}

// --------------------------- drawing::draw ------------------------------------

void polygon::drawing::draw(frame_context& frame_ctx)
{
	if (_scratch_pad.size() < 3 || _invalid) {
		_invalid = false;
		_scratch_pad.clear();
		return;
	}

	_clip_poly = clip_polygon_to_screen(frame_ctx, _scratch_pad);
	if (_clip_poly.size() < 3) {
		_scratch_pad.clear();
		return;
	}

	build_bbox_from(_clip_poly);
#ifndef DEBUG_POLYGON
	build_xlist_from(_clip_poly);
	fill_from_xlist(frame_ctx);
#else
	point f = _fill_point;
	point n = _normal_point;
	point p0, p1, p2;
	clamp_to_vp(frame_ctx, n);

	// draw the vp
	set_color(frame_ctx.state->grfx.gfx.get(), _vp_color);
	p1 = frame_ctx.state->vp.min_pos;
	p2 = frame_ctx.state->vp.max_pos;
	rect(frame_ctx, p1, p2, false);

	// draw the bbox
	set_color(frame_ctx.state->grfx.gfx.get(), _bbox_color);
	p1 = _bbox_tl;
	p2 = _bbox_br;
	rect(frame_ctx, p1, p2, false);

	// draw the filling point
	set_color(frame_ctx.state->grfx.gfx.get(), _fill_color);
	putpixel(frame_ctx, {f.x - 1, f.y});
	putpixel(frame_ctx, {f.x, f.y});
	putpixel(frame_ctx, {f.x + 1, f.y});
	putpixel(frame_ctx, {f.x, f.y - 1});
	putpixel(frame_ctx, {f.x, f.y});
	putpixel(frame_ctx, {f.x, f.y + 1});

	// draw the normal
	putpixel(frame_ctx, n);
	set_color(frame_ctx.state->grfx.gfx.get(), _normal_color);
	line(frame_ctx, f, n);

	// draw the outline (original projected poly, not clipped)
	set_color(frame_ctx.state->grfx.gfx.get(), _debug_color);
	p0 = p1 = _scratch_pad[0];
	for (unsigned int i = 1; i < _scratch_pad.size(); i++) {
		p2 = _scratch_pad[i];
		line(frame_ctx, p1, p2);
		p1 = p2;
	}
	line(frame_ctx, p2, p0);
#endif

	_scratch_pad.clear();
}

void polygon::drawing::clear_scratch_pad()
{
	_scratch_pad.clear();
}

void polygon::drawing::transform(frame_context& frame_ctx, const vector_3& original, vector_3& transformed, const int forward_sign, const bool force)
{
	transformed = _trans_mat * original;

	unit dz = transformed.get(Z_) - frame_ctx.state->camera.position.get(Z_);
	unit depth = (unit)forward_sign * dz;

	if (!force && (depth < frame_ctx.state->proj.near_eps)) {
		_invalid = true;
	}
}

void polygon::drawing::project(frame_context& frame_ctx, const vector_3& transformed, point& p)
{
	vector_3 projected = vector_3::project(transformed,
	                                       frame_ctx.state->camera.position,
	                                       frame_ctx.state->proj.focal_len,
	                                       frame_ctx.state->proj.near_eps,
	                                       FORWARD_LOOKING_SIGN);
	p.x = std::lroundf((unit)frame_ctx.state->vp.mid_pos.x + projected.get(X_));
	p.y = std::lroundf((unit)frame_ctx.state->vp.mid_pos.y - projected.get(Y_));
	DBG("project: p.x: " << DEC(p.x, 4) << ", p.y: " << DEC(p.y, 4));
}

void polygon::drawing::make_color(unit u)
{
	u = std::min(u, UNIT);
	unit factor = u / UNIT;

	_draw_color.c = _base_color.c;

	_draw_color.channel.r = (uint8_t)std::lroundf((unit)_base_color.channel.r * factor);
	_draw_color.channel.g = (uint8_t)std::lroundf((unit)_base_color.channel.g * factor);
	_draw_color.channel.b = (uint8_t)std::lroundf((unit)_base_color.channel.b * factor);
}

// ---------------------- end of class polygon::drawing -------------------------

polygon::polygon()
{
	_draw_ctx = new drawing();
}

polygon::~polygon()
{
	if (_draw_ctx) {
		delete _draw_ctx;
		_draw_ctx = nullptr;
	}

	_points.clear();
}

static unit len2(const vector_3& v)
{
	const unit x = v.get(X_);
	const unit y = v.get(Y_);
	const unit z = v.get(Z_);
	return x * x + y * y + z * z;
}

static unit dist2(const vector_3& a, const vector_3& b)
{
	const unit dx = a.get(X_) - b.get(X_);
	const unit dy = a.get(Y_) - b.get(Y_);
	const unit dz = a.get(Z_) - b.get(Z_);
	return dx * dx + dy * dy + dz * dz;
}

static unit max_radius_from(const vector_t& points, const vector_3& p0)
{
	unit max_d2 = ZERO;

	for (const auto& p : points) {
		vector_3 d = p - p0;
		max_d2 = std::max(max_d2, len2(d));
	}

	return std::sqrt(max_d2);
}

bool polygon::is_consec()
{
	const std::size_t n = _points.size();

	// Choose a tolerance. Using EPSILON in world units can be too small or too big,
	// but this is a safe start.
	const unit eps2 = (EPSILON * CONSEC_TOLERANCE_SCALE) * (EPSILON * CONSEC_TOLERANCE_SCALE);

	// 1) No consecutive duplicates (including wraparound)
	for (std::size_t i = 0; i < n; ++i) {
		vector_3& a = _points[i];
		vector_3& b = _points[(i + 1) % n];
		if (dist2(a, b) <= eps2) {
			return false;
		}
	}

	// 2) No duplicates anywhere (recommended)
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = i + 1; j < n; ++j) {
			if (dist2(_points[i], _points[j]) <= eps2) {
				return false;
			}
		}
	}

	return true;
}

bool polygon::is_degenerate()
{
	vector_3 n = find_normal(); // already handles n==3 vs Newell
	return (len2(n) <= EPSILON * EPSILON);
}

bool polygon::is_planar()
{
	vector_3 p0 = _points[0];
	vector_3 N = find_normal();

	// Scale tolerance to polygon size to avoid false negatives on large coordinates.
	// radius ~ "size" of polygon in world units.
	const unit radius = max_radius_from(_points, p0);
	const unit tol = std::max(EPSILON * 10, EPSILON * radius);

	for (const auto& p : _points) {
		vector_3 d = p - p0;
		const unit dist = vector_3::dot(N, d); // signed distance scaled by |N| (|N| == 1)
		if (std::abs(dist) > tol) {
			return false;
		}
	}

	return true;
}

bool polygon::verify()
{
	if (!_name.length()) {
		ERR("polygon: no name");
		return false;
	}

	if (_points.size() < 3) {
		ERR("polygon: need at least 3 vectors " << DEC(_points.size(), 4));
		return false;
	}

	if (!is_consec()) {
		ERR("polygon: non-consecutive");
		return false;
	}

	if (is_degenerate()) {
		ERR("polygon: degenerate");
		return false;
	}

	if (_points.size() > 3 && !is_planar()) {
		ERR("polygon: not planar");
		return false;
	}

	return true;
}

vector_3 polygon::find_fill()
{
	vector_3 v;
	unit n = static_cast<unit>(_points.size());

	if (n) {
		for (const auto& vec : _points) {
			v += vec;
		}

		v.set(X_, v.get(X_) / n);
		v.set(Y_, v.get(Y_) / n);
		v.set(Z_, v.get(Z_) / n);
	}

	return v;
}

vector_3 polygon::find_normal()
{
	const std::size_t n = _points.size();

	if (n == 3) {
		// --- Fast/accurate path for triangles ---
		vector_3& p0 = _points[0];
		vector_3& p1 = _points[1];
		vector_3& p2 = _points[2];

		vector_3 e1 = p1 - p0;
		vector_3 e2 = p2 - p0;

		vector_3 cr = vector_3::cross(e1, e2);

		return vector_3::normalize(cr);
	}
	else {
		// --- Robust path for 4+ vertices: Newell's method ---
		vector_3 nn(ZERO, ZERO, ZERO);

		for (std::size_t i = 0; i < n; ++i) {
			vector_3& cur = _points[i];
			vector_3& nxt = _points[(i + 1) % n];

			const unit cx = cur.get(X_);
			const unit cy = cur.get(Y_);
			const unit cz = cur.get(Z_);

			const unit nx = nxt.get(X_);
			const unit ny = nxt.get(Y_);
			const unit nz = nxt.get(Z_);

			nn.set(X_, nn.get(X_) + (cy - ny) * (cz + nz));
			nn.set(Y_, nn.get(Y_) + (cz - nz) * (cx + nx));
			nn.set(Z_, nn.get(Z_) + (cx - nx) * (cy + ny));
		}

		return vector_3::normalize(nn);
	}
}

void polygon::init_from_def(frame_context& ctx, const config_ns::polygon_def& def)
{
	_name = def.name;
	_draw_ctx->set_base_color(ctx.state->grfx.gfx.get(), def.color_index);
#ifdef DEBUG_POLYGON
	_force = true;
#else
	_force = def.force;
#endif

	_points.clear();
	for (const auto& v : def.points) {
		_points.push_back(v);
	}

	if (!verify()) {
		sys_error("polygon::init_from_def: verify failed: ", _name.c_str());
	}

	// compute fill + normal from geometry
	_fill = find_fill();
	_normal = find_normal();

	// optional: if cfg normal exists, assert winding matches during transition
	if (def.normal_cfg.has_value()) {
		_config_normal = vector_3::normalize(def.normal_cfg.value());
		if (vector_3::dot(_normal, _config_normal) < ZERO) {
			// either flip or warn; I recommend warn+flip until old configs cleaned
			WARN("computed normal != configuration normal. flipping");
			_normal = _normal * -UNIT;
		}
	}
}

bool polygon::read(const graphics* gfx, std::ifstream& ifs)
{
	LINE line;
	vector_3* v;
	int len;
	bool rc, finish = false;

	while (!ifs.eof() && !finish) {
		while ((!read_word(ifs, line)) && (!ifs.eof()));
		if (ifs.eof()) {
			break;
		}

		switch (line[1]) {
		case 'n':
			len = read_word(ifs, line);
			if (!len) {
				sys_error("polygon::read error polygon");
			}

			_name = line;
			break;
		case 'c':
			len = read_word(ifs, line);
			if (len) {
				int color = atoi(line);
				_draw_ctx->set_base_color(gfx, color);
			}
			else {
				sys_error("polygon::read error polygon");
			}
			break;
		case 'f':
			len = read_word(ifs, line);
			if (len) {
#ifdef DEBUG_POLYGON
				_force = true;
#else
				int f = atoi(line);
				_force = (f ? true : false);
#endif
			}
			else {
				sys_error("polygon::read error polygon");
			}
			break;
		case 'o': // TODO: FIX
			_config_normal.read(ifs);
			break;
		case 'v':
			v = new vector_3;
			rc = v->read(ifs);
			if (!rc) {
				delete v;
				sys_error("polygon::read error polygon");
			}
			_points.push_back(*v);
			delete v;
			break;
		default:
			finish = true;
			ifs.seekg(-4, std::ios::cur);
			break;
		}
	}

	if (!verify()) {
		ERR("polygon: invalid polygon");
		return false;
	}

	_fill = find_fill();
	_normal = find_normal();

	unit d = vector_3::dot(_normal, _config_normal);
	if (d < ZERO) {
		WARN("polygon::read: winding mismatch vs config normal" << _name);
		DBG("polygon::read: config normal:");
		_config_normal.print();
		DBG("polygon::read: calculated normal:");
		_normal.print();
	}

	return true;
}

void polygon::print() const
{
#ifdef DEBUG_PRINTS
	DBG("polygon:");
	DBG(STR("  name: ", 1) << _name);
	DBG(STR("  force: ", 1) << DEC(_force, 4));
	DBG(STR("  fill:", 1));
	_fill.print();
	DBG(STR("  normal:", 1));
	_normal.print();
	DBG(STR("  config normal:", 1));
	_config_normal.print();
	DBG(STR("  draw color: ", 1) << HEX(_draw_ctx->_base_color.c, 8));
	DBG(STR("  depth: ", 1) << _depth);
	if (!_points.empty()) {
		DBG("  points:");
		for (const auto& vec : _points) {
			DBG(STR("  point:", 1));
			vec.print();
		}
	}
#endif // DEBUG_PRINTS
}

void polygon::draw(frame_context& frame_ctx)
{
	vector_3 t;
	point p;

	_draw_ctx->clear_scratch_pad();
	for (const auto& v : _points) {
		_draw_ctx->transform(frame_ctx, v, t, FORWARD_LOOKING_SIGN, _force);
		_draw_ctx->project(frame_ctx, t, p);
		_draw_ctx->_scratch_pad.push_back(p);
	}

#ifdef DEBUG_POLYGON
	_draw_ctx->project(frame_ctx, _draw_ctx->_debug_normal, _draw_ctx->_normal_point);
	_draw_ctx->project(frame_ctx, _draw_ctx->_debug_fill, _draw_ctx->_fill_point);
#endif

	_draw_ctx->draw(frame_ctx);
}

void polygon::update(matrix& m_trans, matrix& m_rot, frame_context& frame_ctx)
{
	vector_3 dist, fill, normal, L;
	unit atten = UNIT;

	_draw_ctx->_trans_mat = m_trans;
	_draw_ctx->_rot_mat = m_rot;

	fill = m_trans * _fill;
	normal = m_rot * _normal;

	// Convention: V is camera → surface
	// normals face camera when dot(N,V) < 0
	vector_3 V = vector_3::normalize(fill - frame_ctx.state->camera.position);
	if (!front_facing(normal, V, _force)) {
		return;
	}

	if (frame_ctx.state->light.type == scene_ns::light_type::directional) {
		// Convention: light.direction is surface->light
		L = vector_3::normalize(frame_ctx.state->light.direction);
	}
	else {
		// Point light: surface->light
		vector_3 to_light = frame_ctx.state->light.position - fill; // surface -> light

		// Distance attenuation
		const unit r2 = vector_3::dot(to_light, to_light);
		// Tune k to your world scale; start here and adjust
		const unit k = DIST_ATTEN;
		atten = UNIT / (UNIT + k * r2);

		L = vector_3::normalize(to_light);
	}

	unit light_angle = lambert(normal, L);
	unit intensity = std::abs(light_angle) * atten;
	if ((intensity > EPSILON) || _force) {
		dist = fill - frame_ctx.state->camera.position;
		_depth = vector_3::dot(dist, dist);
		_draw_ctx->make_color(std::abs(intensity));
		frame_ctx.draw_vec->push_back(this);
	}

#ifdef DEBUG_POLYGON
	_draw_ctx->_debug_fill = fill;
	_draw_ctx->_debug_normal = fill + normal * NORMAL_SCALE_FACTOR;
#endif
}

} // namespace polygon_ns
