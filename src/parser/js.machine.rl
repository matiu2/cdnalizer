%%{
  machine js;

  action rec_string_start {
      string_start = p;
  }

  action rec_string_end {
      onStringFound(string_start, p);
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
  end_script_tag = "</" /script/i '>';
  in_script_js = js end_script_tag;

  # JS inside of a "double quoted" attribute
  bsdq_dq '\"';
  bsdq_char = ^(bsdq | bs) | ((bs any) - bsdq);
  bsdq_string = bsdq dq_char >rec_string_start dq_char* bsdq >rec_string_end;
  bsdq_whole_string = sq_string | bsdq_string | sl_string;
  bsdq_js = (^(sq | bsdq | sl)* string)* dq;
 

}%%
