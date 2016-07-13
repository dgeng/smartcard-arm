#!/usr/bin/perl

use smartcard;

my $mask;
my $fd;

$buffer="a"x255;
$fd = smartcard::serial_init("/dev/smartpoll");
if($fd < 0) { die "Serial Init failed\n"; }

$result = smartcard::send_num_mask($fd, $buffer);
if($result < 0) { die "send_num_mask failed\n"; }
$mask = substr($buffer,0,$result);
print "Mask: $mask\n";

$result = smartcard::power_up_iso($fd, $buffer);
if($result < 0) { die "power_up_iso failed\n"; }
$atr = substr($buffer,0,$result);

@atr1 = split(//,$atr);
print "ATR: ";
foreach(@atr1) { printf("%x ",unpack("C",$_)); }
print "\n";

$result = smartcard::select_file($fd, 0x1f01);
if($result < 0) { die "select_file failed\n"; }
print "Selected file 0x1f01\n";

$result = smartcard::read_binary($fd, 0, 50, $buffer);
if($result < 0) { die "read_binary failed\n"; }
$data = substr($buffer,0,$result);

@data1 = split(//,$data);
print "Data (b4 write): ";
foreach(@data1) { printf("%x ",unpack("C",$_)); }
print "\n";

#$result = smartcard::erase_binary($fd);
#if($result < 0) { die "erase_binary failed\n"; }
#print "Erased file 0x1f01\n";

$data = 5x10;
$result = smartcard::update_binary($fd, 0x14, 10, $data);
if($result < 0) { die "update_binary failed\n"; }
print "Wrote to file 0x1f01\n";

$result = smartcard::read_binary($fd, 0, 50, $buffer);
if($result < 0) { die "read_binary failed\n"; }
$data = substr($buffer,0,$result);

@data1 = split(//,$data);
print "Data (after write): ";
foreach(@data1) { printf("%x ",unpack("C",$_)); }
print "\n";

$result = smartcard::select_file($fd, 0x1f02);
if($result < 0) { die "select_file failed\n"; }
print "Selected file 0x1f02\n";

$pin = pack("C8",0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef);
$result = smartcard::verify_PIN($fd, $pin);
if($result < 0) { die "verify_PIN failed\n"; }
print "PIN verification was successfull\n";

smartcard::serial_deinit($fd);
