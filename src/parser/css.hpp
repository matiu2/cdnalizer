
#line 1 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp.rl"
#pragma once

namespace cdnalizer {
namespace parser {


#line 9 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp.rl"


// State machine exports

#line 15 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp"

#line 13 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp.rl"

// State machine data

#line 21 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp"
static const char _css_url_actions[] = {
	0, 1, 0, 1, 1
};

static const char _css_url_key_offsets[] = {
	0, 0, 1, 2, 3, 7, 13, 19, 
	23, 24, 25, 26, 27
};

static const char _css_url_trans_keys[] = {
	117, 114, 108, 32, 40, 9, 13, 32, 
	34, 39, 41, 9, 13, 32, 34, 39, 
	41, 9, 13, 32, 41, 9, 13, 34, 
	34, 39, 39, 117, 0
};

static const char _css_url_single_lengths[] = {
	0, 1, 1, 1, 2, 4, 4, 2, 
	1, 1, 1, 1, 1
};

static const char _css_url_range_lengths[] = {
	0, 0, 0, 0, 1, 1, 1, 1, 
	0, 0, 0, 0, 0
};

static const char _css_url_index_offsets[] = {
	0, 0, 2, 4, 6, 10, 16, 22, 
	26, 28, 30, 32, 34
};

static const char _css_url_trans_targs[] = {
	2, 1, 3, 0, 4, 0, 4, 5, 
	4, 0, 5, 8, 10, 0, 5, 6, 
	7, 0, 0, 12, 7, 6, 7, 12, 
	7, 0, 0, 9, 7, 9, 0, 11, 
	7, 11, 2, 1, 0
};

static const char _css_url_trans_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 1, 
	3, 0, 0, 3, 3, 0, 0, 0, 
	0, 0, 0, 1, 3, 0, 0, 1, 
	3, 0, 0, 0, 0
};

static const int css_url_start = 12;
static const int css_url_first_final = 12;
static const int css_url_error = 0;

static const int css_url_en_css_url = 12;


#line 16 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp.rl"

/// Parses some CSS, looking for url() functions
/// @param A reference to the pointer to the start of the data. This will be incremented as our search continues
/// @param pe A const reference to the pointer to the end of the input
/// @param path_found This function will be called every time a path is found,
///        passing two iterators, the first letter of the path, and one past the end.
/// p is a reference because when dealing with Apache bucket brigades, it can change, and we changed it also
/// pe is a const reference because apache bucket brigade splitting may change it (but we don't change it).
template <typename Iterator>
Iterator parseCSS(Iterator &p, const Iterator& pe,
                  std::function<void(Iterator, Iterator)> path_found) {
  int cs;

  // Data needed for the actions
  auto css_start = p;

  // State machine initialization
  
#line 95 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp"
	{
	cs = css_url_start;
	}

#line 34 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp.rl"

  // State machine code
  
#line 104 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp"
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
	_keys = _css_url_trans_keys + _css_url_key_offsets[cs];
	_trans = _css_url_index_offsets[cs];

	_klen = _css_url_single_lengths[cs];
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

	_klen = _css_url_range_lengths[cs];
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
	cs = _css_url_trans_targs[_trans];

	if ( _css_url_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _css_url_actions + _css_url_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 4 "/home/ubuntu/projects/cdnalizer/src/parser/css.machine.rl"
	{
      css_start = p;
    }
	break;
	case 1:
#line 8 "/home/ubuntu/projects/cdnalizer/src/parser/css.machine.rl"
	{
      path_found(css_start, p);
    }
	break;
#line 189 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp"
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

#line 37 "/home/ubuntu/projects/cdnalizer/src/parser/css.hpp.rl"

  return p;
}


} /* parser */ 
} /* cdnalizer  */ 
