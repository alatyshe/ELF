#pragma once

#include <iostream>
#include <string>

#define BLACK_C 	"\x1b[0;30;40m"
#define RED_C		"\x1b[0;31;40m"
#define GREEN_C		"\x1b[0;32;40m"
#define ORANGE_C	"\x1b[0;33;40m"
#define BLUE_C		"\x1b[0;34;40m"
#define PURPLE_C	"\x1b[0;35;40m"
#define SHIT_C		"\x1b[0;36;40m"
#define WHITE_C		"\x1b[0;37;40m"

#define BLACK_B		"\x1b[1;30;40m"
#define RED_B		"\x1b[1;31;40m"
#define GREEN_B		"\x1b[1;32;40m"
#define ORANGE_B	"\x1b[1;33;40m"
#define BLUE_B		"\x1b[1;34;40m"
#define PURPLE_B	"\x1b[1;35;40m"
#define SHIT_B		"\x1b[1;36;40m"
#define WHITE_B		"\x1b[1;37;40m"

#define COLOR_END	"\x1b[0m"


namespace debug {
	typedef struct	s_options {
		bool		verbose;

		s_options(): 
			verbose(false)
		{};
	}				t_options;	
}

std::string Backtrace(int skip = 1);
void		decrase_indent();
void		display_debug_info(
		std::string clss_name,
		std::string mthd_name = "",
		std::string color = COLOR_END);