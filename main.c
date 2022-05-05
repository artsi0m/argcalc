#include <sys/queue.h>

#include <err.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


enum token_type { TNUM = 0, TSUB = 1, TADD = 2, TDIV = 3, TMUL = 4,
	TLBR = 5, TRBR = 6 };

struct token_list {
	SIMPLEQ_ENTRY(token_list) next;
	int token_type;
	int payload;
};


SIMPLEQ_HEAD( ,token_list) token_list_head;


struct operator_stack {
	SLIST_ENTRY(operator_stack) next;
	int operator; /* Same value as token_type */
};

SLIST_HEAD(, operator_stack) operator_stack_head;


struct rpn_queue {
	SIMPLEQ_ENTRY(rpm_queue) next;
	int token_type;
	int payload;
};

SIMPLEQ_HEAD( ,rpn_queue) rpn_queue_head;

void
add_token_to_list(int t_type, int load)
{
	struct token_list *node;

	if ((node = malloc(sizeof(*node))) == NULL)
		err(1, NULL);

	node->token_type = t_type;
	node->payload = load;

	SIMPLEQ_INSERT_TAIL(&token_list_head, node, next);
}


void
add_token_to_queue(int t_type, int load)
{
	struct rpn_queue *node;

	if ((node = malloc(sizeof(*node))) == NULL)
		err(1, NULL);

	node->token_type=t_type;
	node->payload = load;

	SIMPLEQ_INSERT_TAIL(&rpn_queue_head, node, next);

}


void
push_to_operator_stack(int operator)
{
	struct operator_stack *p;

	if ((p = malloc(sizeof(*p))) == NULL)
		err(1, NULL);

	p->operator = operator;
	SLIST_INSERT_HEAD(&operator_stack_head, p, next);
}


int
peek_from_operator_stack(void)
{
	int operator = TEOF;
	struct operator_stack *p;

	if (!SLIST_EMPTY(&operator_stack_head)) {
		p = SLIST_FIRST(&operator_stack_head);
		operator = p->operator;
	}

	return operator;
}

int
pop_from_operator_stack(void)
{

	int operator = TEOF;
	struct operator_stack *p;

	if (!SLIST_EMPTY(&operator_stack_head)) {
		p = SLIST_FIRST(&operator_stack_head);
		operator = p->operator;
		SLIST_REMOVE_HEAD(&operator_stack_head, next);
		free(p);
	}

	return operator;
}

int
main(int argc, char **argv)
{

	int is_digit;

	SIMPLEQ_INIT(&token_list_head);
	SLIST_INIT(&operator_stack_head);
	SIMPLEQ_INIT(&rpn_queue_head);

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
			case '(':
				add_token_to_list(TLBR, 0);
				is_digit = 0;
				break;
			case ')':
				add_token_to_list(TRBR, 0);
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
}
