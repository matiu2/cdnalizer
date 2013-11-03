%%{
    machine find_attrib_value;

    action start_attrib_value {
        auto value=p;
    }
    action got_attrib_value {
        auto value=p;
    }
    action good_ending {
        found_attrib_value(value, value);
    }
    action new_tag { new_tag(); }
    action tag_ends { tag_ends(); }

    start_tag = '<' ;
    end_tag = start_tag | '>';
    good_ending = '"' % good_ending;
    bad_ending = end_tag;
    end_value = good_ending | bad_ending;
    chars = any - end_value;
    find_attrib_value := '"' . (chars+ > start_attrib_value % got_attrib_value ) . end_value;
}%%

