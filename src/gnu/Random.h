// This may look like C code, but it is really -*- C++ -*-
/* 
Copyright (C) 1988 Free Software Foundation
    written by Dirk Grunwald (grunwald@cs.uiuc.edu)

This file is part of the GNU C++ Library.  This library is free
software; you can redistribute it and/or modify it under the terms of
the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.  This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef _Random_h
#define _Random_h 1
#ifdef __GNUG__
//#pragma interface
#endif

#include <math.h>

#if MAC
	#define Random gnu_Random
#endif

#include <RNG.h>
#include <RNG_random123.h>

class Random {
protected:
    RNG *pGenerator;
public:
    Random(RNG *generator);
    virtual ~Random() {}
    virtual double operator()() = 0;

    RNG *generator();
    void generator(RNG *p);
};


inline Random::Random(RNG *gen)
{
    pGenerator = gen;
}

inline RNG *Random::generator()
{
    return(pGenerator);
}

inline void Random::generator(RNG *p)
{
    pGenerator = p;
}

class Random_random123 {
protected:
    RNG_random123 *pGenerator;
public:
    Random_random123(RNG_random123 *generator);
    virtual ~Random_random123() {}
    virtual double operator()() = 0;

    RNG_random123 *generator();
    void generator(RNG_random123 *p);
};


inline Random_random123::Random_random123(RNG_random123 *gen)
{
    pGenerator = gen;
}

inline RNG_random123 *Random_random123::generator()
{
    return(pGenerator);
}

inline void Random_random123::generator(RNG_random123 *p)
{
    pGenerator = p;
}

#endif
