#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "merc.h"

#include "_update.h"

static void *
update_print_cb(void *p, va_list ap)
{
	uhandler_t *hdlr = (uhandler_t *) p;
	BUFFER *buf = va_arg(ap, BUFFER *);

	buf_printf(buf, "[%9s] [%9s] [%10s] %5d %5d %c%s\n",
		   hdlr->name,
		   flag_string(module_names, hdlr->mod),
		   hdlr->iter != NULL ?
			flag_string(iterator_names, (flag_t) hdlr->iter):
			"none",
		   hdlr->ticks,
		   hdlr->cnt,
		   hdlr->fun != NULL ? ' ' : '*',
		   hdlr->fun_name);
	return NULL;
}

static void *
update_set_cb(void *p, va_list ap)
{
	uhandler_t *hdlr = (uhandler_t *) p;
	const char *s = va_arg(ap, const char *);
	int value = va_arg(ap, int);

	if (str_cmp(s, hdlr->name))
		return NULL;

	hdlr->ticks = value;
	return hdlr;
}

void do_settick(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	int val;

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		BUFFER *buf;

		buf = buf_new(-1);
		buf_add(buf, "    Name       Module     Iterator    Max   Cur     Function\n");
		buf_add(buf, "----------- ----------- ------------ ----- ----- ---------------\n");
		hash_foreach(&uhandlers, update_print_cb, buf);
		page_to_char(buf_string(buf), ch);
		buf_free(buf);
		return;
	}

	one_argument(argument, arg2, sizeof(arg2));
	val = atoi(arg2);
	if (!val) {
		act_char("Non-zero, please.", ch);
		return;
	}

	if (!hash_foreach(&uhandlers, update_set_cb, arg, val)) {
		dofun("help", ch, "'WIZ SETTICK'");
	}
}

void do_tick(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	
	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')  {
		dofun("help", ch, "'WIZ TICK'");
		return;
	}

	if (update_one(arg))
		return;
	do_tick(ch, str_empty);
}

