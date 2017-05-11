%%{
### Commonly shared mini fsm's used by all js parsers

  machine js;

  action rec_string_start {
      string_start = p;
  }

  action rec_string_end {
      onStringFound(string_start, p);
  }

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
}%%

