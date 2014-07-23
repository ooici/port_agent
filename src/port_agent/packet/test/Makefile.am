AM_CXXFLAGS = -I$(top_builddir)/src -I.. -Wno-write-strings 
DEPLIBS = $(top_builddir)/src/port_agent/packet/libport_agent_packet.a \
          $(top_builddir)/src/common/libcommon.a \
          $(GTEST_MAIN)

####
#    Test Definitions
####
noinst_PROGRAMS = basic_packet_test \
                  buffered_single_char_test \
		  raw_packet_test \
	          raw_packet_data_buffer_test


basic_packet_test_SOURCES = basic_packet_test.cxx 
basic_packet_test_LDADD = $(DEPLIBS) -lgtest

buffered_single_char_test_SOURCES = buffered_single_char_test.cxx 
buffered_single_char_test_LDADD = $(DEPLIBS) -lgtest

raw_packet_test_SOURCES = raw_packet_test.cxx
raw_packet_test_LDADD = $(DEPLIBS) -lgtest

raw_packet_data_buffer_test_SOURCES = raw_packet_data_buffer_test.cxx
raw_packet_data_buffer_test_LDADD = $(DEPLIBS) -lgtest

TESTS = $(noinst_PROGRAMS)

include $(top_builddir)/src/Makefile.am.inc
