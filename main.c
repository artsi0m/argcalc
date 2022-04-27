#include <sys/queue.h>

#include <err.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


enum token_type { TNUM, TMUL, TDIV, TADD, TSUB, TEOF };

struct token_list {
	SIMPLEQ_ENTRY(token_list) next;
	int token_type;
	int payload;
};


SIMPLEQ_HEAD( ,token_list) head;

void
add_token_to_list(int t_type, int load)
{
	struct token_list *node;

	if ((node = malloc(sizeof(*node))) == NULL)
		err(1, NULL);

	node->token_type = t_type;
	node->payload = load;

	SIMPLEQ_INSERT_TAIL(&head, node, next);
}

int
main(int argc, char **argv)
{

	int is_digit;

	SIMPLEQ_INIT(&head);

	for (int i = 0; i < argc; i++) {
		for (int j = 0; argv[i][j] != '\0'; j++) {
			switch(argv[i][j]) {
			case '*':
				add_token_to_list(TMUL, 0);
				is_digit = 0;
				break;
			case '/':
				add_token_to_list(TDIV, 0);
				is_digit = 0;
				break;
			case '+':
				add_token_to_list(TADD, 0);
				is_digit = 0;
				break;
			case '-':
				add_token_to_list(TSUB, 0);
				is_digit = 0;
				break;
			default:
			/*
			 * Set is_digit != 0 if all charaters in argv[i] 
			 * are digits.
			 */
				if (j == 0 || is_digit != 0 )
				    is_digit = isdigit(argv[i][j]);
				else 
				    is_digit = 0;
				break;
			}
		}
		if (is_digit)
			add_token_to_list(TNUM, atoi(argv[i]));
	}
	add_token_to_list(TEOF, 0);
}
