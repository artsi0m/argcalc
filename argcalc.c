/*% cc -Wall -Wextra -g % -o # 
 * Copyright (c) 2022 Artsiom Karakin <rakka.artem@yandex.ru>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if defined(__OpenBSD__)
#include <sys/queue.h>
#else
#include "queue.h"
#endif

#include <err.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


enum token_type { TNUM, TOPR, TLBR, TRBR };
/* Enum's from precedence will be appearing only on operator stack */
enum precedence { SUB = 1, ADD = 2, DIV = 3, MUL = 4, LBR = -1};
enum { MIN_ARGS = 3};

struct token_list {
	SIMPLEQ_ENTRY(token_list) next;
	int token_type;
	int payload;
};

SIMPLEQ_HEAD(, token_list) token_list_head;

struct operator_stack {
	SLIST_ENTRY(operator_stack) next;
	int operator; /* Same value as precedence */
};

SLIST_HEAD(, operator_stack) operator_stack_head;

struct rpn_queue {
	SIMPLEQ_ENTRY(rpn_queue) next;
	int token_type;
	int payload;
};

SIMPLEQ_HEAD(, rpn_queue) rpn_queue_head;

struct eval_stack {
	SLIST_ENTRY(eval_stack) next;
	int num;
};

SLIST_HEAD(, eval_stack) eval_stack_head;

void
add_token_to_list(int t_type, int load)
{
	struct token_list *node;

	if ((node = malloc(sizeof(*node))) == NULL)
		errx(1, "Couldn't allocate token");

	node->token_type = t_type;
	node->payload = load;

	SIMPLEQ_INSERT_TAIL(&token_list_head, node, next);
}

void
add_token_to_queue(int t_type, int load)
{
	struct rpn_queue *node;

	if ((node = malloc(sizeof(*node))) == NULL)
		errx(1, "Couldn't allocate queue node");

	node->token_type = t_type;
	node->payload = load;

	SIMPLEQ_INSERT_TAIL(&rpn_queue_head, node, next);
}

void
push_to_operator_stack(int operator)
{
	struct operator_stack *p;

	if ((p = malloc(sizeof(*p))) == NULL)
		errx(1, "Couldn't allocate operator stack node");

	p->operator = operator;
	SLIST_INSERT_HEAD(&operator_stack_head, p, next);
}

int
peek_from_operator_stack(void)
{
	int operator = LBR;
	struct operator_stack *node;

	if (!SLIST_EMPTY(&operator_stack_head)) {
		node = SLIST_FIRST(&operator_stack_head);
		operator = node->operator;
	}

	return operator;
}

int
pop_from_operator_stack(void)
{
	int operator;
	struct operator_stack *node;

	node = SLIST_FIRST(&operator_stack_head);
	operator = node->operator;
	SLIST_REMOVE_HEAD(&operator_stack_head, next);
	free(node);

	return operator;
}

void
push_to_eval_stack(int num)
{
	struct eval_stack *node;

	if ((node = malloc(sizeof(*node))) == NULL)
		errx(1, "Couldnt allocate evaluation stack node");

	node->num = num;
	SLIST_INSERT_HEAD(&eval_stack_head, node, next);
}

int
pop_from_eval_stack(void)
{
	int num;
	struct eval_stack *node;

	if (SLIST_EMPTY(&eval_stack_head))
		errx(1, "Inconsistent number of operators");

	node = SLIST_FIRST(&eval_stack_head);
	num = node->num;
	SLIST_REMOVE_HEAD(&eval_stack_head, next);
	free(node);

	return num;
}

/*
 * ARGument CALCulator
 * Evaluate infix expression supplied as command line 
 * arguments
 */
