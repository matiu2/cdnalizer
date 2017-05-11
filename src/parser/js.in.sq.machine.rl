%%{
### Parses javascript strings that are inside a "single quote"

  machine js;

  include "js.common.rl";

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

}%%
