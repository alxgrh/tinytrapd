#!/usr/bin/perl

$file = $ARGV[0];

local $/;

open (MIB, $file) or die "unabele to open $file";

$mibtxt = <MIB>;

$mibtxt =~ s/\-\-.*\n//g;

if ( $mibtxt !~ /(\S+) DEFINITIONS ::= BEGIN/){
        die "Bad MIB FORMAT";
}
else {
        print "MIB-BEGIN " . $1 ."\n";
	$mibtxt =~ s/.*::= BEGIN//;
}

if ($mibtxt =~ /IMPORTS(.*?);/s){
	$imports = $1;
	$mibtxt =~ s/.*IMPORTS(.*?);//;
	while ($imports =~ /(.*?)FROM\s+(\S+)/sg){
		$line=$1;
		$impmodule=$2;
		while ($line =~ /\W([a-z][\w\-]+)/g){
                                print "IMPORTS $1 $impmodule\n";
                        }
	}
}

#system       OBJECT IDENTIFIER ::= { mib-2 1 }
while($mibtxt =~ /\W([a-z][\w\-]+)\s+MODULE-IDENTITY.*?::=\s+\{\s+(\S+)\s+(\d+)\s+\}/sg){
        print "$1 $2 $3\n";
}

while($mibtxt =~ /\W([a-z][\w\-]+)\s+OBJECT IDENTIFIER\s+::=\s+\{\s+(\S+)\s+(\d+)\s+\}/sg){
	print "$1 $2 $3\n";
}

while($mibtxt =~ /\W([a-z][\w\-]+)\s+OBJECT-TYPE.*?::=\s+\{\s+(\S+)\s+(\d+)\s+\}/sg){
        print "$1 $2 $3\n";
}


#print $mibtxt;
exit 0;
if ( $line !~ /(\S+) DEFINITIONS ::= BEGIN/){
	die "Bad MIB FORMAT";
}
else {
	print "MIB-BEGIN " . $1 ."\n";
}



while ($line=<MIB>){
	if ($line =~ /^\-\-/){
		next;
	}
	if ($line eq "IMPORTS\n") {
		print $line;
		while ($line=<MIB>){
			if ($line =~ /^\-\-/){
				next;
			}
			if($line !~ /^\s/) {
				last;
			}
			while ($line =~ /\W([a-z][\w\-]+)/g){
				print $1;
			}

		}
	}
}
