%%{
### Parses javascript strings that are inside a "double quote"

  machine js;

  include js "js.common.rl";

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

}%%
