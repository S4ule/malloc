
#include <stdarg.h>
	// va_list
	// va_start()
	// va_arg()
	// va_end()

#include <unistd.h>
	// write()

static size_t
_printf_strlen(const char *str)
{
	size_t size;

	size = 0;
	while (str[size])
	{
		size++;
	}
	return size;
}

static void
_printf_putchar(char c)
{
	write(1, &c, 1);
}

static void
_printf_putstr(const char *str)
{
	write(1, str, _printf_strlen(str));
}

static void
_printf_put_hex_upper(unsigned long n)
{
	char c;

	if (n > 15)
	{
		_printf_put_hex_upper(n / 16);
		n %= 16;
	}
	c = n + '0';
	if (c > '9')
	{
		c = c - '0' - 10 + 'A';
	}
	_printf_putchar(c);
}

static void
_printf_put_hex_lower(unsigned long n)
{
	char c;

	if (n > 15)
	{
		_printf_put_hex_lower(n / 16);
		n %= 16;
	}
	c = n + '0';
	if (c > '9')
	{
		c = c - '0' - 10 + 'a';
	}
	_printf_putchar(c);
}

static void
_printf_put_addr(size_t addr)
{
	char c;

	if (addr > 15)
	{
		_printf_put_addr(addr / 16);
		addr %= 16;
	}
	c = addr + '0';
	if (c > '9')
	{
		c = c - '0' - 10 + 'a';
	}
	_printf_putchar(c);
}

static void
_printf_put_uint(unsigned int n)
{
	char c;

	if (n > 9)
	{
		_printf_put_uint(n / 10);
		n %= 10;
	}
	c = n + '0';
	_printf_putchar(c);
}

static void
_printf_put_int(int n)
{
	unsigned int	num;
	char			c;

	if (n < 0)
	{
		_printf_putchar('-');
		num = n * -1;
	}
	else
		num = n;
	if (num > 9)
	{
		_printf_put_uint(num / 10);
		num %= 10;
	}
	c = num + '0';
	_printf_putchar(c);
}

static int
_printf_pars_args(const char c_type, va_list *args)
{
	switch(c_type)
	{
		case 'd':
		{
			int i = va_arg(*args, int);

			_printf_put_int(i);
			break ;
		}
		case 'u':
		{
			unsigned int i = va_arg(*args, unsigned int);

			_printf_put_uint(i);
			break ;
		}
		case 's':
		{
			const char *s = va_arg(*args, const char *);
			
			_printf_putstr(s);
			break ;
		}
		case 'c':
		{
			int c = va_arg(*args, int);

			_printf_putchar(c);
			break ;
		}
		case 'x':
		{
			unsigned long a = va_arg(*args, unsigned long);

			_printf_put_hex_lower(a);
			break ;
		}
		case 'X':
		{
			unsigned long a = va_arg(*args, unsigned long);

			_printf_put_hex_upper(a);
			break ;
		}
		case 'p':
		{
			size_t p = va_arg(*args, size_t);

			_printf_putstr("0x");
			_printf_put_addr(p);
			break ;
		}
		case '%':
		{
			_printf_putchar('%');
			return 1;
		}
		default:
		{
			_printf_putchar(c_type);
			break ;
		}
	}
	return 0;
}

void
ft_printf(const char* format, ...)
{
	va_list args;
 
 	va_start(args, format);
 	while (*format != '\0')
 	{
 		if (*format == '%')
 		{
 			++format;
 			_printf_pars_args(*format, &args);
 		}
 		else
 		{
 			_printf_putchar(*format);
 		}
 		++format;
 	}
	va_end(args);
	return ;
}