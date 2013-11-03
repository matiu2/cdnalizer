%%{
    machine tag_name;

    action start_tag_name {
        auto tag_start=p;
    }
    action end_tag_name {
        auto tag_end=p;
    }
    action good_ending {
        found_tag(tag_start, tag_end);
    }
    action new_tag { new_tag(); }
    action tag_ends { tag_ends(); }

    start_tag = '<';
    good_ending = space % good_ending;
    bad_ending = ('<' > new_tag) | ('>' % tag_ends);
    ending = good_ending | bad_ending;
    safe_alnum = alnum - ending;
    tag_name := (safe_alnum+ > start_tag_name % end_tag_name ) . ending;
}%%
