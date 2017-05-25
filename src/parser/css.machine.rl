%%{
    machine css;

    action rec_start {
      path_start = p;
      path_start_state = cs;
    }

    action rec_end {
      // We've found the path start and the path end, return the pair
      assert(path_start != decltype(path_start)());
      if (path_start != p)
        return {path_start, p};
    }

    find_url = (any)* -- "url";
    url_start = "url" space* '(' space**;
    url_end = space* ')';
    single_quotes = "'" %rec_start (any - "'")+ "'" >rec_end;
    double_quotes = '"' %rec_start (any - '"')+ '"' >rec_end; 
    no_quotes_term = ["')] | space;
    no_quotes = (any - no_quotes_term) > rec_start (any - no_quotes_term)* %rec_end;
    url_middle = single_quotes | double_quotes | no_quotes;
    single_url = url_start url_middle url_end;
    css := (find_url single_url)*;
}%%
