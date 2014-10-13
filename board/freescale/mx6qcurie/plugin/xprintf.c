#include <stdarg.h>
#include <stdint.h>

extern void xputc(char c);

static int xstrlen(const char *s)
{
	int len = 0;
	while(*s++) len++;
	return len;
}

static void xputs(const char *s)
{
	while(*s)
		xputc(*s++);
}

static void xputn(const char *s, int n)
{
	while(n--)
		xputc(*s++);
}

static void sreverse(char *begin, char *end)
{
	char tmp;
	end--;
	while(begin < end)
	{
		tmp = *begin;
		*begin = *end;
		*end = tmp;
		begin++;
		end--;
	};
}

static int fill_to_width_reverse(char *str, char *buf, int width, char fill)
{
	while(buf - str < width) *buf++ = fill;
	sreverse(str, buf);
	return buf - str;
}

static int itos(char *buf, int32_t i, int width, char fill)
{
	char *str = buf;
	char sign = 0;

	if(i < 0) { i = -i; sign = 1; };

	do
	{
		*buf++ = '0' + i % 10;
		i /= 10;
	} while(i);
	if(sign) *buf++ = '-';

	return fill_to_width_reverse(str, buf, width, fill);
}

static int utos(char *buf, uint32_t u, int width, char fill)
{
	char *str = buf;
	do
	{
		*buf++ = '0' + u % 10;
		u /= 10;
	} while(u);

	return fill_to_width_reverse(str, buf, width, fill);
}

static int htos(char *buf, uint32_t h, uint8_t width, char fill, char *code)
{
	char *str = buf;
	do
	{
		*buf++ = code[h&0xF];
		h >>= 4;
	} while(h);

	return fill_to_width_reverse(str, buf, width, fill);
}

int vxprintf(const char *fmt, va_list ap)
{
	enum { ST_NORMAL, ST_ARG, ST_ESC };

	char buf[32];
	int blen;
	char *str;
	int state = ST_NORMAL;
	int width = 0;
	char fill = ' ';

	while(*fmt)
	{
		switch(state)
		{
			case ST_NORMAL:
				switch(*fmt)
				{
					case '%': width = 0; fill = ' '; state = ST_ARG; break;
					default: xputc(*fmt);
				};
				break;
			case ST_ARG:
				switch(*fmt)
				{
					// percent sign
					case '%': xputc('%'); state = ST_NORMAL; break;
							  // signed & unsigned number
					case 'd': blen = itos(buf, va_arg(ap, int32_t), width, fill); xputn(buf, blen); state = ST_NORMAL; break;
					case 'u': blen = utos(buf, va_arg(ap, uint32_t), width, fill); xputn(buf, blen); state = ST_NORMAL; break;
					case 'p':
					case 'x': blen = htos(buf, va_arg(ap, uint32_t), width, fill, "0123456789abcdef"); xputn(buf, blen); state = ST_NORMAL; break;
					case 'P':
					case 'X': blen = htos(buf, va_arg(ap, uint32_t), width, fill, "0123456789ABCDEF"); xputn(buf, blen); state = ST_NORMAL; break;
							  // character
					case 'c': buf[0] = (char) va_arg(ap, int); xputc(buf[0]); state = ST_NORMAL; break;
							  // string
					case 's':
							  str = va_arg(ap, char*);
							  xputs(str);
							  blen = xstrlen(str);
							  while(blen++ < width)
								  xputc(fill);
							  state = ST_NORMAL;
							  break;
							  // width & fill
					case '0': if(width == 0) fill = '0'; // fill zero in margin
					case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8':
					case '9': width = width * 10 + (*fmt - '0'); break;
					// unknown option, ignore it
					default: xputc(*fmt); state = ST_NORMAL;
				};
				break;
		};

		fmt++;
	};

	return 0;
}

int xprintf(const char *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vxprintf(fmt, ap);
	va_end(ap);

	return ret;
}

