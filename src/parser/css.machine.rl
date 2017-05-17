%%{
    machine css;

    action rec_start {
      path_start = p;
    }

    action rec_end {
      std::string newPath = path_found(path_start, p);
      if (newPath != "")
        return newPath;
      else
        path_start = nullptr;
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
