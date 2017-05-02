%%{
  machine html;

  action rec_attrib_name_start {
    attrib_name_start = p;
  }

  action rec_attrib_name_end {
    attrib_name_end = p;
  }

  action rec_attrib_val_start {
    attrib_val_start = p;
  }

  action rec_attrib_val_end {
    attrib_found(attrib_name_start, attrib_name_end, attrib_val_start, p);
  }

  find_tag = ^("<")*;

  # reusable tag parts
  tag_start = '<' alnum+;
  tag_end = space* '/'? '>';

  # Attribute Name
  attrib_name_chars = alnum | '_' | ':' | '-';
  attrib_name = attrib_name_chars >rec_attrib_name_start attrib_name_chars* %rec_attrib_name_end;

  # Attribute value
  attrib_val_double_quoted = '"' $rec_attrib_val_start (^'"'*) '"' >rec_attrib_val_end;
  attrib_val_single_quoted = "'" $rec_attrib_val_start ^"'"* "'" >rec_attrib_val_end; 
  #attrib_val_no_quote_char = any - (['"/>] | space);
  attrib_val_no_quote_char = alnum;
  attrib_val_no_quotes = attrib_val_no_quote_char >rec_attrib_val_start attrib_val_no_quote_char* %rec_attrib_val_end; 
  attrib_val = attrib_val_double_quoted | attrib_val_single_quoted | attrib_val_no_quotes;

  # Attribute
  attrib = attrib_name space* ('=' space* attrib_val)?;

  # Different types of tags we come across
  empty_xml = "<!>";
  xml_thing = "<!" ^'-' (any - '>')* '>';
  comment= "<!--" (any - '-')* "-->";
  end_tag = "</" alnum+ '>';
  empty_tag = tag_start space* tag_end;
  good_tag = tag_start space+ (attrib space+)* attrib? tag_end;

  tag := xml_thing | comment | end_tag | empty_tag | good_tag;
}%%
