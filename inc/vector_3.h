#pragma once

#include "common.h"
#include "unit.h"

namespace vector_3_ns
{

const int dim = Z_ + 1;

class vector_3
{
private:

	unit _coords[dim];
	bool is_valid_coord(int i) const { return (i >= X_ && i <= Z_); }

public:

	vector_3();
	vector_3(const vector_3&);
	vector_3(const unit[dim]);
	vector_3(const unit, const unit, const unit);
	~vector_3() = default;

	bool read(std::ifstream&);
	void print() const;
	unit get(int) const;
	void set(const int, const unit&);
	static unit dot(const vector_3&, const vector_3&);
	static vector_3 cross(const vector_3&, const vector_3&);
	static vector_3 project(const vector_3&, const vector_3&);
	static vector_3 project(const vector_3&, const vector_3&, unit, unit, int); // pinhole projection
	static vector_3 normalize(const vector_3&);

	vector_3 operator+(const vector_3&) const;
	vector_3 operator-(const vector_3&) const;
	vector_3 operator*(const unit) const;
	vector_3& operator=(const vector_3&) = default;
	vector_3& operator+=(const vector_3&);
	vector_3& operator-=(const vector_3&);

	friend std::ostream& operator<<(std::ostream&, const vector_3&);
	friend std::istream& operator>>(std::istream&, vector_3&);
};

} // namespace vector_3_ns
