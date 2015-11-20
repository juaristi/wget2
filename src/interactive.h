#ifndef _INTERACTIVE_H_
# define _INTERACTIVE_H_

struct wget_interactive_opts {
	int include_external_links;
};

struct wget_interactive_opts *wget_interactive_init();
void wget_interactive_start(struct wget_interactive_opts *opts);
void wget_interactive_deinit(struct wget_interactive_opts *opts);

#endif /* ! _INTERACTIVE_H_ */
