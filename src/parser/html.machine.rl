%%{
  machine tag;

  action rec_tag_name_start {
      tag_name_start = p;
  }

  action rec_tag_name_end {
    tag_found(boost::make_iterator_range(tag_name_start, p));
  }

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
    attrib_found(boost::make_iterator_range(attrib_name_start, attrib_name_end),
                 boost::make_iterator_range(attrib_val_start, p));
  }

  # reusable tag parts
  tag_start = '<' alnum >rec_tag_name_start alnum* %rec_tag_name_end;
  tag_end = space* '/'? '>';

  # Attribute Name
  attrib_name_chars = alnum | '_' | ':' | '-';
  attrib_name = attrib_name_chars >rec_attrib_name_start attrib_name_chars* %rec_attrib_name_end;

  # Attribute value
  dq = '"';
  sq = "'";
  attrib_val_double_quoted = dq ^dq >rec_attrib_val_start (^dq*) dq >rec_attrib_val_end;
  attrib_val_single_quoted = sq ^sq >rec_attrib_val_start (^sq*) sq >rec_attrib_val_end; 
  #attrib_val_no_quote_char = alnum | '-' | '.';  # Non quoted attrib possible chars according to HTML4 spec
  attrib_val_no_quote_char = any - (space | '>');
  attrib_val_no_quotes = (attrib_val_no_quote_char - ['"]) >rec_attrib_val_start ((attrib_val_no_quote_char*) -- "/>") %rec_attrib_val_end;
  attrib_val = attrib_val_double_quoted | attrib_val_single_quoted | attrib_val_no_quotes;

  # Attribute
  attrib = attrib_name space* ('=' space* attrib_val)?;

  # Different types of tags we come across
  empty_xml = "<!>";
  xml_thing = "<!" ^'-' (any - '>')* '>';
  comment= "<!--" (any - '-')* "-->";
  end_tag = "</" ^'>'* '>';
  empty_tag = tag_start space* tag_end;
  good_tag = tag_start space+ (attrib space+)* (attrib)? tag_end;

  tag := xml_thing | comment | end_tag | empty_tag | good_tag;
}%%
