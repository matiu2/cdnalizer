/** Apache filter to cdnalize data.
 *
 * for example it could re-write '<img src="/images/a.gif" />' to '<img src="http://my.cdn.com/images/a.gif"'
 */

typedef struct deflate_filter_config_t
{
    int windowSize;
    int memlevel;
    int compressionlevel;
    apr_size_t bufferSize;
    char *note_ratio_name;
    char *note_input_name;
    char *note_output_name;
} deflate_filter_config;
