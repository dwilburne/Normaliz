/*
 * Normaliz 2.7
 * Copyright (C) 2007-2011  Winfried Bruns, Bogdan Ichim, Christof Soeger
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <cassert>
#include <iostream>
#include "HilbertSeries.h"

//---------------------------------------------------------------------------

namespace libnormaliz {
using std::cout; using std::endl;

template <typename Class>
ostream& operator<< (ostream& out, const vector<Class>& vec) {
	for (size_t i=0; i<vec.size(); ++i) {
		out << " " << vec[i];
	}
	out << endl;
	return out;
}


// Constructor, creates 0/1
template<typename Integer>
HilbertSeries<Integer>::HilbertSeries() {
	nom   = vector<Integer>(1,0);
	denom = vector<Integer>(1,0);
}

// Constructor, creates nom/denom, see class description for format
template<typename Integer>
HilbertSeries<Integer>::HilbertSeries(const vector<Integer>& nominator, const vector<Integer>& denominator) {
	nom   = nominator;
	denom = denominator;
}

// add another HilbertSeries to this
template<typename Integer>
HilbertSeries<Integer>& HilbertSeries<Integer>::operator+=(const HilbertSeries<Integer>& other) {
	vector<Integer> other_nom = other.nom;
	vector<Integer> other_denom = other.denom;

	// adjust denominators
	if (denom.size() < other_denom.size()) 
		denom.resize(other_denom.size());
	else if (denom.size() > other_denom.size())
		other_denom.resize(denom.size());
	size_t d_size = denom.size();
	int diff;
	for (size_t i=1; i<d_size; ++i) {
		diff = denom[i] - other_denom[i];
		if (diff > 0) {        // augment other
			other_denom[i] += diff;
			poly_mult_to(other_nom, i, diff);
		} else if (diff < 0) { // augment this
			diff = -diff;
			denom[i] += diff;
			poly_mult_to(nom, i, diff);
		}
	}
	assert (denom == other_denom);

	// now just add the nominators
	poly_add_to(nom,other_nom);

	return (*this);
}


// simplify, see class description
template<typename Integer>
void HilbertSeries<Integer>::simplify() {

	//TODO check for canceling

	//TODO decompose denominator into cyclotomic polynomials
}

// returns the nominator, repr. as vector of coefficients, the h-vector
template<typename Integer>
const vector<Integer>& HilbertSeries<Integer>::getNominator() const {
	return nom;
}
// returns the denominator, repr. as a vector of the exponents of (1-t^i)^e
template<typename Integer>
const vector<Integer>& HilbertSeries<Integer>::getDenominator() const {
	return denom;
}


template<typename Integer>
ostream& operator<< (ostream& out, const HilbertSeries<Integer>& HS) {
	out << "(";
    for (size_t i=0; i<HS.nom.size(); ++i) {
		if ( HS.nom[i]!=0 ) out << " +("<<HS.nom[i]<<")*t^"<<i;
	}
	out << " ) / (";
    for (size_t i=1; i<HS.denom.size(); ++i) {
		if ( HS.denom[i]!=0 ) out << " (1-t^"<<i<<")^"<<HS.denom[i];
	}
	out << " )" << std::endl;
	return out;
}

//---------------------------------------------------------------------------
// polynomial operations, for polynomials repr. as vector of coefficients
//---------------------------------------------------------------------------

// returns the coefficient vector of 1-t^i
template<typename Integer>
vector<Integer> coeff_vector(size_t i) {
	cout << "create 1-t^"<<i<<" :   ";
	vector<Integer> p(i+1,0);
	p[0] =  1;
	p[i] = -1;
	cout<<p[0]<<p[i]<<"*t^"<<i<<"   size="<<p.size() <<endl;
}

template<typename Integer>
void remove_zeros(vector<Integer>& a) {
	int i=a.size()-1;
	while ( i>0 && a[i]==0 ) --i;
	if (i<a.size()-1) {
		cout<<"resizing to "<<i+1<<endl;
		a.resize(i+1);
	}
}

// a += b  (also possible to define the += op for vector)
template<typename Integer>
void poly_add_to (vector<Integer>& a, const vector<Integer>& b) {
	size_t b_size = b.size();
	if (a.size() < b_size) {
		a.resize(b_size);
	}
	for (size_t i=0; i<b_size; ++i) {
		a[i]+=b[i];
	}
	remove_zeros(a);
}
// a -= b  (also possible to define the += op for vector)
template<typename Integer>
void poly_sub_to (vector<Integer>& a, const vector<Integer>& b) {
	size_t b_size = b.size();
	if (a.size() < b_size) {
		a.resize(b_size);
	}
	for (size_t i=0; i<b_size; ++i) {
		a[i]-=b[i];
	}
	remove_zeros(a);
}

// a * b
template<typename Integer>
vector<Integer> poly_mult(const vector<Integer>& a, const vector<Integer>& b) {
	size_t a_size = a.size();
	size_t b_size = b.size();
	cout << a_size<<"+"<<b_size<<" oder "<<b.size()<<endl;
	vector<Integer> p( a_size + b_size - 1 );
	size_t i,j;
	for (i=0; i<a_size; ++i) {
		if (a[i] == 0) continue;
		for (j=0; j<a_size; ++j) {
			if (b[j] == 0) continue;
			p[i+j] += a[i]*b[j];
		}
	}
	return p;
}

// a *= (1-t^i)^e
template<typename Integer>
void poly_mult_to(vector<Integer>& a, size_t d, size_t e = 1) {
	assert(d>0);
	int i;
	a.reserve(a.size() + d*e);
	while (e>0) {
		a.resize(a.size() + d);
		for (i=a.size(); i>=d; --i) {
			a[i] -= a[i-d];
		}
		e--;
	}
}

// division with remainder, a = q * b + r, deg(r) < deg(b), needs |leadcoef(b)| = 1
template<typename Integer>
void poly_div(vector<Integer>& q, vector<Integer>& r, const vector<Integer>& a, const vector<Integer>&b) {
	assert(b.back()!=0); // no unneeded zeros
	assert(b.back()==1 || b.back()==-1);
	r = a;
	remove_zeros(r);
	size_t b_size = b.size();
	int degdiff = r.size()-b_size; //degree differenz
	if (degdiff<0) {
		q = vector<Integer>();
	} else {
		q = vector<Integer>(degdiff+1);
	}
	Integer divisor;
	size_t i=0;

	while (degdiff >= 0) {
		divisor = r.back()/b.back();
		q[degdiff] = divisor;
		// r -= divisor * t^degdiff * b
		for (i=0; i<b_size; ++i) {
			r[i+degdiff] -= divisor * b[i];
		}
		remove_zeros(r);
		degdiff = r.size()-b_size;
	}

	return;
}

} //end namespace libnormaliz
