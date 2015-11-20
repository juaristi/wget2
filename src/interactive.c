/*
 * Copyright(c) 2012-2014 Tim Ruehsen
 * Copyright(c) 2015 Free Software Foundation, Inc.
 *
 * This file is part of Wget.
 *
 * Wget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Wget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Wget.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Wget2 interactive mode
 *
 * Changelog
 * 20.11.2015  Ander Juaristi  created
 *
 */
#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <libwget.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include "wget.h"
#include "interactive.h"

struct wget_interactive_cmd {
	const char *cmd;
	int (* cmd_handler)(char *const *args);
};

static int __handle_tab_key(int count, int key)
{
	/* TODO this should search in the link list */
	printf("\tPressed tab (%d).\n", key);
	return 0;
}

static int __cmd_exit(char *const *args)
{
	printf("Exiting.\n");
	return 1;
}

/* TODO implement */
static int __cmd_help(char *const *args)
{
	printf("Here goes the help\n");
	return 0;
}

/*
 * TODO implement
 */
static int process_cmd (const char *line, struct wget_interactive_cmd *cmds, int ncmds)
{
	int i, retval = 0, cmd_found = 0;
	/*
	 * TODO Should further be split into cmd + args
	 */
	const char *cmd = line;
	char **args = NULL;

	for (i = 0; i < ncmds && cmd_found == 0; i++) {
		if (!strcmp(cmds[i].cmd, cmd)) {
			retval = cmds[i].cmd_handler(args);
			cmd_found = 1;
		}
	}

	if (cmd_found == 0)
		printf("Unknown command '%s'\n", cmd);

	printf("\tWrote: %s\n", line);
	return retval;
}

/*
 * Returns non-zero on success.
 */
static int init_readline()
{
	int retval = 0;

	// Bind TAB to our custom handler
	retval = rl_bind_key('\t', __handle_tab_key);

	/*
	 * TODO
	 * Scan all the internal links found (+ external links if 'include_external_links' is asserted)
	 * if the user has already passed some URLs from the command line.
	 * If no URLs were given, do nothing, but auto-completion will not work.
	 * The user specifies new URLs to scan with the 'url' command:
	 *
	 * 	url http://ftp.gnu.org/wget/
	 *
	 * All the links found in each URL is appended to the existing URLs, and eligible for auto-completion.
	 */
	return !retval;
}

struct wget_interactive_opts *wget_interactive_init()
{
	struct wget_interactive_opts *opts = NULL;

	if (!init_readline())
		goto end;

	opts = wget_malloc(sizeof(struct wget_interactive_opts));
	opts->include_external_links = 0;

end:
	return opts;
}

/*
 * This is a blocking function.
 * It will enter a continuous loop that won't break until the user types 'exit', or hits Ctrl-D (EOF).
 */
void wget_interactive_start(struct wget_interactive_opts *opts)
{
	char *line = NULL;
	int exit = 0;
	struct wget_interactive_cmd cmds[] = {
		{
			.cmd = "exit",
			.cmd_handler = __cmd_exit
		},
		{
			.cmd = "help",
			.cmd_handler = __cmd_help
		}
	};

	if (!opts)
		return;

	printf("Entering wget2 interactive console.\n");
	printf("Type 'help' for a list of available commands.\n");
	printf("Type 'exit' to exit from wget2.\n\n");
	do {
		line = readline("wget2 > ");
		if (line && *line)
			exit = process_cmd(line, cmds, countof(cmds));
	} while (!exit);
}

void wget_interactive_deinit(struct wget_interactive_opts *opts)
{
	if (!opts)
		return;

	free(opts);
}
