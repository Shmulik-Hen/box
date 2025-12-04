#ifndef _UNIT_H
#define _UNIT_H
#include <fstream.h>

class unit
{
	long num;

public:

	unit();
	~unit();
	unit(const long &);
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
	friend int operator>(const unit &, const unit &);
	friend int operator>=(const unit &, const unit &);
	friend int operator<(const unit &, const unit &);
	friend long convert(const char *);
	friend int mod(const long &);
	friend unit abs(const unit &);
	friend unit sin(const unit &);
	friend unit cos(const unit &);
	friend unit sqrt(const unit &);
	friend ostream &operator<<(ostream &, const unit &);
	friend istream &operator>>(istream &, unit &);
	void read(ifstream &);
};

const unit UNIT(1024);
const unit ZERO(0);
#endif
