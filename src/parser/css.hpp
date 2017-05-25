
#line 1 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp.rl"
#pragma once

#include <string>
#include <functional>
#include <boost/range/iterator_range.hpp>

namespace cdnalizer {
namespace parser {


#line 13 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp.rl"


// State machine exports

#line 19 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp"

#line 17 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp.rl"

// State machine data

#line 25 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp"
static const char _css_actions[] = {
	0, 1, 0, 1, 1
};

static const char _css_key_offsets[] = {
	0, 0, 1, 3, 5, 9, 15, 21, 
	25, 26, 27, 28, 29
};

static const char _css_trans_keys[] = {
	117, 114, 117, 108, 117, 32, 40, 9, 
	13, 32, 34, 39, 41, 9, 13, 32, 
	34, 39, 41, 9, 13, 32, 41, 9, 
	13, 34, 34, 39, 39, 117, 0
};

static const char _css_single_lengths[] = {
	0, 1, 2, 2, 2, 4, 4, 2, 
	1, 1, 1, 1, 1
};

static const char _css_range_lengths[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 
	0, 0, 0, 0, 0
};

static const char _css_index_offsets[] = {
	0, 0, 2, 5, 8, 12, 18, 24, 
	28, 30, 32, 34, 36
};

static const char _css_indicies[] = {
	1, 0, 2, 1, 0, 3, 1, 0, 
	3, 5, 3, 4, 5, 7, 8, 4, 
	5, 6, 10, 4, 4, 11, 10, 9, 
	12, 13, 12, 4, 4, 14, 10, 15, 
	4, 16, 10, 17, 1, 0, 0
};

static const char _css_trans_targs[] = {
	1, 2, 3, 4, 0, 5, 6, 8, 
	10, 6, 7, 12, 7, 12, 9, 9, 
	11, 11
};

static const char _css_trans_actions[] = {
	0, 0, 0, 0, 0, 0, 1, 0, 
	0, 0, 3, 3, 0, 0, 1, 0, 
	1, 0
};

static const int css_start = 12;
static const int css_first_final = 12;
static const int css_error = 0;

static const int css_en_css = 12;


#line 20 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp.rl"

/// Parses some CSS, looking for url() functions
/// @param A reference to the pointer to the start of the data. This will be incremented as our search continues
/// @param pe A const reference to the pointer to the end of the input
/// p is a reference because when dealing with Apache bucket brigades, it can change, and we changed it also
/// pe is a const reference because apache bucket brigade splitting may change it (but we don't change it).
/// cs is a reference to the current state of parsing
template <typename Iterator>
boost::iterator_range<Iterator> parseCSS(Iterator &p, Iterator pe, int &cs) {

  Iterator path_start;

  int path_start_state = css_start;

  // If current state is undefined, set it to our start state
  if (cs == -1)
    cs = css_start;

  // State machine initialization
  
#line 105 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp"
	{
	cs = css_start;
	}

#line 40 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp.rl"

  // State machine code
  
#line 114 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _css_trans_keys + _css_key_offsets[cs];
	_trans = _css_index_offsets[cs];

	_klen = _css_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _css_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _css_indicies[_trans];
	cs = _css_trans_targs[_trans];

	if ( _css_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _css_actions + _css_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 4 "/home/ubuntu/projects/cdnalizer/src/parser/css.machine.rl"
	{
      path_start = p;
      path_start_state = cs;
    }
	break;
	case 1:
#line 9 "/home/ubuntu/projects/cdnalizer/src/parser/css.machine.rl"
	{
      // We've found the path start and the path end, return the pair
      assert(path_start != decltype(path_start)());
      if (path_start != p)
        return {path_start, p};
    }
	break;
#line 204 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 43 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp.rl"

  // If we get here we've found the path_start but not the path end return just
  // the path_start, and an empty iterator
  if (path_start != decltype(path_start)())
    return {path_start, Iterator{}};
}


} /* parser */ 
} /* cdnalizer  */ 
