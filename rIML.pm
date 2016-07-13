package rIML;

use sCode;

sub read {
        my $input="";
        my $c="";
        my $length="";
        while(1){
                sysread(STDIN, $c, 1);
                $input.=$c;
                if($input=~/<arg\s*length="(.*?)"\s*>/){
                        $length=$1;
                        last
                }        
        }
        $length+=6;
        $input="";
        my $r=0;
        my $buff="";
        while(1){
                $r=sysread(STDIN, $buff, $length);
                if($r<=0){
                        return undef;
                }
                $input .= $buff;
                $length=$length-$r;
                if($length==0){last}
        }
        if($input =~ /<\/arg>$/){
                $input=$`;
        }else{
                return undef;
        }        
        return sCode->decode(_unescape_string($input));
}

sub send {
        my $self=shift;
        my @arr=@_;
        for(@arr){
                print $_;                                
        }        
}        

sub unescape_string {
        my $self=shift;
        my $str=shift;
        $str=~s/&lt/</g;
        $str=~s/&gt/</g;
        $str=~s/&quot/"/g;
        $str=~s/&apos/`/g;
        $str=~s/&amp/&/g;
        return $str;
}


sub _unescape_string {
        my $str=shift;
        $str=~s/&lt/</g;
        $str=~s/&gt/</g;
        $str=~s/&quot/"/g;
        $str=~s/&apos/`/g;
        $str=~s/&amp/&/g;
        return $str;
}

sub escape_string {
        my $self=shift;
        my $str=shift;
        $str=~s/&/&amp/g;
        $str=~s/`/&apos/g;
        $str=~s/"/&quot/g;        
        $str=~s/</&lt/g;
        $str=~s/>/&gt/g;
        return $str
}
1;
