#pragma once

#include <iostream>
#include <string>

#define BLACK_C 	"\u001b[30m"
#define RED_C		"\u001b[31m"
#define GREEN_C		"\u001b[32m"
#define YELLOW_C	"\u001b[33m"
#define BLUE_C		"\u001b[34m"
#define MAGENTA_C	"\u001b[35m"
#define CYAN_C		"\u001b[36m"
#define WHITE_C		"\u001b[37m"

#define BLACK_B 	"\u001b[30;1m"
#define RED_B		"\u001b[31;1m"
#define GREEN_B		"\u001b[32;1m"
#define YELLOW_B	"\u001b[33;1m"
#define BLUE_B		"\u001b[34;1m"
#define MAGENTA_B	"\u001b[35;1m"
#define CYAN_B		"\u001b[36;1m"
#define WHITE_B		"\u001b[37;1m"

#define COLOR_END	"\u001b[0m"


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
		std::string color = COLOR_END,
		bool display = false);