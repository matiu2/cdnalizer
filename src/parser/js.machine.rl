%%{
  machine js;

  action rec_string_start {
      string_start = p;
  }

  action rec_string_end {
      onStringFound(string_start, p);
  }

  # This action exists to stop 'rec_string_start' being called at the 's' in
  # </script>
  action noop {
  }

  # Normal JS (inside a <script></script> tag)
  sq = "'";
  dq = '"';
  bs = '\\';
  sl = '/';
  sq_char = ^(sq | bs) | (bs any);
  sq_string = sq sq_char >rec_string_start sq_char* sq >rec_string_end;
  dq_char = ^(dq | bs) | (bs any);
  dq_string = dq dq_char >rec_string_start dq_char* dq >rec_string_end;
  sl_char = ^(sl | bs) | (bs any);
  sl_string = sl sl_char >rec_string_start sl_char* sl >rec_string_end;
  string = sq_string | dq_string | sl_string;
  js = (^(sq | dq | sl)* string)*;

  # JS inside of a script tag
  end_script_tag = "</" "script"i '>';
  in_script_js = js end_script_tag;

  # JS inside of a "double quoted" attribute, eg. <button onclick="location = \"/some/path\"" />
  #                                                   starts here ^
  # Commonly used sub machine - anything except backslash or double quote
  not_bsdq = (any - (bs | dq));
  # After the attribute start, read in junk that is not a js string
  # Before we find the js \"string\" or 'string' or /string/ we can accept
  # any char except '\', '"', '/', "'"
  dq_b4_js_string = (
      (any - (bs | dq | sq | sl)) |
      (bs ^dq)
  )*;
  # The escaped js \"string\"
  dq_escaped_string = 
       # It could be empty
       (bs dq bs dq) |
       # Or a full string
       (
           # The beginning \"
           bs dq 
           # The first character can be anything except dq or backslash
           (
               (not_bsdq) | 
               # Or the first character can be a backslash as long as it's not followed by a dq
               bs (any - dq)
               # Or the first character can be a dq ending the whole state machine.
               # We don't record that here because it's gotten to later via the '|'
               # (or) operator
           ) >rec_string_start 
           # Subsequent characters (in the \"string\" can be any thing except '\' or '"'
           (  
               not_bsdq |
               # Then it can be followed by a bs but not bs dq
               bs ^dq
           )* # Any number of this group of chars
           # Finally it should be delimeted by '\"'
           bs dq @rec_string_end
       );
  # Before we find a js string we can accept all sorts of junk
  in_dq_js = 
     # The first character we see is the double quote that starts the attribute
     dq
     # Junk that comes before the js string
     dq_b4_js_string
     # If we find '\' followed by anything other than bs, we need to go back to the beginning
     (
         (bs ^dq) |
         # or we may find a \"string\"
         dq_escaped_string |
         sq_string |
         sl_string
     )*
     # The final state of the parser
     dq;

  ## JS inside of a 'single quoted' html attribute, eg <button onClick='do_something(\'some path\');' />
  # Commonly used sub machine - anything except backslash or double quote
  not_bssq = (any - (bs | sq));
  # After the attribute start, read in junk that is not a js string
  # Before we find the js \"string\" or 'string' or /string/ we can accept
  # any char except '\', '"', '/', "'"
  sq_b4_js_string = (
      (any - (bs | sq | sq | sl)) |
      (bs ^sq)
  )*;
  # The escaped js \"string\"
  sq_escaped_string = 
       # It could be empty
       (bs sq bs sq) |
       # Or a full string
       (
           # The beginning \"
           bs sq 
           # The first character can be anything except sq or backslash
           (
               (not_bssq) | 
               # Or the first character can be a backslash as long as it's not followed by a sq
               bs (any - sq)
               # Or the first character can be a sq ending the whole state machine.
               # We don't record that here because it's gotten to later via the '|'
               # (or) operator
           ) >rec_string_start 
           # Subsequent characters (in the \"string\" can be any thing except '\' or '"'
           (  
               not_bssq |
               # Then it can be followed by a bs but not bs sq
               bs ^sq
           )* # Any number of this group of chars
           # Finally it should be delimeted by '\"'
           bs sq @rec_string_end
       );
  # This searches for java script strings delimited by "\'", ", or /
  in_sq_js = 
     # The first character we see is the double quote that starts the attribute
     sq
     # Junk that comes before the js string
     sq_b4_js_string
     # If we find '\' followed by anything other than bs, we need to go back to the beginning
     (
         (bs ^sq) |
         # or we may find a \"string\"
         sq_escaped_string |
         dq_string |
         sl_string
     )*
     # The final state of the parser
     sq;

  ## TODO handle unquoted attributes ? (they  would be really ugly with '/' path separators is them

}%%
