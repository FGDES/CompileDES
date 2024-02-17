/** @file cfl_conflequiv.h 

Abstractions that maintaine conflict-equivalence.

*/

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2015  Michael Meyer and Thomnas Moor.
   Exclusive copyright is granted to Klaus Schmidt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */


#ifndef FAUDES_CONFEQUIV_H
#define FAUDES_CONFEQUIV_H

#include "cfl_definitions.h"
#include "cfl_generator.h"


namespace faudes {

/**
 * Test for conflicts
 *
 * A family of generators is non-blocking, if their parallel composition
 * is non-blocking (all accessible states are co-accessible).
 *
 * This implementation applies a number of conflict equivalent 
 * simplifications before finally testing for conflicts in the
 * parallel composition; see also  
 * ConflictEquivalentAbstraction(vGenerator&, const EventSet&)
 * This approach has been originally proposed by R. Malik  and H. Flordal 
 * in "Compositional verification in supervisory 
 * control", SIAM Journal of Control and Optimization, 2009.
 * 
 * The current implementation is experimental with code based on Michael Meyer's
 * BSc Thesis. 
 * 
 *
 * @param rGenVec
 *   Vector of input generators
 * @return res
 *   true if there are no conflicts
 *
 * @ingroup GeneratorFunctions
 */
extern FAUDES_API bool IsNonblocking(const GeneratorVector& rGenVec);


/**
 * Conflict equivalent abstraction.
 *
 * Two generators are conflict equivalent w.r.t. a set of silent events,
 * if, for any test generator defined over the not-silent events, either
 * both or non are conflicting. This functions implements a selection of
 * conflict equivalent transformations proposed by R. Malik  and H. Flordal 
 * in "Compositional verification in supervisory control", SIAM Journal of 
 * Control and Optimization, 2009.
 * 
 * The current implementation is experimental with code based on Michael Meyer's
 * BSc Thesis. 
 *
 * @param rGen
 *   Input generator
 * @param rSilentEvents
 *   Set of silent events, i.e., events not shared
 *   with any other generator to compose with.
 *
 * @ingroup GeneratorFunctions
 */
extern FAUDES_API void ConflictEquivalentAbstraction(vGenerator& rGen, const EventSet& rSilentEvents);




} // namespace faudes

#endif 

