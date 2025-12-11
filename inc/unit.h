#ifndef __UNIT_H__
#define __UNIT_H__

#include <fstream>
#include "common.h"

namespace unit_ns
{

using std::ifstream;
using std::istream;
using std::ostream;

class unit
{
	long num{0};

public:

	unit() {};
	unit(const long &);
	~unit() {};
	unit operator+(const unit &) const;
	unit operator-(const unit &) const;
	unit operator*(const unit &) const;
	unit operator/(const unit &) const;
	unit operator-() const;
	unit &operator+=(const unit &);
	unit &operator-=(const unit &);
	unit &operator*=(const unit &);
	unit &operator/=(const unit &);
	operator int();
	operator long();
	bool read(ifstream &);
	void print() const;

	friend long convert(const char *);
	friend int operator>(const unit &, const unit &);
	friend int operator>=(const unit &, const unit &);
	friend int operator<(const unit &, const unit &);
	friend int mod(const long &);
	friend unit abs(const unit &);
	friend unit sin(const unit &);
	friend unit cos(const unit &);
	friend unit sqrt(const unit &);
	friend ostream &operator<<(ostream &, const unit &);
	friend istream &operator>>(istream &, unit &);
};

const unit UNIT(1024);
const unit ZERO(0);

} // namespace unit_ns
#endif //__UNIT_H__
