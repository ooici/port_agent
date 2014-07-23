if HAVE_GMOCK
  SUBDIRS = test
endif

noinst_LIBRARIES= libport_agent_packet.a

libport_agent_packet_a_SOURCES = packet.cxx packet.h \
                                 buffered_single_char.cxx buffered_single_char.h \
	                         raw_header.cxx raw_header.h \
	                         raw_packet.cxx raw_packet.h \
	                         raw_packet_data_buffer.cxx raw_packet_data_buffer.h

libport_agent_packet_a_CXXFLAGS = -I$(top_builddir)/src
libport_agent_packet_a_LIBADD = $(top_builddir)/src/common/libcommon.a

include $(top_builddir)/src/Makefile.am.inc
