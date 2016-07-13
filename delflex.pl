#!/usr/bin/perl

use smartcard;

my $mask;
my $fd;

$buffer="a"x520;
$fd = smartcard::serial_init("/dev/smartpoll");
if($fd < 0) { die "Serial Init failed\n"; }

$result = smartcard::send_num_mask($fd, $buffer);
if($result < 0) { die "send_num_mask failed\n"; }
$mask = substr($buffer,0,$result);
print "Mask: $mask\n";

$buffer="a"x520;
$result = smartcard::power_up_iso($fd, $buffer);
if($result < 0) { die "power_up_iso failed\n"; }
$atr = substr($buffer,0,$result);

@atr1 = split(//,$atr);
print "ATR: ";
foreach(@atr1) { printf("%x ",unpack("C",$_)); }
print "\n";

$result = smartcard::select_file($fd, 0x3f00);
if($result < 0) { die "select_file failed\n"; }
print "Selected file 0x3f00\n";

$pin=pack("C8",0x47,0x46,0x55,0x39,0x38,0x49,0x90,0x63);

$result = smartcard::verify_PIN($fd, $pin);
if($result < 0) { die "verify_PIN failed\n"; }
print "Successfully verified the PIN for the master file\n";

$result = smartcard::delete_file($fd, 0x2030);
if($result < 0) { die "delete_file failed\n"; }
print "Deleted file 0x2030\n";

smartcard::serial_deinit($fd);
