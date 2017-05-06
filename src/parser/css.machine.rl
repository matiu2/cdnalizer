%%{
    machine css;

    action rec_start {
      url_start = p;
    }

    action rec_end {
      path_found(url_start, p);
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
