// -*- mode: c++; c-basic-offset: 4 -*-
#ifndef CLICK_FROMIPSUMDUMP_HH
#define CLICK_FROMIPSUMDUMP_HH
#include <click/element.hh>
#include <click/task.hh>

/*
=c

FromIPSummaryDump(FILENAME [, I<KEYWORDS>])

=s analysis

reads packets from an IP summary dump file

=d

Reads IP packet descriptors from a file produced by ToIPSummaryDump, then
creates packets containing info from the descriptors and pushes them out the
output. Optionally stops the driver when there are no more packets.

The file may be compressed with gzip(1) or bzip2(1); FromIPSummaryDump will
run zcat(1) or bzcat(1) to uncompress it.

FromIPSummaryDump reads from the file named FILENAME unless FILENAME is a
single dash `C<->', in which case it reads from the standard input. It will
not uncompress the standard input, however.

Keyword arguments are:

=over 8

=item STOP

Boolean. If true, then FromIPSummaryDump will ask the router to stop when it
is done reading. Default is false.

=item ACTIVE

Boolean. If false, then FromIPSummaryDump will not emit packets (until the
`C<active>' handler is written). Default is true.

=item ZERO

Boolean. Determines the contents of packet data not set by the dump. If true,
this data is zero. If false (the default), this data is random garbage.

=item PROTO

Byte (0-255). Sets the IP protocol used for output packets when the dump
doesn't specify a protocol. Default is 6 (TCP).

=item MULTIPACKET

Boolean. If true, then FromIPSummaryDump will emit multiple packets for each
line---specifically, it will emit as many packets as the packet count field
specifies. Default is false.

=item SAMPLE

Unsigned real number between 0 and 1. FromIPSummaryDump will output each
packet with probability SAMPLE. Default is 1. FromIPSummaryDump uses
fixed-point arithmetic, so the actual sampling probability may differ
substantially from the requested sampling probability. Use the
C<sampling_prob> handler to find out the actual probability. If MULTIPACKET is
true, then the sampling probability applies separately to the multiple packets
generated per record.

=item DEFAULT_CONTENTS

String, containing a space-separated list of content names (see
ToIPSummaryDump for the possibilities). Defines the default contents of the
dump.

=back

Only available in user-level processes.

=n

Packets generated by FromIPSummaryDump always have IP version 4 and IP header
length 5. The rest of the packet data is zero or garbage, unless set by the
dump. Generated packets will usually have incorrect lengths, but the extra
header length annotations are set correctly.

=h sampling_prob read-only

Returns the sampling probability (see the SAMPLE keyword argument).

=h active read/write

Value is a Boolean.

=h encap read-only

Returns `IP'. Useful for ToDump's USE_ENCAP_FROM option.

=h filesize read-only

Returns the length of the FromIPSummaryDump file, in bytes, or "-" if that
length cannot be determined.

=h filepos read-only

Returns FromIPSummaryDump's position in the file, in bytes.

=h stop write-only

When written, sets `active' to false and stops the driver.

=a

ToIPSummaryDump */

class FromIPSummaryDump : public Element { public:

    FromIPSummaryDump();
    ~FromIPSummaryDump();

    const char *class_name() const	{ return "FromIPSummaryDump"; }
    const char *processing() const	{ return AGNOSTIC; }
    FromIPSummaryDump *clone() const	{ return new FromIPSummaryDump; }

    int configure(const Vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
    void uninitialize();
    void add_handlers();

    void run_scheduled();
    Packet *pull(int);

    enum Content {	// must agree with ToIPSummaryDump
	W_NONE, W_TIMESTAMP, W_TIMESTAMP_SEC, W_TIMESTAMP_USEC,
	W_SRC, W_DST, W_LENGTH, W_PROTO, W_IPID,
	W_SPORT, W_DPORT, W_TCP_SEQ, W_TCP_ACK, W_TCP_FLAGS,
	W_PAYLOAD_LENGTH, W_COUNT, W_FRAG, W_FRAGOFF,
	W_PAYLOAD,
	W_LAST
    };
    static int parse_content(const String &);
    static const char *unparse_content(int);

    static const char * const tcp_flags_word = "FSRPAUXY";

  private:

    static const int BUFFER_SIZE = 32768;
    static const int SAMPLING_SHIFT = 28;
    
    int _fd;
    char *_buffer;
    int _pos;
    int _len;
    int _buffer_len;
    int _save_char;

    Vector<int> _contents;
    uint16_t _default_proto;
    uint32_t _sampling_prob;

    bool _stop : 1;
    bool _format_complaint : 1;
    bool _zero;
    bool _active;
    bool _multipacket;
    Packet *_work_packet;
    uint32_t _multipacket_extra_length;

    Task _task;
    Vector<String> _words;	// for speed

    struct timeval _time_offset;
    String _filename;
    FILE *_pipe;
    off_t _file_offset;

    int error_helper(ErrorHandler *, const char *);
    int read_buffer(ErrorHandler *);
    int read_line(String &, ErrorHandler *);

    void bang_data(const String &, ErrorHandler *);
    Packet *read_packet(ErrorHandler *);
    Packet *handle_multipacket(Packet *);

    static String read_handler(Element *, void *);
    static int write_handler(const String &, Element *, void *, ErrorHandler *);
    
};

#endif
