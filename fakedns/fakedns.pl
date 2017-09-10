#!/usr/bin/perl
# http://search.cpan.org/dist/Net-DNS/lib/Net/DNS/Nameserver.pm

use strict;
use warnings;
use Net::DNS::Nameserver;

my %records = (
	"A" => {
		"thefilaks.net" => "10.0.0.1",
		"filakovi.cz" => "10.0.0.1",
	},
	"PTR" => {
		"1.0.0.10.in-addr.arpa" => "thefilaks.net",
	},
);

sub reply_handler {
	my ($qname, $qclass, $qtype, $peerhost, $packet, $conn) = @_;
	my ($rcode, @ans, @auth, @add);

	print "Received packet from $peerhost to ". $conn->{sockhost}. "\n";
	my ($question) = $packet->question;
	my $name =  $question->name;
	print "Query Name: $name\n";

	if ($qtype eq "A") {
		my ($ttl, $rdata) = (3600, $records{$qtype}{$name});
		my $rr = new Net::DNS::RR("$qname $ttl $qclass $qtype $rdata");
		push @ans, $rr;
		$rcode = "NOERROR";
	}
	elsif ($qtype eq "PTR") {
		my ($ttl, $rdata) = (3600, $records{$qtype}{$name});
		my $rr = new Net::DNS::RR("$qname $ttl $qclass $qtype $rdata");
		push @ans, $rr;
		$rcode = "NOERROR";
	}

	# mark the answer as authoritive (by setting the 'aa' flag
	return ($rcode, \@ans, \@auth, \@add, { aa => 1 });
}

my $ns = new Net::DNS::Nameserver(
	LocalAddr    => "10.0.0.1",
	LocalPort    => 53,
	ReplyHandler => \&reply_handler,
	Verbose      => 1
	) || die "couldn't create nameserver object\n";

$ns->main_loop;
