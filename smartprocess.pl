#!/usr/bin/perl
use sCode;
use rIML;
use smartcard;

print "<iml><head><title>Welcome Smart guy!</title></head>";

{
    $info="a"x300;
    $buffer="a"x520;

    $fd = smartcard::serial_init("/dev/smartpoll");
    if($fd < 0) { 
            print "<card>Unable to open serial port</card></iml>\n"; 
            exit -1;
    };

    $result = smartcard::power_up_iso($fd, $buffer);
    if( $result < 0 ) {
            print "<card>Unable to power_up_iso card</card></iml>\n"; 
            exit -1;
    };

    $result = smartcard::select_file($fd, 0x3f00);
    if($result < 0) {
            print "<card>Unable to select master DF 0x3f00</card></iml>\n"; 
            exit -1;
    };
    print STDERR "Selected file 0x3f00\n";

    $result = smartcard::select_file($fd, 0x2030);
    if($result < 0) {
            print "<card>Unable to select data file 0x2030</card></iml>\n"; 
            exit -1;
    };
    print STDERR "Selected file 0x2030\n";

    $buffer = "a"x520;
    $result = smartcard::read_binary($fd, 0, 1, $buffer);
    if($result < 0) {
            print "<card>Read_binary failed</card></iml>\n"; 
            exit -1;
    };
    print STDERR substr( $buffer, 0, 1);
    $buffer = unpack("C",substr($buffer,0,1));
    print STDERR "data length = $buffer\n";

    $result = smartcard::read_binary($fd, 1, $buffer, $info);
    if($result < 0) {
            print "<card>Read_binary failed</card></iml>\n"; 
            exit -1;
    };
    $info = substr($info,0,$buffer);	
    print STDERR "Info = $info\n";

    smartcard::serial_deinit($fd);

    $info=rIML->escape_string($info);
}

print "<card><svar mode=\"init\" passwd=\"12345\"/>\n";
print "<svar mode=\"populate\" string=\"$info\" passwd=\"12345\"/>\n";
print "<br/><br/>\n";
print "<large>  You are certified smart :-)</large>\n";
print "<br/><br/>\n";
print "<table>\n";
print "<tr><td>Name:</td><td><input type=\"text\" width=\"22\" value=\"_name\"/></td></tr>\n";
print "<tr><td>Address:</td><td><input type=\"text\" width=\"22\" value=\"_addr\"/></td></tr>\n";
print "<tr><td>Sex:</td><td><input type=\"text\" width=\"22\" value=\"_sex\"/></td></tr>\n";
print "<tr><td>Title:</td><td><input type=\"text\" width=\"22\" value=\"_title\"/></td></tr>\n";
print "<tr><td>Balance:</td><td><input type=\"text\" width=\"22\" value=\"_balance\"/></td></tr>\n";
print "</table><br/><br/>\n";
print "<image id=\"img0\" mode=\"create\" width=\"230\" height=\"80\"/>\n";
print "<image id=\"img1\" tagid=\"img0\" mode=\"src\" src=\"smiley.gif\" xoff=\"75\"/>\n";
print "</card></iml>\n";
