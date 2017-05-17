%%{
  ## Finds </script>
  machine end_script;
  script_word = /script/i;
  script = "</" script_word ">";
  end_script := ((any*) -- script) script;
}%%
