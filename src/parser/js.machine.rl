%%{
  machine js;

  include js "js.common.rl";

  # This action exists to stop 'rec_string_start' being called at the 's' in
  # </script>
  action noop {
  }

  # Normal JS (inside a <script></script> tag)
  js = (^(sq | dq | sl)* string)*;

  # JS inside of a script tag
  end_script_tag = "</" "script"i '>';
  in_script_js = js end_script_tag;

  include "js.in.dq.machine.rl";
  include "js.in.sq.machine.rl";

  ## TODO handle unquoted attributes ? (they  would be really ugly with '/' path separators is them
  js_read_attrib = in_dq_js | in_sq_js;


}%%
