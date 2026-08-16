#include "search_params.hpp"
#include <cassert>
#include <cstring>

// S073. The table and the accessors; the values themselves are declared by the
// list in search_params.hpp and are constants or variables depending on the
// build.


#ifdef CHESSO_TUNE
#define CHESSO_DEFINE_SEARCH_PARAM(sym, name, def, lo, hi) int sym = def;
CHESSO_SEARCH_PARAMS(CHESSO_DEFINE_SEARCH_PARAM)
#undef CHESSO_DEFINE_SEARCH_PARAM
#endif


// Unconditional: the same rows in both builds, which is the whole point of the
// file. Nothing here reads a parameter's live value.
#define CHESSO_SEARCH_PARAM_ROW(sym, name, def, lo, hi) {name, def, lo, hi},

static const search_param_t param_table[] = {
    CHESSO_SEARCH_PARAMS(CHESSO_SEARCH_PARAM_ROW)};

#undef CHESSO_SEARCH_PARAM_ROW


// The live values, in the same order. `const int*` covers both builds: the
// default build points at the `inline constexpr` constants, the tune build at
// the variables. Taking their addresses here does not stop the constants
// folding at the use sites in search.cpp, which is where they are read.
#define CHESSO_SEARCH_PARAM_ADDRESS(sym, name, def, lo, hi) &sym,

static const int* const param_values[] = {
    CHESSO_SEARCH_PARAMS(CHESSO_SEARCH_PARAM_ADDRESS)};

#undef CHESSO_SEARCH_PARAM_ADDRESS


size_t search_param_count()
{ return sizeof(param_table) / sizeof(param_table[0]); }


const search_param_t& search_param_info(size_t index)
{
  assert(index < search_param_count());
  return param_table[index];
}


int search_param_value(size_t index)
{
  assert(index < search_param_count());
  return *param_values[index];
}


#ifdef CHESSO_TUNE

// Mutable aliases of the same objects param_values points at. Two arrays rather
// than a const_cast so the default build never has a writable handle on a
// constant at all.
#define CHESSO_SEARCH_PARAM_SLOT(sym, name, def, lo, hi) &sym,

static int* const param_slots[] = {
    CHESSO_SEARCH_PARAMS(CHESSO_SEARCH_PARAM_SLOT)};

#undef CHESSO_SEARCH_PARAM_SLOT


bool search_param_set(const char* name, int value)
{
  for (size_t i = 0; i < search_param_count(); ++i) {
    if (std::strcmp(param_table[i].name, name) != 0) { continue; }

    if (value < param_table[i].min_value || value > param_table[i].max_value) {
      return false;
    }

    *param_slots[i] = value;

    // Every set, not only the two the reduction table is built from. A table
    // rebuilt from unchanged coefficients is the same table, and the
    // alternative is a per-parameter list that a later parameter is added
    // outside of.
    search_params_rebuild_derived();

    return true;
  }

  return false;
}

#endif
