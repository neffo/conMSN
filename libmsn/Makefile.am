noinst_LIBRARIES = libmsn.a

EXTRA_DIST = libmsn.h chat_lists.h errors.h md5.h msn_commands.h parse_utils.h

libmsn_a_SOURCES = libmsn.c msn_commands.c parse_utils.c md5.c errors.c chat_lists.c

CFLAGS += $(EB_CFLAGS) -I../src -I../libproxy -I$(top_srcdir) $(GLIB_CFLAGS) $(GTK_CFLAGS)