int
main(int argc, char **argv)
{

	int is_digit;

	SIMPLEQ_INIT(&token_list_head);
	SLIST_INIT(&operator_stack_head);
	SIMPLEQ_INIT(&rpn_queue_head);
	SLIST_INIT(&eval_stack_head);

	struct token_list *token_node;
	struct rpn_queue *rpn_node;

	int operator;
	int operand_first;
	int operand_second;

	/* Turn charaters from command line arguments into tokens */
	for (int i = 1; i < argc && argc > MIN_ARGS; i++) {
		for (int j = 0; argv[i][j] != '\0'; j++) {
			switch (argv[i][j]) {
			case 'x':
				add_token_to_list(TOPR, MUL);
				is_digit = 0;
				break;
			case '/':
				add_token_to_list(TOPR, DIV);
				is_digit = 0;
				break;
			case '+':
				add_token_to_list(TOPR, ADD);
				is_digit = 0;
				break;
			case '-':
				add_token_to_list(TOPR, SUB);
				is_digit = 0;
				break;
			case '(':
				add_token_to_list(TLBR, LBR);
				is_digit = 0;
				break;
			case ')':
				add_token_to_list(TRBR, 0);
				is_digit = 0;
				break;
			case '{':
				add_token_to_list(TLBR, LBR);
				is_digit = 0;
				break;
			case '}':
				add_token_to_list(TRBR, 0);
				is_digit = 0;
				break;
			default:
			/*
			 * Set is_digit != 0 if all charaters in argv[i] 
			 * are digits.
			 */
				if (j == 0 || is_digit != 0)
					is_digit = isdigit(argv[i][j]);
				else
					is_digit = 0;
				break;
			}
		}
		if (is_digit)
			add_token_to_list(TNUM, atoi(argv[i]));
	}

	/* Translate infix expression into reverse polish notation */
	while (!SIMPLEQ_EMPTY(&token_list_head)) {
		token_node = SIMPLEQ_FIRST(&token_list_head);
		if (token_node->token_type == TNUM)
			add_token_to_queue(TNUM, token_node->payload);
		else if (token_node->token_type == TOPR) {
			while (peek_from_operator_stack() > token_node->payload)
				add_token_to_queue(TOPR,
				    pop_from_operator_stack());
			push_to_operator_stack(token_node->payload);
		} else if (token_node->token_type == TLBR) {
			push_to_operator_stack(LBR);
		} else if (token_node->token_type == TRBR) {
			while (peek_from_operator_stack() != LBR)
				add_token_to_queue(TOPR,
				    pop_from_operator_stack());
		/* Pop the left bracket from the stack and discard it */
			pop_from_operator_stack(); 
		}
		SIMPLEQ_REMOVE_HEAD(&token_list_head, next);
		free(token_node);
	}
	while (!SLIST_EMPTY(&operator_stack_head)) {
		add_token_to_queue(TOPR, pop_from_operator_stack());
	}


	/* Evaluate RPN expression using stack */
	while (!SIMPLEQ_EMPTY(&rpn_queue_head)) {
		rpn_node = SIMPLEQ_FIRST(&rpn_queue_head);
		if (rpn_node->token_type == TNUM)
			push_to_eval_stack(rpn_node->payload);
		else if (rpn_node->token_type == TOPR) {
			operator = rpn_node->payload;
			operand_second = pop_from_eval_stack();
			operand_first = pop_from_eval_stack();
			switch (operator) {
			case SUB:
				push_to_eval_stack(operand_first -
				    operand_second);
				break;
			case ADD:
				push_to_eval_stack(operand_first +
				    operand_second);
				break;
			case DIV:
				push_to_eval_stack(operand_first /
				    operand_second);
				break;
			case MUL:
				push_to_eval_stack(operand_first *
				    operand_second);
				break;
			default:
				break;
			}
		}
		SIMPLEQ_REMOVE_HEAD(&rpn_queue_head, next);
		free(rpn_node);
	}

	if (!SLIST_EMPTY(&eval_stack_head))
		printf("%d \n", pop_from_eval_stack());

	return 0;
}
