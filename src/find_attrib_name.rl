%%{
    machine find_attrib_name;

    action start_attrib_name {
        auto name_start=p;
    }
    action got_attrib_name_end {
        auto name_end=p;
    }
    action good_ending {
        found_attrib_name(name_start, name_end);
    }
    action new_tag { new_tag(); }
    action tag_ends { tag_ends(); }

    start_tag = '<' % new_tag;
    end_tag = start_tag | '>' % tag_ends;
    good_ending = '=' % good_ending;
    bad_ending = end_tag | '"';
    end_name = good_ending | bad_ending;
    safe_alnum = alnum - end_name;
    find_attrib_name := (safe_alnum+ > start_attrib_name % got_attrib_name_end ) . space* . end_name;
}%%
